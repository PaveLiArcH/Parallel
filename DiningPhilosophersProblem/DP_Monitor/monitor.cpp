#include <Windows.h>
#include <stdio.h>

// http://rosettacode.org/wiki/Dining_philosophers
// http://en.wikipedia.org/wiki/Dining_philosophers_problem

enum PhilosopherState
{
	Think,
	Eat,
};

const int g_PhilosophersCount=5;

HANDLE g_monitorMutex;

struct Philosopher
{
	int id;
	PhilosopherState state;

	long long timesAte;
};

Philosopher g_Philosophers[g_PhilosophersCount];

HANDLE g_PhilosophersThreads[g_PhilosophersCount];

BOOL monitorAskMayEat(Philosopher *philosopher)
{
	WaitForSingleObject(g_monitorMutex, INFINITE);

	auto _id=philosopher->id;
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
	auto _philosopher=static_cast<Philosopher *>(lpParameter);
	printf("%d started\n", _philosopher->id);

	while (true)
	{
		Sleep(rand()%100);
		_philosopher->state=PhilosopherState::Think;

		Sleep(rand()%100);

		while (!monitorAskMayEat(_philosopher)) { Sleep(rand()%100); }

		printf("%d eat\n", _philosopher->id);
		_philosopher->timesAte++;
		Sleep(rand()%100);

		monitorSayFreeForks(_philosopher);

		printf("%d think\n", _philosopher->id);
	}
	return 0;
}

int main()
{
	g_monitorMutex=CreateMutex(nullptr, FALSE, nullptr);
	
	for (int i=0; i<g_PhilosophersCount; i++)
	{
		auto _philosopher=&g_Philosophers[i];
		_philosopher->id=i;
		g_PhilosophersThreads[i]=CreateThread(nullptr, 0, &philosopherRoutine, &g_Philosophers[i], 0, nullptr);
	}
	WaitForMultipleObjects(g_PhilosophersCount, g_PhilosophersThreads, TRUE, INFINITE);
}