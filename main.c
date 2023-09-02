#include <unistd.h>
#include <fcntl.h>
#include "server.h"
#include "filefd.h"
#include "logclient.h"
#include <signal.h>

int main(int argc, char **argv)
{

    /*init acceptor*/
    struct acceptor ac;
    acceptor_init(&ac, AF_INET, SOCK_STREAM, "192.168.80.129", 1234, 100);
    log_client_init(&ac.log_cli, "192.168.80.128", 12345);
    log_client_open_file(&ac.log_cli, "./acceptor.log", O_CREAT | O_TRUNC | O_RDWR);

    /*init epoller*/
    struct epoller ep;
    epoller_init(&ep);
    log_client_init(&ep.log_cli, "192.168.80.128", 12345);
    log_client_open_file(&ep.log_cli, "./epoller.log", O_CREAT | O_TRUNC | O_RDWR);

    /*init connection regulor*/
    struct conner co;
    conner_init(&co);

    /*init client send data dealer*/
    struct dealer de;
    struct thread_pool dealer_thread_pool;
    thread_pool_init(&dealer_thread_pool, 10, 3, 5);
    dealer_init(&de, &dealer_thread_pool, dealer_run);
    log_client_init(&de.log_cli, "192.168.80.128", 12345);
    log_client_open_file(&de.log_cli, "./dealer.log", O_CREAT | O_TRUNC | O_RDWR);

    /*binf ac ep de log for server sys*/
    struct server srv;
    server_init(&srv, &ac, &ep, &co, &de);
    log_client_init(&srv.log_cli, "192.168.80.128", 12345);
    log_client_open_file(&srv.log_cli, "./server.log", O_CREAT | O_TRUNC | O_RDWR);

    server_run(&srv);
    server_cmd(&srv);
}
