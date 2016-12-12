#include "cs537.h"
#include "request.h"

// 
// server.c: A very, very simple web server
//
// To run:
//  server <portnum (above 2000)>
//
// Repeatedly handles HTTP requests sent to this port number.
// Most of the work is done within routines written in request.c
//

// CS537: Parse the new arguments too
void getargs(int *port, int* tnum, int* bnum, int argc, char *argv[])
{
    if (argc != 4) {
	fprintf(stderr, "Usage: %s <portnum> <threads> <buffers>\n", argv[0]);
	exit(1);
    }
    *port = atoi(argv[1]);
    if (*port < 2000) {
        fprintf(stderr, "Port number should be larger than 2000\n");
    }
    *tnum = atoi(argv[2]);
    *bnum = atoi(argv[3]);
}

void put(int connfd, server_t* shm) {
    shm->buffer[shm->bend] = connfd;
    shm->bend = (shm->bend + 1) % shm->bnum; 
    shm->bcount ++;
}

int get(server_t* shm) {
    int connfd = shm->buffer[shm->bstart];
    shm->bstart = (shm->bstart + 1) % shm->bnum;
    shm->bcount --;
    return connfd;
}

void producer(int connfd, server_t* shm) {
    pthread_mutex_lock(shm->mutex);
    while(shm->bcount == shm->bnum) {
        pthread_cond_wait(shm->empty, shm->mutex);
    }
    put(connfd, shm);
    pthread_cond_signal(shm->fill);
    pthread_mutex_unlock(shm->mutex);  
}

void *consumer(void* arg) {
    while(1) {
        server_t* shm = (server_t*) arg;
        pthread_mutex_lock(shm->mutex);
        while(shm->bcount == 0) {
            pthread_cond_wait(shm->fill, shm->mutex);
        }
        int connfd = get(shm);
        pthread_cond_signal(shm->empty);
        pthread_mutex_unlock(shm->mutex);

        requestHandle(connfd);
        Close(connfd);
    }
}
int main(int argc, char *argv[])
{
    int listenfd, connfd, port, clientlen;
    struct sockaddr_in clientaddr;
    
    server_t shm;
    pthread_t* threads;
    
    getargs(&port, &shm.tnum, &shm.bnum, argc, argv);

    // malloc memory for buffer
    shm.buffer = malloc(sizeof(int) * shm.bnum);
    shm.empty = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(shm.empty, NULL);
    shm.fill = (pthread_cond_t*) malloc(sizeof(pthread_cond_t));
    pthread_cond_init(shm.fill, NULL);
    shm.mutex = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(shm.mutex, NULL);
    threads = (pthread_t*) malloc(sizeof(pthread_t) * shm.tnum);

    shm.bstart = 0;
    shm.bend = 0;
    shm.bcount = 0;
    // 
    // CS537: Create some threads...
    //

    int i;
    for (i = 0; i < shm.tnum; i ++) {
        pthread_create((pthread_t*) threads + i, NULL, consumer, &shm);
    }

    listenfd = Open_listenfd(port);
    while (1) {
	clientlen = sizeof(clientaddr);
	connfd = Accept(listenfd, (SA *)&clientaddr, (socklen_t *) &clientlen);

    producer(connfd, &shm);
	// 
	// CS537: In general, don't handle the request in the main thread.
	// Save the relevant info in a buffer and have one of the worker threads 
	// do the work. 
	// 
    /*
	requestHandle(connfd);

	Close(connfd);
    */
    }

}


    


 
