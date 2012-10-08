#include <Windows.h>
#include <stdio.h>

// http://rosettacode.org/wiki/Dining_philosophers
// http://en.wikipedia.org/wiki/Dining_philosophers_problem

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

const int g_PhilosophersCount=5;

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

DWORD WINAPI philosopherRoutine(LPVOID lpParameter)
{
	auto _philosopher=static_cast<Philosopher *>(lpParameter);
	printf("%d started\n", _philosopher->id);

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
		printf("%d has %s fork\n", _philosopher->id, _strings[0]);

		Sleep(rand()%100);
		_philosopher->state=_states[2];
		WaitForSingleObject(_forks[1], INFINITE);
		_philosopher->state=_states[3];
		printf("%d has %s fork\n", _philosopher->id, _strings[1]);

		_philosopher->state=PhilosopherState::Eat;
		printf("%d eat\n", _philosopher->id);
		_philosopher->timesAte++;
		Sleep(rand()%100);

		_philosopher->state=_states[4];
		ReleaseMutex(_forks[1]);

		_philosopher->state=_states[5];
		ReleaseMutex(_forks[0]);

		printf("%d think\n", _philosopher->id);
	}
	return 0;
}

int main()
{
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		g_Forks[i]=CreateMutex(nullptr, FALSE, nullptr);
	}
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		auto _philosopher=&g_Philosophers[i];
		_philosopher->id=i;
		_philosopher->leftFork=i;
		_philosopher->rightFork=(i+1)%g_PhilosophersCount;
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}
	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);
}