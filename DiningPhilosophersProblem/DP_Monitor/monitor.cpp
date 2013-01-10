////////////////////////////////////////////////////////////////////////////////
/// Разрешение проблемы обедающих философов при помощи монитора
///
///	Основная идея подхода:
///  Философ может есть, лишь когда не ест ни один из его непосредственных соседей.
///
////////////////////////////////////////////////////////////////////////////////

#include "..\commonDefs.h"

// набор состояний философов
enum PhilosopherState
{
	Think,	// размышление
	Eat,	// прием пищи
};

// мьютекс монитора
HANDLE g_monitorMutex;

// структура, хранящая информацию о философе
struct Philosopher
{
	int id;					// порядковый номер
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
		case (PhilosopherState::Eat):
			_state="Ea";
			break;
		}
		printf("%d:%s(%d) ", _philosopher->id, _state, _philosopher->timesAte);
	}
	ReleaseMutex(viewMutex);
}

// метод запрос разрешения на прием пищи
// философ автоматически переходит в режим приема пищи в случае доступности
BOOL monitorAskMayEat(Philosopher *philosopher)
{
	// «вход» в монитор
	WaitForSingleObject(g_monitorMutex, INFINITE);

	int _id=philosopher->id;
	Philosopher* _left=&g_Philosophers[(_id-1<0)?g_PhilosophersCount-1:_id-1];
	Philosopher* _right=&g_Philosophers[(_id+1)%g_PhilosophersCount];
	
	// проверяем не едят ли непосредственные соседи
	BOOL _retVal=(_left->state!=PhilosopherState::Eat) && (_right->state!=PhilosopherState::Eat);
	if (_retVal)
	{
		philosopher->state=PhilosopherState::Eat;
	}

	// «выход» из монитора
	ReleaseMutex(g_monitorMutex);
	return _retVal;
}

// метод для сообщения о освобождении вилок и прекращении приема пищи
void monitorSayFreeForks(Philosopher *philosopher)
{
	// «вход» в монитор
	WaitForSingleObject(g_monitorMutex, INFINITE);

	philosopher->state=PhilosopherState::Think;

	// «выход» из монитора
	ReleaseMutex(g_monitorMutex);
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

		while (!monitorAskMayEat(_philosopher)) { Sleep(rand()%100); }

		//printf("%d eat\n", _philosopher->id);
		_philosopher->timesAte++;
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif
		Sleep(DELAY_EAT);

		monitorSayFreeForks(_philosopher);

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

	g_monitorMutex=CreateMutex(nullptr, FALSE, nullptr);
#ifndef DIRECT_STATE_VIEW
	CreateThread(nullptr, 0, &stateViewer, nullptr, 0, nullptr);
#endif
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		Philosopher *_philosopher=&g_Philosophers[i];
		_philosopher->id=i;
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}
	
	Sleep(DELAY_TOTAL); // ждем заданное время
	g_isTimeout=true; // устанавливаем флаг необходимости завершения потоков

	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);

	stateView(); // выводим окончательные результаты
}