#include "..\commonDefs.h"

enum PhilosopherState
{
	Think,
	Eat,
};

HANDLE g_monitorMutex;

struct Philosopher
{
	int id;
	PhilosopherState state;

	long long timesAte;
};

Philosopher g_Philosophers[g_PhilosophersCount];

HANDLE g_PhilosophersThreads[g_PhilosophersCount];

HANDLE viewMutex;
CONSOLE_SCREEN_BUFFER_INFO info;
HANDLE stdOutHandle;

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

BOOL monitorAskMayEat(Philosopher *philosopher)
{
	WaitForSingleObject(g_monitorMutex, INFINITE);

	int _id=philosopher->id;
	Philosopher* _left=&g_Philosophers[(_id-1<0)?g_PhilosophersCount-1:_id-1];
	Philosopher* _right=&g_Philosophers[(_id+1)%g_PhilosophersCount];

	BOOL _retVal=(_left->state!=PhilosopherState::Eat) && (_right->state!=PhilosopherState::Eat);
	if (_retVal)
	{
		philosopher->state=PhilosopherState::Eat;
	}

	ReleaseMutex(g_monitorMutex);
	return _retVal;
}

void monitorSayFreeForks(Philosopher *philosopher)
{
	WaitForSingleObject(g_monitorMutex, INFINITE);

	philosopher->state=PhilosopherState::Think;

	ReleaseMutex(g_monitorMutex);
}

DWORD WINAPI philosopherRoutine(LPVOID lpParameter)
{
	Philosopher *_philosopher=static_cast<Philosopher *>(lpParameter);
	//printf("%d started\n", _philosopher->id);
#ifdef DIRECT_STATE_VIEW
	stateView();
#endif

	while (true)
	{
		Sleep(rand()%100);
		_philosopher->state=PhilosopherState::Think;

		Sleep(rand()%100);

		while (!monitorAskMayEat(_philosopher)) { Sleep(rand()%100); }

		//printf("%d eat\n", _philosopher->id);
		_philosopher->timesAte++;
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif
		Sleep(rand()%100);

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
	Sleep(200);
	while (true)
	{
		stateView();
		Sleep(50);
	}
}
#endif

int main()
{
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
	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);
}