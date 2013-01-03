#include <stdint.h>
#include <stdlib.h>

#include <Windows.h>

#include <iostream>

// для генерации случайных чисел
#include <omp.h>
#include <time.h>

using std::cout;
using std::endl;

const int N=2048;
const int N2=N*N;

const int g_LIMIT_VALUE=100;

int A[N][N], B[N][N], C[N][N];

int main()
{
	LARGE_INTEGER _start, _end;
	QueryPerformanceCounter(&_start);
	
#pragma omp parallel
	{
		srand(int(time(NULL)) ^ omp_get_thread_num());
#pragma omp for
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

	#pragma omp parallel for
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