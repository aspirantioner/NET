#include "log.h"

const char *leveArray[] = {"DEBUG", "INFO", "WARN", "ERROR"};

void logEventInit(logEvent *p)
{
    p->line = __LINE__;
    p->_log_level = INFO;
    p->threadid = pthread_self();
    p->_log_str_len = 0;
}

void logtimeUpate(logEvent *p)
{
    strcpy(p->date, __DATE__);
    strcpy(p->time, __TIME__);
}

void logattrUpate(logEvent *p, loglevel level, const char *filename, pthread_t thread_id, int line, const char *funcname)
{
    p->line = line;
    p->threadid = thread_id;
    strcpy(p->file, filename);
    strcpy(p->_func_name, funcname);
    p->_log_level = level;
    p->_log_str_len = snprintf(p->_log_str, LOG_STR_LEN, "%s %s %s %d %s [%s] %lu ", p->date, p->time, p->file, p->line, p->_func_name, leveArray[p->_log_level], p->threadid);
}

void setlogLevel(logEvent *p, loglevel level)
{
    p->_log_level = level;
}

void setlogFunc(logEvent *p, const char *funcname)
{
    strcpy(p->_func_name, funcname);
}

void setlogLine(logEvent *p, int line)
{
    p->line = line;
}

void setlogFile(logEvent *p, const char *filename)
{
    strcpy(p->file, filename);
}

void setlogStr(logEvent *p, char *str)
{
    p->_log_str_len = strlen(str) + 1;
    strncpy(p->_log_str, str, p->_log_str_len - 1);
    p->_log_str[p->_log_str_len - 1] = '\0';
}

void setlogThreadid(logEvent *p, pthread_t threadid)
{
    p->threadid = threadid;
}

void logAppendInit(logAppender *p, const char *appendfile, int mode)
{
    p->mode = mode;
    strcpy(p->appendfile, appendfile);
    p->appendfd = 0;
}

void logAppend(logEvent *logevent, logAppender *logappender)
{
    write(logappender->appendfd, logevent->_log_str, logevent->_log_str_len);
}

void *logAppender_run(void *q)
{
    if (!q)
    {
        return NULL;
    }
    struct logInfo *loginfo = q;
    logAppend(loginfo->logevent_p, loginfo->logappender_p);
    free(loginfo->logevent_p);
    free(loginfo);
}
