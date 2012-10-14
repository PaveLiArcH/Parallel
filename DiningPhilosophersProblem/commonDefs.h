#include <Windows.h>
#include <stdio.h>

// http://rosettacode.org/wiki/Dining_philosophers
// http://en.wikipedia.org/wiki/Dining_philosophers_problem

#define nullptr NULL

//#define DIRECT_STATE_VIEW

const int g_PhilosophersCount=5;

#define DELAY_INIT rand()%100
#define DELAY_STATE_CHANGE rand()%100
#define DELAY_THINK rand()%100
#define DELAY_EAT rand()%100

#define DELAY_TOTAL 5000

#define DELAY_VIEW_UPDATE 50
#define DELAY_VIEW_INIT 200

bool g_isTimeout;