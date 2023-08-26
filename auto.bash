gcc -g -c array_queue.c
gcc -g -c thread_pool.c
gcc -g -c conner.c
gcc -g -c log.c
gcc -g -c logrever.c
gcc -g -c acceptor.c -o acceptor.o
gcc -g -c epoller.c
gcc -g -c dealer.c
gcc -g -c server.c
gcc -g -c main.c
gcc thread_pool.o acceptor.o epoller.o conner.o dealer.o log.o array_queue.o server.o logrever.o main.o -lpthread  -g -o main
rm *.o
