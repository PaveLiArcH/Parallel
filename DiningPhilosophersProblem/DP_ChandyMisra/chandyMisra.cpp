#include <Windows.h>
#include <stdio.h>

/*
In 1984, K. Mani Chandy and J. Misra proposed a different solution to the dining philosophers problem to allow for arbitrary agents (numbered P1, ..., Pn) to contend for an arbitrary number of resources, unlike Dijkstra's solution. It is also completely distributed and requires no central authority after initialization. However, it violates the requirement that "the philosophers do not speak to each other" (due to the request messages).
— For every pair of philosophers contending for a resource, create a fork and give it to the philosopher with the lower ID. Each fork can either be dirty or clean. Initially, all forks are dirty.
— When a philosopher wants to use a set of resources (i.e. eat), he must obtain the forks from his contending neighbors. For all such forks he does not have, he sends a request message.
— When a philosopher with a fork receives a request message, he keeps the fork if it is clean, but gives it up when it is dirty. If he sends the fork over, he cleans the fork before doing so.
— After a philosopher is done eating, all his forks become dirty. If another philosopher had previously requested one of the forks, he cleans the fork and sends it.
This solution also allows for a large degree of concurrency, and will solve an arbitrarily large problem.
It also solves the starvation problem. The clean / dirty labels act as a way of giving preference to the most "starved" processes, and a disadvantage to processes that have just "eaten". One could compare their solution to one where philosophers are not allowed to eat twice in a row without letting others use the forks in between. Their solution is more flexible than that, but has an element tending in that direction.
In their analysis they derive a system of preference levels from the distribution of the forks and their clean/dirty states. They show that this system may describe an acyclic graph, and if so, the operations in their protocol cannot turn that graph into a cyclic one. This guarantees that deadlock cannot occur. However, if the system is initialized to a perfectly symmetric state, like all philosophers holding their left side forks, then the graph is cyclic at the outset, and their solution cannot prevent a deadlock. Initializing the system so that philosophers with lower IDs have dirty forks ensures the graph is initially acyclic.
*/

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