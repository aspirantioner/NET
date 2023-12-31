#include "server.h"
#include "lio_thread.h"
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
init server system
bind ac,ep,co,de,lo and set server log,exit sig
*/
void server_init(server *srv, acceptor *ac, epoller *ep, conn_pool *co, dealer *de,cache_pool* ca)
{
	char buf[1024]; // info char buffer
	int buf_len = 0;

	/*init server acceptor*/
	assert(ac != NULL);
	srv->ac = ac;
	buf_len=sprintf(buf, "srv: acceptor bind -ip:%s -port:%hu init success!\n", srv->ac->server_ip, srv->ac->server_port);
	write(STDOUT_FILENO, buf, buf_len);
	bzero(buf, buf_len);

	/*init server epoller*/
	assert(ep != NULL);
	srv->ep = ep;
	buf_len=sprintf(buf, "srv: epoller -epfd:%d init success!\n", srv->ep->epfd);
	write(STDOUT_FILENO, buf, buf_len);
	bzero(buf, buf_len);

	/*init server conn_pool*/
	srv->co = co;
	buf_len=sprintf(buf, "srv: conn_pool -exitnum:%d init success!\n", srv->co->conn_num);
	write(STDOUT_FILENO, buf, buf_len);
	bzero(buf, buf_len);

	/*init server dealer*/
	assert(de != NULL);
	srv->de = de;
	buf_len=sprintf(buf, "srv: dealer -minnum:%d -maxnum:%d -exitnum:%d -queue capacity:%d init success!\n", de->dealer_thread_pool_p->work_thread_min_num, de->dealer_thread_pool_p->work_thread_max_num, de->dealer_thread_pool_p->work_thread_exit_num, de->dealer_thread_pool_p->queue->capacity);
	write(STDOUT_FILENO, buf, buf_len);
	bzero(buf, buf_len);

	assert(ca != NULL);
	srv->ca = ca;
	buf_len=sprintf(buf, "srv: cache_pool -cache_elem_size:%d -cache_elem_sum:%d -recycle_queue_limit:%d init success!\n", ca->cache_elem_size, ca->cache_elem_sum,ca->cache_elem_recycle_limit);
	write(STDOUT_FILENO, buf, buf_len);
	bzero(buf, buf_len);

	write(STDOUT_FILENO, "server init successfully!\n", 27);
	return;
}

/*
start new thread for ac,ep,de and lo
run server system
*/
void server_run(struct server *p)
{

	lio_thread_set(&p->ac->thread, acceptor_run, p);
	lio_thread_set(&p->ep->thread, epoller_run, p);
	lio_thread_run(&p->ac->thread);
	lio_thread_run(&p->ep->thread);
	thread_pool_run(p->de->dealer_thread_pool_p); // finnally start de threads
	write(STDOUT_FILENO, "server start running!\n", 23);
	return;
}

/*
all server system destroy
*/
void server_destroy(struct server *p)
{
	char buf[1024] = {0};

	/*close acceptor*/
	acceptor_destroy(p->ac);
	int len = sprintf(buf, "acceptor has exit!\n");
	write(STDOUT_FILENO, buf, strlen(buf));

	if (p->log_cli.log_pkt.log_append.appendfd > 0)
	{
		logEventInit(&p->log_cli.log_pkt.log_event);
		logtimeUpate(&p->log_cli.log_pkt.log_event);
		logattrUpate(&p->log_cli.log_pkt.log_event, INFO, __FILE__, p->ac->thread.thread_id, __LINE__, __FUNCTION__);
		p->log_cli.log_pkt.log_event._log_str_len += sprintf(p->log_cli.log_pkt.log_event._log_str + p->log_cli.log_pkt.log_event._log_str_len, "%s", buf);
		log_client_write(&p->log_cli);
	}
	

	/*close all client conn fd in conn_pool*/
	conn_pool_destroy(p->co);
	bzero(buf, len);
	len = sprintf(buf, "conn_pool has close all!\n");
	write(STDOUT_FILENO, buf, len);

	if (p->log_cli.log_pkt.log_append.appendfd > 0)
	{
		logEventInit(&p->log_cli.log_pkt.log_event);
		logtimeUpate(&p->log_cli.log_pkt.log_event);
		logattrUpate(&p->log_cli.log_pkt.log_event, INFO, __FILE__, p->ac->thread.thread_id, __LINE__, __FUNCTION__);
		p->log_cli.log_pkt.log_event._log_str_len += sprintf(p->log_cli.log_pkt.log_event._log_str + p->log_cli.log_pkt.log_event._log_str_len, "%s", buf);
		log_client_write(&p->log_cli);
	}

	/*close epoller*/
	epoller_destroy(p->ep);
	bzero(buf, len);
	len = sprintf(buf, "epoller has exit!\n");
	write(STDOUT_FILENO, buf, len);
	
	if (p->log_cli.log_pkt.log_append.appendfd > 0)
	{
		logEventInit(&p->log_cli.log_pkt.log_event);
		logtimeUpate(&p->log_cli.log_pkt.log_event);
		logattrUpate(&p->log_cli.log_pkt.log_event, INFO, __FILE__, p->ac->thread.thread_id, __LINE__, __FUNCTION__);
		p->log_cli.log_pkt.log_event._log_str_len += sprintf(p->log_cli.log_pkt.log_event._log_str + p->log_cli.log_pkt.log_event._log_str_len, "%s", buf);
		log_client_write(&p->log_cli);
	}

	/*close dealer*/
	thread_pool_destory(p->de->dealer_thread_pool_p); // destroy dealer thread pool
	write(STDOUT_FILENO, "dealer thread pool has destory!\n", 33);
	if (p->log_cli.log_pkt.log_append.appendfd > 0)
	{
		logEventInit(&p->log_cli.log_pkt.log_event);
		logtimeUpate(&p->log_cli.log_pkt.log_event);
		logattrUpate(&p->log_cli.log_pkt.log_event, INFO, __FILE__, p->ac->thread.thread_id, __LINE__, __FUNCTION__);
		p->log_cli.log_pkt.log_event._log_str_len += sprintf(p->log_cli.log_pkt.log_event._log_str + p->log_cli.log_pkt.log_event._log_str_len, "dealer thread pool has destory!\n");
		log_client_write(&p->log_cli);
	}
	/*close dealer log file*/
	if (p->de->log_cli.log_pkt.log_append.appendfd > 0)
	{
		log_client_close(&p->de->log_cli);
	}

	/*close server log file*/
	if (p->log_cli.log_pkt.log_append.appendfd > 0)
	{
		log_client_close(&p->log_cli);
	}
}

/*
communicate with server system
for user cmd
*/
void server_cmd(struct server *p)
{
	char cmd_buf[1024];
	int len = 0;

	sleep(2); // buff for new thread running
	while (true)
	{
		write(STDOUT_FILENO, "srv: ", strlen("srv: "));
		read(STDIN_FILENO, cmd_buf, 1024);
		if (strncmp(cmd_buf, "exit", 4) == 0)
		{
			server_destroy(p);
			break;
		}
		bzero(cmd_buf, 1024);
	}
	write(STDOUT_FILENO, "srv: server has exit successfully!\n", 36);
	pthread_exit(NULL);
}
