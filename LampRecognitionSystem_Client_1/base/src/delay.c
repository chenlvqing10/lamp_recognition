#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/select.h>

void delayus(unsigned int us)
{
	struct timeval tim;
	tim.tv_sec = us / 1000000;
	tim.tv_usec = us % 1000000;
	select(0,NULL,NULL,NULL,&tim);
}

void delayms(unsigned int ms)
{
	delayus(ms * 1000);
}
