#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "fileop.h"
#include "collect.h"
#include "crc32.h"

extern FILE *file_log;


int getTime(char *out, int fmt)
{
	if(out == NULL)
	{
	    return FAILED;
	}
	time_t t;
	struct tm *tp;
	t = time(NULL);

	tp = localtime(&t);
	if(fmt == 0)
	{
	    sprintf(out, "%2.2d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	}
	else if(fmt == 1)
	{
	    sprintf(out, "%2.2d-%2.2d-%2.2d", tp->tm_year+1900, tp->tm_mon+1, tp->tm_mday);
	}
	else if(fmt == 2)
	{
	    sprintf(out, "%2.2d:%2.2d:%2.2d", tp->tm_hour, tp->tm_min, tp->tm_sec);
	}
	return SUCCESS;
}


int write_log(char *str)
{
    char curTime[100] = {0};
    int ret = -1;

    memset(curTime,0,100);
	
    if(NULL == file_log || NULL == str)	
    {
	return FAILED;
    }
	
    getTime(curTime, 0);
    ret = fprintf(file_log, "[%s] : %s", curTime, str);
  
    if(ret >= 0)
    {
    	fflush(file_log);
        return SUCCESS;
    }
    else
    {
	return FAILED;
    }
}
