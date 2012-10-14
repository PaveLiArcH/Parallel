////////////////////////////////////////////////////////////////////////////////
/// Разрешение проблемы обедающих философов путем присвоения приоритета ресурсам
///
///	Основная идея подхода:
///  Каждая рабочая единица всегда берёт сначала вилку с наименьшим номером, 
///   а потом вилку с наибольшим номером из двух доступных. 
///   Далее освобождается сначала вилка с бо́льшим номером, потом — с меньшим.
///
/// Такой подход гарантирует, что не возникнет ситуация, когда по одной вилке 
///  будет у каждого из философов - что позволит есть как минимум 
///  одному из философов.
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
	int leftFork;			// номер левой вилки
	int rightFork;			// номер правой вилки
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

	HANDLE _forks[2];
	PhilosopherState _states[6];
	char *_strings[2];

	if (_philosopher->leftFork<_philosopher->rightFork)
	{
		_forks[0]=g_Forks[_philosopher->leftFork];
		_forks[1]=g_Forks[_philosopher->rightFork];
		
		_states[0]=PhilosopherState::WaitLeftFork;
		_states[1]=PhilosopherState::HasLeftFork;
		_states[2]=PhilosopherState::WaitRightFork;
		_states[3]=PhilosopherState::HasRightFork;
		_states[4]=PhilosopherState::UnlockLeftFork;
		_states[5]=PhilosopherState::UnlockRightFork;

		_strings[0]="left";
		_strings[1]="right";
	} else
	{
		_forks[0]=g_Forks[_philosopher->rightFork];
		_forks[1]=g_Forks[_philosopher->leftFork];
		
		_states[0]=PhilosopherState::WaitRightFork;
		_states[1]=PhilosopherState::HasRightFork;
		_states[2]=PhilosopherState::WaitLeftFork;
		_states[3]=PhilosopherState::HasLeftFork;
		_states[4]=PhilosopherState::UnlockRightFork;
		_states[5]=PhilosopherState::UnlockLeftFork;

		_strings[0]="right";
		_strings[1]="left";
	}

	while (!g_isTimeout)
	{
		Sleep(DELAY_STATE_CHANGE);
		_philosopher->state=PhilosopherState::Think;

		Sleep(DELAY_THINK);
		_philosopher->state=_states[0];
		WaitForSingleObject(_forks[0], INFINITE);
		_philosopher->state=_states[1];
		//printf("%d has %s fork\n", _philosopher->id, _strings[0]);
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif

		Sleep(DELAY_STATE_CHANGE);
		_philosopher->state=_states[2];
		WaitForSingleObject(_forks[1], INFINITE);
		_philosopher->state=_states[3];
		//printf("%d has %s fork\n", _philosopher->id, _strings[1]);
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

		_philosopher->state=_states[4];
		ReleaseMutex(_forks[1]);

		_philosopher->state=_states[5];
		ReleaseMutex(_forks[0]);

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
		_philosopher->leftFork=i;
		_philosopher->rightFork=(i+1)%g_PhilosophersCount;
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}

	Sleep(DELAY_TOTAL); // ждем заданное время
	g_isTimeout=true; // устанавливаем флаг необходимости завершения потоков

	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);

	stateView(); // выводим окончательные результаты
}