gcc -g -c array_queue.c
gcc -g -c bitmap.c
gcc -g -c filefd.c
gcc -g -c lio_thread.c
gcc -g -c thread_pool.c
gcc -g -c log.c
gcc -g -c logserver.c
gcc -g -c log_test.c
gcc thread_pool.o bitmap.o filefd.o lio_thread.o log.o array_queue.o logserver.o log_test.o -lpthread  -g -o log_test
rm *.o
