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
#include <fstream>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <windows.h>
#include <string>
#include <iostream>
#include <chrono>

//using std::ofstream;
using namespace std;

const int		PRODUCER_COUNT			= 4;	// количество производителей
const int		CONSUMER_COUNT			= 12;	// количество потребителей

const int		BUFFER_SIZE				= 8;	// размер циклического буфера
const int		PRODUCER_SLEEP_TIME_MS	= 250;	// максимальная пауза между циклами производства
const int		CONSUMER_SLEEP_TIME_MS	= 3000;	// максимальная пауза между циклами потребления

const int		MAX_RUNNING_TIME		= 3000;	// максимальное время выполнения

int				buffer[BUFFER_SIZE];			// циклический буфер для очереди
unsigned int	queue_start				= 0;	// индекс начала буфера
unsigned int	queue_end				= 0;	// индекс конца буфера

// флаг для остановки потока-производителя
bool		stop_production			= false;

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

void * timerThread(void *)
{
	Sleep(MAX_RUNNING_TIME);
	printf("Timer elapsed\n");
	exit(0);
}

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

long getTime()
{
	namespace sc = std::chrono;

	auto time = sc::system_clock::now(); // get the current time

	auto since_epoch = time.time_since_epoch(); // get the duration since epoch

	// I don't know what system_clock returns
	// I think it's uint64_t nanoseconds since epoch
	// Either way this duration_cast will do the right thing
	auto millis = sc::duration_cast<sc::milliseconds>(since_epoch);

	long now = millis.count(); // just like java (new Date()).getTime();
	
	return now;
}

////////////////////////////////////////////////////////////////////////////////
// функция потока-производителя
////////////////////////////////////////////////////////////////////////////////
void * producer(void *num)
{
	int* _num=reinterpret_cast<int *>(num);
	int _id=*_num;
	delete _num;
	
	char _name[20];
	sprintf(_name, "producer_%d", _id);
	ofstream _ofile(_name);
	
	long _start=getTime();
	printf ("Producer %d start\n", _id);
	_ofile<<"Producer "<<_id<<" started at "<<_start<<endl;
	_ofile<<"====="<<endl;
	_ofile<<"wait started "<<getTime()-_start<<endl;

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
			_ofile<<"queue is full at "<<getTime()-_start<<endl;
			pthread_cond_wait(&queue_not_full_cond, &queue_mutex);
		}
		
		_ofile<<"wait ended "<<getTime()-_start<<endl;

		if (isExiting) break;
		_ofile<<"produce started "<<getTime()-_start<<endl;
		// добавляем новый элемент в очередь
		int item = num_sequence() % 100000;
		buffer[queue_end] = item;
		queue_end = (queue_end + 1) % BUFFER_SIZE;
		printf ("[+] item (%5d) has been produced by %d,  queue size = %2d (%2d, %2d)\n", item, _id, get_queue_size(), queue_start, queue_end);
		_ofile<<"produce ended "<<getTime()-_start<<endl;

		pthread_mutex_unlock(&queue_mutex);

		// посылаем сигнал для пробуждения потока-потребителя в случае,
		// если он ждет появления новых элементов в очереди
		pthread_cond_broadcast(&queue_not_empty_cond);

		_ofile<<"some work started "<<getTime()-_start<<endl;
		Sleep (my_rand() % PRODUCER_SLEEP_TIME_MS);
		_ofile<<"some work ended "<<getTime()-_start<<endl;
		_ofile<<"wait started "<<getTime()-_start<<endl;
	}

	printf ("Producer %d exiting\n", _id);
	_ofile<<"===="<<endl;
	_ofile<<"Producer "<<_id<<" exited at "<<getTime()-_start<<endl;
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
	
	char _name[20];
	sprintf(_name, "consumer_%d", _id);
	ofstream _ofile(_name);
	
	long _start=getTime();
	printf ("Consumer %d start\n", _id);
	_ofile<<"Consumer "<<_id<<" started at "<<_start<<endl;
	_ofile<<"====="<<endl;
	_ofile<<"wait started "<<getTime()-_start<<endl;

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
			_ofile<<"queue is empty at "<<getTime()-_start<<endl;
			pthread_cond_wait(&queue_not_empty_cond, &queue_mutex);
		}
		
		_ofile<<"wait ended "<<getTime()-_start<<endl;

		if (isExiting) break;
		_ofile<<"consume started "<<getTime()-_start<<endl;
		// удаляем обработанный элемент из очереди
		int item = buffer[queue_start];
		queue_start = (queue_start + 1) % BUFFER_SIZE;
		printf ("[-] item (%5d) has been processed by %d, queue size = %2d (%2d, %2d)\n", item, _id, get_queue_size(), queue_start, queue_end);
		_ofile<<"consume ended "<<getTime()-_start<<endl;

		pthread_mutex_unlock(&queue_mutex);

		// посылаем сигнал для пробуждения потока-производителя в случае,
		// если он ждет освобождения места в очереди
		pthread_cond_broadcast(&queue_not_full_cond);

		_ofile<<"some work started "<<getTime()-_start<<endl;
		Sleep (my_rand() % CONSUMER_SLEEP_TIME_MS);
		_ofile<<"some work ended "<<getTime()-_start<<endl;
		
		ofile_data(item);
		
		_ofile<<"wait started "<<getTime()-_start<<endl;
	}

	printf ("Consumer %d exiting\n", _id);
	_ofile<<"===="<<endl;
	_ofile<<"Consumer "<<_id<<" exited at "<<getTime()-_start<<endl;
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

	pthread_t _sleepTimer;
	int res = pthread_create(
			&_sleepTimer,			// идентификатор потока
			NULL,					// аттрибуты потока
			&timerThread,				// функция потока
			NULL);					// функция потока без аргумента
		if (res != 0)
		{
			printf("pthread_create failed (%d)\n", res);
			return 0;
		}

	atexit(exitFunc);
	getchar();

	return 0;
}