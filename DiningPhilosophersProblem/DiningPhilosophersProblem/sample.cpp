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
	HANDLE leftFork;
	HANDLE rightFork;
	PhilosopherState state;

	long long timesAte;
};

Philosopher g_Philosophers[g_PhilosophersCount];

HANDLE g_PhilosophersThreads[g_PhilosophersCount];

DWORD WINAPI philosopherRoutine(LPVOID lpParameter)
{
	auto _philosopher=static_cast<Philosopher *>(lpParameter);
	printf("%d started\n", _philosopher->id);

	while (true)
	{
		Sleep(rand()%100);
		_philosopher->state=PhilosopherState::Think;

		Sleep(rand()%100);
		_philosopher->state=PhilosopherState::WaitLeftFork;
		WaitForSingleObject(_philosopher->leftFork, INFINITE);
		_philosopher->state=PhilosopherState::HasLeftFork;
		printf("%d has left fork\n", _philosopher->id);

		Sleep(rand()%100);
		_philosopher->state=PhilosopherState::WaitRightFork;
		WaitForSingleObject(_philosopher->rightFork, INFINITE);
		_philosopher->state=PhilosopherState::HasRightFork;
		printf("%d has right fork\n", _philosopher->id);

		_philosopher->state=PhilosopherState::Eat;
		printf("%d eat\n", _philosopher->id);
		_philosopher->timesAte++;
		Sleep(rand()%100);

		_philosopher->state=PhilosopherState::UnlockLeftFork;
		ReleaseMutex(_philosopher->leftFork);

		_philosopher->state=PhilosopherState::UnlockRightFork;
		ReleaseMutex(_philosopher->rightFork);

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
		_philosopher->leftFork=g_Forks[i];
		_philosopher->rightFork=g_Forks[(i+1)%g_PhilosophersCount];
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}
	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);
}