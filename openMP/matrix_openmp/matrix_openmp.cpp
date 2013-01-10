#include <stdint.h>
#include <stdlib.h>

#include <Windows.h>

#include <iostream>

// для генерации случайных чисел
#include <omp.h>
#include <time.h>

using std::cout;
using std::endl;

const int N=256;

const int g_LIMIT_VALUE=100;

int A[N][N], B[N][N], C[N][N];

//#define USED_SCHEDULE schedule(static, 5)
//#define USED_SCHEDULE schedule(dynamic, 5)
#define USED_SCHEDULE schedule(guided)

int main()
{
	int nthreads, tid, procs, maxt, inpar, dynamic, nested;

#pragma omp parallel private(nthreads, tid)
	{
		/* Obtain thread number */
		tid = omp_get_thread_num();

		/* Only master thread does this */
		if (tid == 0) 
		{
			printf("Thread %d getting environment info...\n", tid);

			/* Get environment information */
			procs = omp_get_num_procs();
			nthreads = omp_get_num_threads();
			maxt = omp_get_max_threads();
			inpar = omp_in_parallel();
			dynamic = omp_get_dynamic();
			nested = omp_get_nested();

			/* Print environment information */
			printf("Number of processors = %d\n", procs);
			printf("Number of threads = %d\n", nthreads);
			printf("Max threads = %d\n", maxt);
			printf("In parallel? = %d\n", inpar);
			printf("Dynamic threads enabled? = %d\n", dynamic);
			printf("Nested parallelism supported? = %d\n", nested);
		}
	}

	LARGE_INTEGER _start, _end;
	QueryPerformanceCounter(&_start);

#pragma omp parallel
	{
		srand(int(time(NULL)) ^ omp_get_thread_num());
#pragma omp for USED_SCHEDULE
		for(int i=0; i<N; i++)
		{
			for (int j=0; j<N; j++)
			{
				A[i][j]=rand()%g_LIMIT_VALUE;
				B[i][j]=rand()%g_LIMIT_VALUE;
			}
		}
	}

	QueryPerformanceCounter(&_end);
	auto _result=_end.QuadPart-_start.QuadPart;

	cout<<"Generation time: "<<_result<<endl;

	QueryPerformanceCounter(&_start);

#pragma omp parallel for USED_SCHEDULE
	for(int i=0; i<N; i++)
	{
		for (int j=0; j<N; j++)
		{
			int _temp=0;
			for (int r=0; r<N; r++)
			{
				_temp+=A[i][r]*B[r][j];
			}
			C[i][j]=_temp;
		}
	}

	QueryPerformanceCounter(&_end);
	_result=_end.QuadPart-_start.QuadPart;

	cout<<"Multiplication time: "<<_result<<endl;
	system("pause");
}