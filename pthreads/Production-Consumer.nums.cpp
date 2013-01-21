////////////////////////////////////////////////////////////////////////////////
// Данный пример демонстрирует использование условных переменных для решения
// классической задачи "Производитель-Потребитель" с использованием очереди
// на основе циклического буфера.
//
// Условные переменные используются для проверки двух условий - на непустую и
// непереполненную очередь. В случае пустой очереди поток-потребитель будет
// ждать пока в очереди не появятся новые элементы, а в случае полной очереди
// поток-производитель приостоновит свою работу до появления свободного места
// в очереди.
//
// Работа потока-производителя останавливается из основого потока программы.
// После остановки потока-производителя поток-потребитель продолжит работу
// до тех пор пока не обработает все оставшиеся элементы из очереди.
//
// С данным примером можно провести следующий эксперимент - поменять местами
//	максимальные паузы между циклами производства.
////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <fstream>

using std::ofstream;

const int		PRODUCER_COUNT			= 4;	// количество производителей
const int		CONSUMER_COUNT			= 12;	// количество потребителей

const int		BUFFER_SIZE				= 8;	// размер циклического буфера
const int		PRODUCER_SLEEP_TIME_MS	= 250;	// максимальная пауза между циклами производства
const int		CONSUMER_SLEEP_TIME_MS	= 3000;	// максимальная пауза между циклами потребления

int				buffer[BUFFER_SIZE];			// циклический буфер для очереди
unsigned int	queue_start				= 0;	// индекс начала буфера
unsigned int	queue_end				= 0;	// индекс конца буфера

// флаг для остановки потока-производителя
bool			stop_production			= false;

// идентификаторы потоков
pthread_t		threads[PRODUCER_COUNT+CONSUMER_COUNT];

// мьютекс для доступа к очереди
pthread_mutex_t	queue_mutex				= PTHREAD_MUTEX_INITIALIZER;

// условные переменные для непереполненого и непустого буфера
pthread_cond_t	queue_not_full_cond		= PTHREAD_COND_INITIALIZER;
pthread_cond_t	queue_not_empty_cond	= PTHREAD_COND_INITIALIZER;

// обеспечении единой последовательности случайных чисел для всех потоков
pthread_mutex_t		rnd_mutex	= PTHREAD_MUTEX_INITIALIZER;	// мьютекс для ГПСЧ
int					rnd_seed	= 1;							// Значение SEED для ГПСЧ

// обеспечение генерации последовательности чисел
pthread_mutex_t		nums_mutex = PTHREAD_MUTEX_INITIALIZER;	// мьютекс для последовательности

// обеспечение вывода в файл
pthread_mutex_t		ofstream_mutex = PTHREAD_MUTEX_INITIALIZER;	// мьютекс для вывода в ofstream

#ifndef PTHREAD_WIN32
////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////
void Sleep(unsigned int msTime)
{
	timespec	time;
	time.tv_sec	= msTime/1000;
	time.tv_nsec = (msTime - time.tv_sec*1000)*1000*1000;
	int res = nanosleep(&time, NULL);
}
#endif

////////////////////////////////////////////////////////////////////////////////
// функция генерации псевдослучайного числа с общей для всех потоков
// последовательностью
////////////////////////////////////////////////////////////////////////////////
int my_rand()
{
	pthread_mutex_lock(&rnd_mutex);
	srand(rnd_seed);
	int res = rnd_seed = rand();
	pthread_mutex_unlock(&rnd_mutex);
	return res;
}

////////////////////////////////////////////////////////////////////////////////
// функция генерации очередного числа последовательности
////////////////////////////////////////////////////////////////////////////////
int num_sequence()
{
	static int _num=0;
	pthread_mutex_lock(&nums_mutex);
	int _old=_num;
	_num=(_num+1)%100000;
	pthread_mutex_unlock(&nums_mutex);
	return _old;
}

////////////////////////////////////////////////////////////////////////////////
// функция для вывода в файл
////////////////////////////////////////////////////////////////////////////////
void ofile_data(int data)
{
	static ofstream _ofile("output");
	pthread_mutex_lock(&nums_mutex);
	_ofile<<data<<" ";
	pthread_mutex_unlock(&nums_mutex);
}

////////////////////////////////////////////////////////////////////////////////
// функция для определения размера очереди
////////////////////////////////////////////////////////////////////////////////
unsigned int get_queue_size()
{
	return (queue_start <= queue_end) ?
		queue_end - queue_start :
		queue_end + BUFFER_SIZE - queue_start;
}

