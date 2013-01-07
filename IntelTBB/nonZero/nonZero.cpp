#include <climits>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "tbb/task_scheduler_init.h"
#include "tbb/blocked_range.h"
#include "tbb/parallel_reduce.h"
#include "tbb/tick_count.h"

using namespace std;
using namespace tbb;

float Foo(float f)
{
	return f;
}

template <typename T>
class nonZeroCount
{
public:
	// constructor
	nonZeroCount(const vector<T>& v) :
		vec(v),
		nonZeroCounter(0)
	{}

	// splitting constructor
	nonZeroCount(nonZeroCount<T>& x, split) :
		vec(x.vec),
		nonZeroCounter(0)
	{}

	void operator()( const blocked_range<size_t>& r )
	{
		long _temp=nonZeroCounter;
		size_t _end = r.end();
		for( size_t i=r.begin(); i!=_end; ++i )
		{
			if(vec[i] != 0)
			{
				_temp++;
			}
		}
		nonZeroCounter=_temp;
	}

	// reduce operation
	void join(const nonZeroCount<T>& x)
	{
		nonZeroCounter += x.nonZeroCounter;
	}

public:
	long	nonZeroCounter;
private:
	const vector<T>& vec;
};

template <typename T>
class nonZeroCountSerial
{
public:
	nonZeroCountSerial(const vector<T>& v) :
		vec(v),
		nonZeroCounter(0)
	{}

	void run()
	{
		size_t _end=vec.size();
		for(size_t i = 0; i < _end; i++)
		{
			if (vec[i] != 0)
			{
				nonZeroCounter++;
			}
		}
	}

public:
	long	nonZeroCounter;
private:
	const vector<T>& vec;	
};

int main(int argc, char* argv[])
{
	// инициализация планировщика задач TBB
	task_scheduler_init init;

	// генерация исходного массива данных
	const size_t N = 1000000;
	vector<float> v(N);
	for (size_t i = 0; i < N; ++i)
		v[i] = rand()/static_cast<float>(RAND_MAX);

	// применение последовательного алгоритма
	tick_count t0 = tick_count::now();
	nonZeroCountSerial<float> s1(v);
	s1.run();
	tick_count t1 = tick_count::now();
	cout << "Time for action (serial) =" << (t1-t0).seconds() << " seconds" << endl;

	cout << "Non zero items count=" << s1.nonZeroCounter << endl;

	// применение алгоритма parallel_reduce
	tick_count t2 = tick_count::now();
	nonZeroCount<float> s2(v);
	parallel_reduce(blocked_range<size_t>(0, N), s2);
	tick_count t3 = tick_count::now();
	cout << "Time for action (parallel) =" << (t3-t2).seconds() << " seconds" << endl;

	cout << "Non zero items count=" << s2.nonZeroCounter << endl;

	double speedup = (t1-t0).seconds()/(t3-t2).seconds();
	cout << "Speedup =" << speedup << endl;

	//system("pause");

	return 0;
}

