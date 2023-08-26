#pragma once
#ifndef _LOG_H
#define _LOG_H
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#define LOG_APPEND_INIT 200
#define LOG_STR_LEN 1024

typedef enum loglevel
{
    DEBUG,
    INFO,
    WARN,
    ERROR
} loglevel; // event log level

typedef struct logEvent
{
    char *date;             // event occur date year-month-day
    char *time;             // event occur time
    const char *file;       // run source file
    int line;               // source file line
    pthread_t threadid;     // run thread id
    const char *_func_name; // run func name
    int _log_level;
    char _log_str[LOG_STR_LEN];
    int _log_str_len;
} logEvent;

// write log file file fd
typedef struct logAppender
{
    int mode;               // w-r?
    const char *appendfile; // log file name
    int appendfd;
} logAppender;

typedef struct logInfo
{
    struct logEvent *logevent_p;
    struct logAppender *logappender_p;
} logInfo;

/*
p must malloc memory ptr
*/

void logEventInit(logEvent *p);

void logtimeUpate(logEvent *p);

void logattrUpate(logEvent *p, loglevel level, const char *filename, pthread_t thread_id, int line, const char *funcname);

void setlogLevel(logEvent *p, loglevel level);

void setlogFunc(logEvent *p, const char *funcname);

void setlogLine(logEvent *p, int line);

void setlogFile(logEvent *p, const char *filename);

void setlogStr(logEvent *p, char *str);

void setlogThreadid(logEvent *p, pthread_t threadid);

void logAppendInit(logAppender *p, const char *appendfile, int mode);

void logAppend(logEvent *logevent, logAppender *logappender);

void *logAppender_run(void *p);

#endif
