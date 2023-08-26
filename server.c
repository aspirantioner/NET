#include "server.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/signal.h>
#include <assert.h>

/*
thread receive exit signal and exit
 */
void exit_handle(int sig)
{
    pthread_exit(NULL);
}

/*
init server system
bind ac,ep,co,de,lo and set server log,exit sig
*/
void server_init(server *srv, acceptor *ac, epoller *ep, conner *co, dealer *de, log_rever *lo, logAppender *log_p, int exit_sig)
{
    char buf[1024];//info char buffer

    /*init server epoller*/
    assert(ep != NULL);
    srv->ep = ep;
    sprintf(buf, "srv: epoller -epfd:%d init success!\n", srv->ep->epfd);
    write(STDOUT_FILENO, buf, strlen(buf));
    bzero(buf, strlen(buf));

    /*init server acceptor*/
    assert(ac != NULL);
    srv->ac = ac;
    sprintf(buf, "srv: acceptor bind -ip:%s -port:%hu init success!\n", srv->ac->server_ip, srv->ac->server_port);
    write(STDOUT_FILENO, buf, strlen(buf));
    bzero(buf, strlen(buf));

    /*init server conner*/
    srv->co = co;
    sprintf(buf, "srv: conner -exitnum:%d init success!\n", srv->co->conn_num);
    write(STDOUT_FILENO, buf, strlen(buf));
    bzero(buf, strlen(buf));

    /*init server dealer*/
    assert(de != NULL);
    srv->de = de;
    sprintf(buf, "srv: dealer -minnum:%d -maxnum:%d -exitnum:%d -queue capacity:%d -detect time:%d -tolerate time:%d init success!\n", de->dealer_thread_pool.work_thread_min_num, de->dealer_thread_pool.work_thread_max_num, de->dealer_thread_pool.work_thread_exit_num, de->dealer_thread_pool.queue->capacity, de->dealer_thread_pool.regulor_detect_time, de->dealer_thread_pool.regulor_tolerate_time);
    write(STDOUT_FILENO, buf, strlen(buf));
    bzero(buf, strlen(buf));

    /*init server logrever*/
    assert(lo != NULL);
    srv->lo = lo;
    sprintf(buf, "srv: dealer -minnum:%d -maxnum:%d -exitnum:%d -queue capacity:%d -detect time:%d -tolerate time:%d init success!\n", lo->log_thread_pool.work_thread_min_num, lo->log_thread_pool.work_thread_max_num, lo->log_thread_pool.work_thread_exit_num, lo->log_thread_pool.queue->capacity, lo->log_thread_pool.regulor_detect_time, lo->log_thread_pool.regulor_tolerate_time);
    write(STDOUT_FILENO, buf, strlen(buf));
    bzero(buf, strlen(buf));

    /*set exit sig*/
    assert(exit_sig > 32);
    srv->exit_sig = exit_sig;

    /*set exit_sig handle for epoller and acceptor*/
    struct sigaction act;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    act.sa_handler = exit_handle;
    if (sigaction(srv->exit_sig, &act, NULL))
    {
        perror("sig register fail");
    }
    sprintf(buf, "srv: -exit sig:%d register successfully\n", srv->exit_sig);
    write(STDOUT_FILENO, buf, strlen(buf));
    bzero(buf, strlen(buf));

    /*set logappender*/
    srv->logappender = log_p;
    write(STDOUT_FILENO, "server init successfully!\n", 27);
    return;
}

/*
start new thread for ac,ep,de and lo
run server system
*/
void server_run(struct server *p)
{
    thread_pool_run(&p->lo->log_thread_pool);//first start log pool thread 
    pthread_create(&p->ac->self_thread_id, NULL, accpetor_run, p);//start ac thread
    pthread_create(&p->ep->self_thread_id, NULL, epoller_run, p);//start ep thread
    thread_pool_run(&p->de->dealer_thread_pool);//finnally start de threads
    write(STDOUT_FILENO, "server start running!\n", 23);
    return;
}

