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
	int leftFork;
	int rightFork;
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

	while (true)
	{
		Sleep(rand()%100);
		_philosopher->state=PhilosopherState::Think;

		Sleep(rand()%100);
		_philosopher->state=_states[0];
		WaitForSingleObject(_forks[0], INFINITE);
		_philosopher->state=_states[1];
		//printf("%d has %s fork\n", _philosopher->id, _strings[0]);
#ifdef DIRECT_STATE_VIEW
		stateView();
#endif

		Sleep(rand()%100);
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
		Sleep(rand()%100);

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
		_philosopher->leftFork=i;
		_philosopher->rightFork=(i+1)%g_PhilosophersCount;
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}
	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);
}