////////////////////////////////////////////////////////////////////////////////
// функция потока-производителя
////////////////////////////////////////////////////////////////////////////////
void * producer(void *num)
{
	int* _num=reinterpret_cast<int *>(num);
	int _id=*_num;
	delete _num;
	printf ("Producer %d start\n", _id);

	bool isExiting=false;
	// Цикл производства
	while (true)
	{

		pthread_mutex_lock(&queue_mutex);

		// проверка на остановку производства
		if (stop_production)
		{
			pthread_mutex_unlock(&queue_mutex);
			pthread_cond_broadcast(&queue_not_empty_cond);
			isExiting=true;
			break;
		}

		// если очередь переполнена - ждем
		while (get_queue_size() == BUFFER_SIZE - 1)
		{
			// проверка на остановку производства
			if (stop_production)
			{
				pthread_mutex_unlock(&queue_mutex);
				pthread_cond_broadcast(&queue_not_empty_cond);
				isExiting=true;
				break;
			}

			printf ("Producer %d is waiting (queue is full)\n", _id);
			pthread_cond_wait(&queue_not_full_cond, &queue_mutex);
		}

		if (isExiting) break;
		// добавляем новый элемент в очередь
		int item = num_sequence() % 100000;
		buffer[queue_end] = item;
		queue_end = (queue_end + 1) % BUFFER_SIZE;
		printf ("[+] item (%5d) has been produced,  queue size = %2d (%2d, %2d)\n", item, get_queue_size(), queue_start, queue_end);

		pthread_mutex_unlock(&queue_mutex);

		// посылаем сигнал для пробуждения потока-потребителя в случае,
		// если он ждет появления новых элементов в очереди
		pthread_cond_broadcast(&queue_not_empty_cond);

		Sleep (my_rand() % PRODUCER_SLEEP_TIME_MS);
	}

	printf ("Producer %d exiting\n", _id);
	pthread_exit(0);
}

////////////////////////////////////////////////////////////////////////////////
// функция потока-потребителя
////////////////////////////////////////////////////////////////////////////////
void * consumer(void *num)
{
	int* _num=reinterpret_cast<int *>(num);
	int _id=*_num;
	delete _num;
	printf ("Consumer %d start\n", _id);

	bool isExiting=false;
	// Цикл потребления
	while (true)
	{
		pthread_mutex_lock(&queue_mutex);

		// проверка на окончание работы
		if (stop_production && get_queue_size() == 0)
		{
			pthread_mutex_unlock(&queue_mutex);
			pthread_cond_broadcast(&queue_not_full_cond);
			isExiting=true;
			break;
		}

		// если очередь пуста - ждем
		while (get_queue_size() == 0)
		{
			// проверка на окончание работы
			if (stop_production && get_queue_size() == 0)
			{
				pthread_mutex_unlock(&queue_mutex);
				pthread_cond_broadcast(&queue_not_full_cond);
				isExiting=true;
				break;
			}

			printf("Consumer %d is waiting (queue is empty)\n", _id);
			pthread_cond_wait(&queue_not_empty_cond, &queue_mutex);
		}

		if (isExiting) break;
		// удаляем обработанный элемент из очереди
		int item = buffer[queue_start];
		queue_start = (queue_start + 1) % BUFFER_SIZE;
		printf ("[-] item (%5d) has been processed, queue size = %2d (%2d, %2d)\n", item, get_queue_size(), queue_start, queue_end);

		pthread_mutex_unlock(&queue_mutex);

		// посылаем сигнал для пробуждения потока-производителя в случае,
		// если он ждет освобождения места в очереди
		pthread_cond_broadcast(&queue_not_full_cond);

		Sleep (my_rand() % CONSUMER_SLEEP_TIME_MS);
		
		ofile_data(item);
	}

	printf ("Consumer %d exiting\n", _id);
	pthread_exit(0);
}

void exitFunc()
{
	printf("Stopping\n");
	stop_production = true;

	// ожидание завершения потоков
	for (int i = 0; i < PRODUCER_COUNT+CONSUMER_COUNT; ++i)
	{
		int res = pthread_join(
			threads[i],				// идентификатор потока
			NULL);					// указатель на возвращаемое значение
		if (res != 0)
			printf("pthread_join failed (%d)\n", res);
	}
}

int main(int argc, char *argv[])
{
	printf("Press enter to stop\n");

	for (int i=0; i<PRODUCER_COUNT; i++)
	{
		// создание потока-производителя
		int res = pthread_create(
			&threads[i],			// идентификатор потока
			NULL,					// аттрибуты потока
			&producer,				// функция потока
			new int(i));					// функция потока без аргумента
		if (res != 0)
		{
			printf("pthread_create failed (%d)\n", res);
			return 0;
		}
	}

	for (int i=0; i<CONSUMER_COUNT; i++)
	{
		// создание потока-потребителя
		int res = pthread_create(
			&threads[PRODUCER_COUNT+i],			// идентификатор потока
			NULL,					// аттрибуты потока
			&consumer,				// функция потока
			new int(i));					// функция потока без аргумента
		if (res != 0)
		{
			printf("pthread_create failed (%d)\n", res);
			return 0;
		}
	}

	//getchar();
	Sleep(7000);

	return 0;
}