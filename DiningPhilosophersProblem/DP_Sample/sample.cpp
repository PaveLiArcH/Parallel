////////////////////////////////////////////////////////////////////////////////
/// Моделирование проблемы обедающих философов
///
///	В данном примере не используется средств разрешения возникающих
///	 блокировок и, как следствие, в нормальных условиях
///	 все философы должны перейти в состояние «ожидание левой вилки».
////////////////////////////////////////////////////////////////////////////////

#include "..\commonDefs.h"

// набор состояний философов
enum PhilosopherState
{
	Think,				// размышление
	WaitLeftFork,		// ожидание левой вилки
	HasLeftFork,		// наличие левой вилки
	WaitRightFork,		// ожидание правой вилки
	HasRightFork,		// наличие правой вилки
	Eat,				// прием пищи
	UnlockLeftFork,		// освобождение левой вилки
	UnlockRightFork,	// освобождение правой вилки
};

// набор мьютексов-вилок
HANDLE g_Forks[g_PhilosophersCount];

// структура, хранящая информацию о философе
struct Philosopher
{
	int id;					// порядковый номер
	HANDLE leftFork;		// мьютекс левой вилки
	HANDLE rightFork;		// мьютекс правой вилки
	PhilosopherState state;	// состояние

	long long timesAte;		// количество успешных приемов пищи
};

// набор философов
Philosopher g_Philosophers[g_PhilosophersCount];

// набор потоков, моделирующих действия соответствующих философов
HANDLE g_PhilosophersThreads[g_PhilosophersCount];

HANDLE viewMutex;
CONSOLE_SCREEN_BUFFER_INFO info;
HANDLE stdOutHandle;

// метод для отображения текущего состояния
void stateView()
{
	WaitForSingleObject(viewMutex, INFINITE);

	SetConsoleCursorPosition(stdOutHandle, info.dwCursorPosition);
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		char *_state;
		Philosopher *_philosopher=&g_Philosophers[i];
		switch(_philosopher->state)
		{
		case (PhilosopherState::Think):
			_state="Th";
			break;
		case (PhilosopherState::WaitLeftFork):
			_state="WL";
			break;
		case (PhilosopherState::HasLeftFork):
			_state="HL";
			break;
		case (PhilosopherState::WaitRightFork):
			_state="WR";
			break;
		case (PhilosopherState::HasRightFork):
			_state="HR";
			break;
		case (PhilosopherState::Eat):
			_state="Ea";
			break;
		case (PhilosopherState::UnlockLeftFork):
			_state="UL";
			break;
		case (PhilosopherState::UnlockRightFork):
			_state="UR";
			break;
		}
		printf("%d:%s(%d) ", _philosopher->id, _state, _philosopher->timesAte);
	}
	ReleaseMutex(viewMutex);
}

// метод, моделирующий действия философа
DWORD WINAPI philosopherRoutine(LPVOID lpParameter)
{
	Philosopher *_philosopher=static_cast<Philosopher *>(lpParameter);
	//printf("%d started\n", _philosopher->id);
#ifdef DIRECT_STATE_VIEW
	stateView();
#endif
	
	while (!g_isTimeout)
	{
		Sleep(DELAY_STATE_CHANGE);
		_philosopher->state=PhilosopherState::Think;

		Sleep(DELAY_THINK);
		_philosopher->state=PhilosopherState::WaitLeftFork;
		WaitForSingleObject(_philosopher->leftFork, INFINITE);
		_philosopher->state=PhilosopherState::HasLeftFork;
		//printf("%d has left fork\n", _philosopher->id);
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif

		Sleep(DELAY_STATE_CHANGE);
		_philosopher->state=PhilosopherState::WaitRightFork;
		WaitForSingleObject(_philosopher->rightFork, INFINITE);
		_philosopher->state=PhilosopherState::HasRightFork;
		//printf("%d has right fork\n", _philosopher->id);
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif

		_philosopher->state=PhilosopherState::Eat;
		//printf("%d eat\n", _philosopher->id);
		_philosopher->timesAte++;
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif
		Sleep(DELAY_EAT);

		_philosopher->state=PhilosopherState::UnlockLeftFork;
		ReleaseMutex(_philosopher->leftFork);

		_philosopher->state=PhilosopherState::UnlockRightFork;
		ReleaseMutex(_philosopher->rightFork);

		//printf("%d think\n", _philosopher->id);
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif
	}
	return 0;
}

#ifndef DIRECT_STATE_VIEW
DWORD WINAPI stateViewer(LPVOID)
{
	Sleep(DELAY_VIEW_INIT);
	while (!g_isTimeout)
	{
		stateView();
		Sleep(DELAY_VIEW_UPDATE);
	}
	return 0;
}
#endif

int main()
{
	g_isTimeout=false;

	viewMutex=CreateMutex(nullptr, FALSE, nullptr);
	stdOutHandle=GetStdHandle(STD_OUTPUT_HANDLE);
	GetConsoleScreenBufferInfo((HANDLE)STD_OUTPUT_HANDLE, &info);

	// создаем мьютексы для вилок
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		g_Forks[i]=CreateMutex(nullptr, FALSE, nullptr);
	}
#ifndef DIRECT_STATE_VIEW
	CreateThread(nullptr, 0, &stateViewer, nullptr, 0, nullptr);
#endif
	// инициализируем структуры с информацией о философах и запускаем потоки моделирования их действий
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		Philosopher *_philosopher=&g_Philosophers[i];
		_philosopher->id=i;
		_philosopher->leftFork=g_Forks[i];
		_philosopher->rightFork=g_Forks[(i+1)%g_PhilosophersCount];
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}

	Sleep(DELAY_TOTAL); // ждем заданное время
	g_isTimeout=true; // устанавливаем флаг необходимости завершения потоков

	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE); // ожидаем завершения потоков

	stateView(); // выводим окончательные результаты — в данном примере данный код будет недостижим
}