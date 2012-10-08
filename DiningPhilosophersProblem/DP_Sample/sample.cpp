#include "..\commonDefs.h"

enum PhilosopherState
{
	Think,
	WaitLeftFork,
	HasLeftFork,
	WaitRightFork,
	HasRightFork,
	Eat,
	UnlockLeftFork,
	UnlockRightFork,
};

HANDLE g_Forks[g_PhilosophersCount];

struct Philosopher
{
	int id;
	HANDLE leftFork;
	HANDLE rightFork;
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
		_philosopher->state=PhilosopherState::WaitLeftFork;
		WaitForSingleObject(_philosopher->leftFork, INFINITE);
		_philosopher->state=PhilosopherState::HasLeftFork;
		//printf("%d has left fork\n", _philosopher->id);
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif

		Sleep(rand()%100);
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
		Sleep(rand()%100);

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

	for (int i=0; i<g_PhilosophersCount; i++)
	{
		g_Forks[i]=CreateMutex(nullptr, FALSE, nullptr);
	}
#ifndef DIRECT_STATE_VIEW
	CreateThread(nullptr, 0, &stateViewer, nullptr, 0, nullptr);
#endif
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		Philosopher *_philosopher=&g_Philosophers[i];
		_philosopher->id=i;
		_philosopher->leftFork=g_Forks[i];
		_philosopher->rightFork=g_Forks[(i+1)%g_PhilosophersCount];
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}
	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);
}