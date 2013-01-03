#include <stdint.h>
#include <stdlib.h>

#include <Windows.h>

#include <iostream>

using std::cout;
using std::endl;

const int N=1048576;

const int g_LIMIT_VALUE=RAND_MAX;

int increment(long inc[], long size)
{
	// inc[] ������, � ������� ��������� ����������
	// size ����������� ����� �������
	int p1, p2, p3, s;

	p1 = p2 = p3 = 1;
	s = -1;

	// ��������� ������ ��������� �� ������� ������� ��������
	do
	{
		if (++s % 2)
		{
			inc[s] = 8*p1 - 6*p2 + 1;
		}
		else
		{
			inc[s] = 9*p1 - 9*p3 + 1;
			p2 *= 2;
			p3 *= 2;
		}
		p1 *= 2;
		// ��������� ������, ���� ������� ���������� ���� �� � 3 ���� ������ ���������� ��������� � �������
	} while (3*inc[s] < size);  

	return s > 0 ? --s : 0;// ���������� ���������� ��������� � �������
}

template<class T>
void shellSort(T a[], long size)
{
	// inc ���������, ���������� ����� ���������� ���������
	// i � j ����������� ���������� �����
	// seq[40] ������, � ������� �������� ����������
	long inc, i, j, seq[40];
	int s;//���������� ��������� � ������� seq[40]

	// ���������� ������������������ ����������
	s = increment(seq, size);
	while (s >= 0)
	{
		//��������� �� ������� ��������� ����������
		inc = seq[s--];
		// ���������� ��������� � ������������ inc
		for (i = inc; i < size; i++)
		{
			T temp = a[i];
			// �������� �������� �� ��� ���, ���� �� ������ �� ����� ��� �� ���������� � ������ �������
			for (j = i-inc; (j >= 0) && (a[j] > temp); j -= inc)
				a[j+inc] = a[j];
			// ����� ���� ������� ������ �� ����� j+inc �������, ������� ��������� �� i �����
			a[j+inc] = temp;
		}
	}
}

int main()
{
	int *A=new int[N];
	LARGE_INTEGER _start, _end;
	
	srand(0);
	for(int i=0; i<N; i++)
	{
		A[i]=rand()%g_LIMIT_VALUE;
	}
	
	QueryPerformanceCounter(&_start);

	shellSort(A, N);

	QueryPerformanceCounter(&_end);
	auto _result=_end.QuadPart-_start.QuadPart;

	cout<<"Sort time: "<<_result<<endl;
	system("pause");
}