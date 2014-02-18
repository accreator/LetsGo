#include"utility.h"
#include<cstdio>
#include<cstdlib>
#include<ctime>

static void (*ExitFunc[UTILITY_EXIT_FUNC_MAX_NUM])(void);
static int ExitFuncNum = 0;

void utility_atexit(void (*func)(void))
{
	static bool flag = 1;
	if(flag)
	{
		atexit(utility_exit);
		flag = 0;
	}
	if(ExitFuncNum == UTILITY_EXIT_FUNC_MAX_NUM)
	{
		printf("ERROR: too many exit functions\n");
	}
	else
	{
		ExitFunc[ExitFuncNum] = func;
		ExitFuncNum ++;
	}
}

void utility_exit()
{
	while(ExitFuncNum --)
	{
		(*ExitFunc[ExitFuncNum])();
	}
}

void utility_rand_init()
{
	ASSERT(RAND_MAX == 32767);
#ifndef DEBUG
	srand(time(NULL));
#else
	srand(1);
#endif
}

int utility_rand_get()
{	
	return rand();
}

int utility_clock()
{
	return clock();
}