/*
communicate with server system
for user cmd
*/
void server_cmd(struct server *p)
{
    char cmd_buf[1024];
    int len = 0;
    while (true)
    {
        sleep(3);//buff for new thread running
        write(STDOUT_FILENO, "srv: ", strlen("srv: "));
        read(STDIN_FILENO, cmd_buf, 1024);
        struct work_unit *unit;
        struct logInfo *loginfo;
        struct logEvent *logevent;
        if (strncmp(cmd_buf, "exit", 4) == 0)
        {
		    char buf[1024] = {0};

		    /*close acceptor*/
		    pthread_kill(p->ac->self_thread_id, p->exit_sig);
		    pthread_join(p->ac->self_thread_id, NULL);
		    len = sprintf(buf, "acceptor has exit!\n");
		    write(STDOUT_FILENO, buf, strlen(buf));
		    close(p->ac->listen_socket);
		   
		    if (p->ac->logappender)
		    {
		        loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
		        logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
		        logtimeUpate(logevent);
		        logattrUpate(logevent, INFO, __FILE__, p->ac->self_thread_id, __LINE__, __FUNCTION__);
		        logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "%s", buf);
		        loginfo->logevent_p = logevent;
		        loginfo->logappender_p = p->ac->logappender;
		        unit = (struct work_unit *)malloc(sizeof(struct work_unit));
		        unit->work_data = loginfo;
		        unit->work_fun = logAppender_run;
		        send_work_func(&p->lo->log_thread_pool, unit);
		    }
			
		    /*close all client conn fd in conner*/
		    conner_closeall(p->co);
		    bzero(buf, len);
		    len = sprintf(buf, "conner has close all!\n");
		    write(STDOUT_FILENO, buf, len);
		    if (p->logappender)
		    {
		        loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
		        logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
		        logtimeUpate(logevent);
		        logattrUpate(logevent, INFO, __FILE__, p->ep->self_thread_id, __LINE__, __FUNCTION__);
		        logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "%s", buf);
		        loginfo->logevent_p = logevent;
		        loginfo->logappender_p = p->logappender;
		        unit = (struct work_unit *)malloc(sizeof(struct work_unit));
		        unit->work_data = loginfo;
		        unit->work_fun = logAppender_run;
		        send_work_func(&p->lo->log_thread_pool, unit);
		    }

		    /*close epoller*/
		    pthread_kill(p->ep->self_thread_id, p->exit_sig);
		    pthread_join(p->ep->self_thread_id, NULL);
		    bzero(buf, len);
		    len = sprintf(buf, "epoller has exit!\n");
		    write(STDOUT_FILENO, buf, len);
		    close(p->ep->epfd);//close epoll fd
		    if (p->ep->logappender)
		    {

		        loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
		        logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
		        logtimeUpate(logevent);
		        logattrUpate(logevent, INFO, __FILE__, p->ep->self_thread_id, __LINE__, __FUNCTION__);
		        logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "%s", buf);
		        loginfo->logevent_p = logevent;
		        loginfo->logappender_p = p->ep->logappender;
		        unit = (struct work_unit *)malloc(sizeof(struct work_unit));
		        unit->work_data = loginfo;
		        unit->work_fun = logAppender_run;
		        send_work_func(&p->lo->log_thread_pool, unit);
		    }

		    /*close dealer*/
		    thread_pool_destory(&p->de->dealer_thread_pool);//destroy dealer thread pool
		    write(STDOUT_FILENO, "dealer thread pool has destory!\n", 33);
		    if (p->de->logappender)
		    {
		        loginfo = (struct logInfo *)malloc(sizeof(struct logInfo));
		        logevent = (struct logEvent *)malloc(sizeof(struct logEvent));
		        logtimeUpate(logevent);
		        logattrUpate(logevent, INFO, __FILE__, p->ep->self_thread_id, __LINE__, __FUNCTION__);
		        logevent->_log_str_len += sprintf(logevent->_log_str + logevent->_log_str_len, "dealer thread pool has destory!\n");
		        loginfo->logappender_p = p->de->logappender;
		        loginfo->logevent_p = logevent;
		        loginfo->logappender_p = p->logappender;
		        unit = (struct work_unit *)malloc(sizeof(struct work_unit));
		        unit->work_data = loginfo;
		        unit->work_fun = logAppender_run;
		        send_work_func(&p->lo->log_thread_pool, unit);
		    }

		    /*close log thread pool*/
		    thread_pool_destory(&p->lo->log_thread_pool);
		    write(STDOUT_FILENO, "logger thread pool has destory!\n", 33);
		    
		    /*close log appender fd*/
		    if(p->ac->logappender){
		    	close(p->ac->logappender->appendfd);
		    }
		    if(p->ep->logappender){
		    	close(p->ep->logappender->appendfd);
		    }
		    if(p->de->logappender){
		    	close(p->de->logappender->appendfd);
		    }
		    if(p->logappender){
		    	close(p->logappender->appendfd);
		    }
		    break;
		}
		bzero(cmd_buf, 1024);
    }
    write(STDOUT_FILENO, "srv: server has exit successfully!\n", 36);
    pthread_exit(NULL);
}
