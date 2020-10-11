/////////////////////////////////////////////////////////////////////////////
// Author            : Stefanos-Stamatis Achlatis <sachlatis@gmail.com>
// Description       : Askisi 4 Leitourgika Systimata, tmima k.Tsanaka
// Last Update       : 03-06-2020
/////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <string.h>         // aparaititi gia thn synartisi strncmp
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>

#define STREQUAL(x, y) (strncmp((x), (y), strlen(y) ) == 0 )
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define BLUE "\033[34m"
#define RED "\033[31;1m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"

#define BUFFER_SIZE 64

int getMode = 0;  // ginetai ena otan egrapsa sto termatiko "get" kai perimenw apo server sata
int codeTime = 0; // ginetai ena otan o server exei steilei to code "ddf609ca447dce" kai perimenei na to grapsw sto terminal
int ackTime = 0;  // ginetai ena otan o xristeis egrapse to code kai perimenei o server na steilei to ACK
int sendRequestForWalk = 0; // ginetai ena otan o xristeis grapsei sto termatiko kati:
                            // pou einai la8wos (error checking o server) h aitima gia voltoula

int main(int argc, char **argv){
    
    // set default host,port,debugMode
    char host[256];
    strcpy(host, "tcp.akolaitis.os.grnetcloud.net");
    int port = 8080;
    int debugMode = 0;
    
    // check if set to not default parametrs aslo error cheching of the parametrs
    if (argc > 1){
        
        int foundError = 0;
        int changedParam = 0;
        // channgedParam is for detectinf errors like: ./ask4 peinaw
        for(int i=0; i<argc; i++){
            if (STREQUAL(argv[i],"--host") == 1){
                if (argc > i+1 && STREQUAL(argv[i+1],"--host") == 0 && STREQUAL(argv[i+1],"--port") == 0
                    && STREQUAL(argv[i+1],"--debug") == 0){
                   
                    strcpy(host, argv[i+1]);
                    changedParam = 1;
                }
                else foundError = 1;
            }
            
            if (STREQUAL(argv[i],"--port") == 1){
                // to argc >i+1 pianei error tis morfhs sketo --host
                if (argc > i+1 && STREQUAL(argv[i+1],"--host") == 0 && STREQUAL(argv[i+1],"--port") == 0
                    && STREQUAL(argv[i+1],"--debug") == 0){
                    
                    port = atoi(argv[i+1]);
                    if (port <1024)
                        foundError = 1;
                    else
                        changedParam = 1;

                }
                else foundError = 1;
            }
            if (STREQUAL(argv[i],"--debug")){
                debugMode = 1;
                changedParam = 1;
            }
        }
        if(foundError == 1 || argc > 6 || changedParam == 0){
            printf(RED"Something went wrong! :-(\n");
            printf("Make sure that your command looks like: ./ask4 [--host HOST] [--port PORT] [--debug]\n");
            printf("Also please make sure that your HOST is smaller than 256 chars and the PORT is not a well known port\n");
            printf("Otherwise contact us at: sachlatis@gmail.com"WHITE"\n");
            exit(EXIT_FAILURE);
        }
    
    }
    
    // for tasting perpuses
    // printf("The host is: ");
    // printf("%s\n",host);
    // printf("The port is: ");
    // printf("%d\n",port);
    // printf("The debugMode is: ");
    // printf("%d\n",debugMode);
    
    
    // creation of client's socet dhmiourgia socket
    
    int domain = AF_INET; // internet domain
    int type = SOCK_STREAM; // orizw protocol tcp kai oxi udp pou ta petaei ola kai ginetai xamos
    int sock_fd = socket(domain, type, 0); // arxikopoio to socket mou, prosoxi to 8080 einai to socket tou aggelou
                                           // to diko m einai kapoio apo 1024 kai panw
                                          // protocol =0 let the os choose the protocol panw sto kanali epikoinwnias
    if (sock_fd < 0) {  // error checking
        perror("socket");
        return -1;
    }
    
    // creation of bind onomatodosia socket OXI APARAITITO
    
    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons(0); /* Let the system choose */
    sin.sin_addr.s_addr = htonl(INADDR_ANY); // ip mixanhw p dexetai sundseis to socket
                                            // an mia mixani einai sundedemenoi tautoxrona se polla diktia tote to socket
                                            // mporei na prosdioristei apo poio diktio mono 8a dexete sindeseis
                                            // INADDR_ANY dexetai sundeseis apo pantou
    if (bind(sock_fd, (struct sockaddr *) &sin, sizeof(sin)) < 0){
        perror("bind");
        return -1;
    }
    
    // creation of connection syndesh
    
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port); // port number pou akouei o server. 8080
    struct hostent *hostp = gethostbyname(host);  //vazw hostname kai thn fun epeidi 8elw network byte order
    bcopy(hostp->h_addr, &sa.sin_addr, hostp->h_length); // kai meta to kanw copy sto sa.sin_addr
    printf(BLUE"Connecting!"WHITE"\n");
    if (connect(sock_fd, (struct sockaddr *) &sa, sizeof(sa)) < 0){ //error checiking
        printf("oups");
        perror("connect");
        return -1;
    }
    printf(GREEN"Connected to %s:%d"WHITE"\n",host,port); //ola komple sunexizoume
    
    
    while (1) {
        fd_set inset;
        int maxfd;
        
        FD_ZERO(&inset);                // we must initialize before each call to select
        FD_SET(STDIN_FILENO, &inset);   // select will check for input from stdin
        FD_SET(sock_fd, &inset);          // select will check for input from pipe
        
        // select only considers file descriptors that are smaller than maxfd
        maxfd = MAX(STDIN_FILENO, sock_fd ) + 1;
        
        // wait until any of the input file descriptors are ready to receive
        int ready_fds = select(maxfd, &inset, NULL, NULL, NULL);
        if (ready_fds <= 0) {
            perror("select");
            continue;                                       // just try again
        }
        
        // user has typed something, we can read from stdin without blocking
        if (FD_ISSET(STDIN_FILENO, &inset)) {
            char buffer[101];
            int n_read = read(STDIN_FILENO, buffer, 100);   // error checking!
            if (n_read == -1) {
                close(sock_fd);
                perror("read");
                exit(-1);
            }
            buffer[n_read] = '\0';                          // why?
            
            // New-line is also read from the stream, discard it.
            if (n_read > 0 && buffer[n_read-1] == '\n') {
                buffer[n_read-1] = '\0';
            }
            
            //printf(BLUE"Got user input: '%s'"WHITE"\n", buffer);
            
            if (n_read >= 4 && strncmp(buffer, "exit", 4) == 0) {
                close(sock_fd);
                exit(0);
            }
            else if (n_read >= 5 && strncmp(buffer, "help", 4) == 0){
                printf(YELLOW"Available commands:\n");
                printf("* 'help'                     : Print this help message\n");
                printf("* 'exit'                     : Exit\n");
                printf("* 'get'                      : Retrieve sensor data\n");
                printf("* 'N name surname reason'    : Ask permission to go out"WHITE"\n");
            }
            else if (n_read >= 4 && strncmp(buffer, "get", 3) == 0){
                
                getMode = 1;
                char buffer[4]="get";
                int n_write = write(sock_fd, buffer, 4);
                if (n_write == -1) {
                    close(sock_fd);
                    perror("write");
                    exit(-1);
                }
                if (debugMode==1) printf(YELLOW"[DEBUG] sent 'get'"WHITE"\n");
            }
            else if (codeTime == 1){
                codeTime = 0;
                if (debugMode==1) printf(YELLOW"[DEBUG] sent '%s'"WHITE"\n",buffer);
                int n_write = write(sock_fd, buffer, n_read);
                if (n_write == -1) {
                    close(sock_fd);
                    perror("write");
                    exit(-1);
                }
                ackTime = 1;
            }
            else {
                int sendRequestForWalk = 1;
                int n_write = write(sock_fd, buffer, n_read);
                if (n_write == -1) {
                    perror("write");
                    exit(-1);
                }
                if (debugMode==1) printf(YELLOW"[DEBUG] sent '%s'"WHITE"\n",buffer);
            }
            
        }
        
        // an perasei to if tote o server m egrapse kati! we can read without blocking
        if (FD_ISSET(sock_fd, &inset)) {
            
            char buffer2[101];
            int n_read = read(sock_fd, buffer2, 100);
            if (n_read == -1) {
                perror("read");
                exit(-1);
            }
            buffer2[n_read] = '\0';                          // why?
            
            // New-line is also read from the stream, discard it.
            if (n_read > 0 && buffer2[n_read-1] == '\n') {
                buffer2[n_read-1] = '\0';
            }
            
            //if (n_read >= 20){
            if (getMode==1){
                getMode = 0;
            if (debugMode==1) printf(YELLOW"[DEBUG] read '%s'"WHITE"\n",buffer2);
            
            printf(BLUE"---------------------------\n");
            printf("Latest event:\n");
            int a = atoi(buffer2);
            if (a==0) printf("boot (0)\n");
            else if (a==1) printf("setup (1)\n");
            else if (a==2) printf("interval (2)\n");
            else if (a==3) printf("button (3)\n");
            else if (a==4) printf("motion (4)\n");
            
            char tempBuffer[2];
            for(int i=6;i<8;i++){
                tempBuffer[i-6]=buffer2[i];
            }
            printf("Temperature is: %s.",(tempBuffer));
            
            char tempBufferDec[2];
            for(int i=8;i<10;i++){
                tempBufferDec[i-8]=buffer2[i];
            }
            printf("%s\n",(tempBufferDec));
            
            char lumin[3];
            for(int i=2;i<5;i++)
                lumin[i-2] = buffer2[i];
            printf("Light level is: %d\n",atoi(lumin));
            
            char mytime[10];
            for(int i=11;i<22;i++){
                mytime[i-11] = buffer2[i];
            }
            //printf("%s\n",mytime);
            int t = atoi(mytime);
            //printf("edwww %d\n",t);
            time_t rawtime=t;
            struct tm *info;
            info = localtime(&rawtime); // kanei unix time local time
            printf("Timestamp is: %s",asctime(info)); // kanei to local time visible apo ton an8rwpo
            }
            else if (ackTime==1){
                if (debugMode==1) printf(YELLOW"[DEBUG] read '%s'"WHITE"\n",buffer2);
                printf(GREEN"Response: '%s'"WHITE"\n",buffer2);
                ackTime = 0;

            }
            else{
                int sendRequestForWalk = 0;
                if (debugMode==1) printf(YELLOW"[DEBUG] read '%s'"WHITE"\n",buffer2);
                //printf("%s\n",buffer2);
                if (strncmp(buffer2, "try again", 9) != 0){
                    printf(BLUE"Send verification code: '%s'"WHITE"\n",buffer2);
                    codeTime = 1;
                }
                
            }
            
        }
    }
    
    return 0;
}
