#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>


int errnum, len;
int balance = 0;
socklen_t socklen = sizeof(struct sockaddr_in);
int sockfd, new_sock;
// Structs für Ziel und unsere eigene Adresse
struct sockaddr_in addr, client_addr;
pthread_t tid[1], dispatch;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;


void
handle_error_en(int en, char *msg) {
    errno = en;
    perror(msg);
    exit(EXIT_FAILURE);
}


struct thread_info {    /* Used as argument to thread_start() */
    int                 thread_num;       /* Application-defined thread # */
    int                 sockfd;
    struct sockaddr_in  client_addr;      
};


void
*workerThread(void *thread_arg) {

    printf("\nThread %lu started\n", pthread_self());

    pthread_mutex_lock(&lock);
    struct thread_info *t_info = thread_arg;
    struct sockaddr_in client_addr_t = t_info->client_addr;
    int new_sock = t_info->sockfd;
    pthread_mutex_unlock(&lock);

    char msg[64];
    memset(&msg, 0, sizeof(msg));

    char host[NI_MAXHOST], service[NI_MAXSERV];
    socklen_t client_len = sizeof(struct sockaddr);




    if(new_sock < 0) {
            handle_error_en(new_sock, "new_sock");
        } else {
            // recieve first message
            read(sockfd, msg, sizeof(msg));
            getnameinfo((struct sockaddr * ) &client_addr_t, client_len, host, sizeof(host), service, sizeof(service), NI_NUMERICSERV);
            printf("\nThread %lu recieved connection from %s on port %s.\n", pthread_self(), host, service);

            strcpy(msg, "Welcome!");
            len = write(new_sock, msg, sizeof(msg));
            if(len < 64) {
                do {    // resend until all bytes have been sent
                    len = write(new_sock, msg, sizeof(msg));
                } while(len < 64);
            }
            memset(msg, 0, sizeof(msg));
        }

        do {
            // Kontostand an Client senden
            printf("Sending current balance of %i € to client %s\n", balance, host);
            // Schreibe aktuellen Kontostand in den Buffer
            sprintf(msg, "%d", balance);

            // Sende Kontostand an Client
            len = write(new_sock, msg, sizeof(msg));

            // Prüfe ob alle Bytes versendet wurden
            if(len < 64) {
                printf("Es wurden nur %i von %lu Bytes versandt. Sende erneut.\n", len, sizeof(msg));
                do {    // Wir senden solange, bis alle Bytes versendet wurden.
                    len = write(new_sock, msg, sizeof(msg));
                } while(len < 64);
            } else printf("Erfolgreich versandt\n");
            memset(msg, 0, sizeof(msg));


            // Operation vom Client lesen
            len = read(new_sock, msg, sizeof(msg));
            printf("\nLese %i Bytes\n", len);


            // Kontostand anpassen mit den empfangenen Daten
            pthread_mutex_lock(&lock);
            balance = balance + (int) strtol(msg, NULL, 10);
            pthread_mutex_unlock(&lock);
        } while(len > 0);

        printf("\nClient %s disconnected\n", host);
        
    return NULL;
}


// the dispatcher handles incoming connections and creates worker threads for each one
void
*dispatcherThread(void *thread_arg) {

    printf("Dispatcher started\nWaiting for connections...\n");
    
    pthread_attr_t attr;
    errnum = pthread_attr_init(&attr);
    if(errnum != 0)
        handle_error_en(errnum, "pthread_attr_init");

    // set detach state to true, so worker threads clean up after themselves
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    // create thread info for worker threads
    struct thread_info t_info;
    memset(&t_info, 0, sizeof(struct thread_info));

    // main loop for handling incoming connections
    while(1) {
        int i = 0;
        new_sock = accept(sockfd, (struct sockaddr *) &client_addr, &socklen);

        t_info.sockfd = new_sock;
        t_info.client_addr = client_addr;
        t_info.thread_num = i;

        if(pthread_create(&tid[i++], &attr, workerThread, &t_info) != 0) {
            printf("Work thread creation failed!\n");
        }

        sleep(1);
    }


    // destroy unneeded thread attributes
    errnum = pthread_attr_destroy(&attr);
    if (errnum != 0)
        handle_error_en(errnum, "pthread_attr_destroy");
    
    return NULL;
}


int
main(int argc, char *argv[]) {

    // check args
    if(argc == 2) { // only 2 args allowed (program name, service)
        if(strtol(argv[1], NULL, 0) > 65535 || strtol(argv[1], NULL, 0) < 1025) {
            fprintf(stderr, "%s: Invalid Arguments\nUsage: %s [1025 - 65535]\n", argv[0], argv[0]);
            exit(EXIT_FAILURE);
        }
    }
    else {
        fprintf(stderr, "%s: Invalid Arguments\nUsage: %s [1025 - 65535]\n", argv[0], argv[0]);
        exit(EXIT_FAILURE);
    }


    // initialize address structs
    memset(&addr, 0, sizeof(struct sockaddr_in));
    memset(&client_addr, 0, sizeof(struct sockaddr_in));


    // create the socket for the server
    printf("Initializing socket...\n");
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0) {
        perror("Couldn't create socket: ");
        exit(EXIT_FAILURE);
    }

    // set the IP address and get the service from args
    printf("Initializing address...\n");
    addr.sin_family = AF_INET;  // currently only IPV4 support
    addr.sin_port = htons((unsigned short) strtol(argv[1], NULL, 0));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);


    // bind the socket
    printf("binding socket...\n");
    errnum = bind(sockfd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in));
    if(errnum < 0) {
        fprintf(stderr, "IP-Adresse konnte nicht angebunden werden: %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }


    // Holen uns die uns zugewiesene Adresse ab und geben den Port aus
    errnum = getsockname(sockfd, (struct sockaddr *) &addr, &socklen);
    if(errnum < 0) {
        fprintf(stderr, "Problem bei getsock(): %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }
    printf("\nSuccess!\nThe server is running on port %hu\n", ntohs(addr.sin_port));


    // set socket into passive listen state
    errnum = listen(sockfd, 5);
    if(errnum < 0) {
        fprintf(stderr, "Problem bei listen(): %s\n", strerror(errnum));
        exit(EXIT_FAILURE);
    }

    // create dispatch thread which handles incoming connections
    pthread_create(&dispatch, NULL, dispatcherThread, NULL);
    // main program is done, wait for dispatcher
    pthread_join(dispatch, NULL);

    return 0;
}