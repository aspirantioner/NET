#include <unistd.h>
#include <fcntl.h>
#include "server.h"
#include <signal.h>

int main()
{
    /*init log appender*/
    logAppender epoller_logappender;
    logAppendInit(&epoller_logappender, "./epoll.log", O_TRUNC | O_RDWR);
    logAppender dealer_logappender;
    logAppendInit(&dealer_logappender, "./deal.log", O_TRUNC | O_RDWR);
    logAppender acceptor_logappender;
    logAppendInit(&acceptor_logappender, "./acceptor.log", O_CREAT | O_TRUNC | O_RDWR);
	logAppender server_logappender;
	logAppendInit(&server_logappender, "./server.log", O_CREAT | O_TRUNC | O_RDWR);
    /*init acceptor*/
    struct acceptor ac;
    acceptor_init(&ac, AF_INET, SOCK_STREAM, "192.168.80.129", 1234, 100, &acceptor_logappender);

    /*init epoller*/
    struct epoller ep;
    epoller_init(&ep, &epoller_logappender);

    /*init connection regulor*/
    struct conner co;
    conner_init(&co);

    /*init client send data dealer*/
    struct dealer de;
    dealer_init(&de, &dealer_logappender, dealer_run,100, 3, 10, 41, 5, 5);

    /*init log receiver*/
    struct log_rever lo;
    log_rever_init(&lo, 100, 3, 10, 41, 5, 5);

    /*binf ac ep de log for server sys*/
    struct server srv;
    server_init(&srv, &ac, &ep, &co, &de, &lo, &server_logappender, 41);
    
    server_run(&srv);
    server_cmd(&srv);
}
