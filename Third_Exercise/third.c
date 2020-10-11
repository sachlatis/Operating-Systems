#include <sys/types.h>
 #include <sys/select.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<signal.h>
#include<sys/wait.h>
#include<math.h>
#include <limits.h>
#include <time.h>
#include <ctype.h>
#include <string.h>
// aparaititi gia thn synartisi strncmp
#define STREQUAL(x, y) (strncmp((x), (y), strlen(y) ) == 0 )
#define read_end 0
#define write_end 1
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define BLUE "\033[34m"
#define MAGENTA "\033[35m"
#define WHITE "\033[37m"
#define YELLOW "\033[33m"
#define GREEN "\033[32m"

int randomNum(int lower, int upper){
    /* Intializes random number generator */
    
    //time(0): gives the time in seconds since the Unix epoch: 1589831161 deutera 18/05 stis 11 to vradu
    //you're guaranteed your seed will be the same only once,
    //unless you start your program multiple times within the same second
    // srand seeds the rand with seed:time(0)
    srand(time(0));
    int num = (rand() %
               (upper - lower + 1)) + lower;
    return num;
}


int main(int argc, char **argv){
    int n = atoi(argv[1]);
    int mode=0; //default mode set to 0->round-robin
    int cnt=0; //cnt gia to round-robin
    
    //------Checking Sestion-------//
    if (n<=0){
        perror("Usage: %s <nChildren> [--random] [--round-robin]");
        return -1;
    }//first input not a num or not a valid num
    if (argc > 3 || argc < 2){
        perror("Usage: %s <nChildren> [--random] [--round-robin]");
        return -1;
    }// more or less inputs that expected
    
    if (argc==3){
        if (STREQUAL(argv[2],"--round-robin"))
            mode=0;
        else if (STREQUAL(argv[2],"--random"))
            mode=1;
        else {
            perror("Usage: %s <nChildren> [--random] [--round-robin]");
            return -1;
        }
    }
    //------ End of Checking Sestion-------//
    
    pid_t pid[n];   //
    
    int in[n][2]; // mpainei sto paidi me ta data poy prepei na douylepsei to paidi
    int out[n][2]; // ta data poy stelnoun ta paidia ston father
    for(int i=0;i<n;i++){
        if (pipe(in[i])!=0){
            perror("pipe");
            return -1;
        }
        if (pipe(out[i])!=0){
            perror("pipe");
            return -1;
        }
    }
    for(int i=0;i<n;i++){
        pid[i]=fork();      //edw geniountai ta paidia
        if (pid[i]==-1){
            perror("fork");
            return -1;
        }
        else if (pid[i]==0){
            int val;
            // eimai paidi kai den xrieazomai apo ta pipes
            // to akro write tou in pipe
            // oute to akri read tou out pipe
            for(int i=0; i<n; i++){
                close(in[i][1]);
                close(out[i][0]);
            }
            
            while(1){
                read(in[i][0], &val, sizeof(int)); //"koptiki" sunarthsh perimenoun ta data
                printf(BLUE"[Child %d] [%d] Child received %d!"WHITE"\n", (i+1), getpid(), val);
                val++;
                sleep(5);
                write(out[i][1], &val, sizeof(int));
                printf(GREEN"[Child %d] [%d] Child Finished hard work, writing back %d!"WHITE "\n", (i+1),getpid(),val);
            }
        }
    }
    while (1) {
        //einai kwdikas kai twn duo alla praktika logw tou while loop twn paidiwn ta paidia den mpainoun pote edw
        //kleinw akra pipe pou den 8elei o pateras
        for(int i=0; i<n; i++){
            close(in[i][0]);
            close(out[i][1]);
        }
        // o father diavazei pipes kai terminal asugrxronaa araaa select
        fd_set inset;
        // o filed descr set
        int maxfd;
        FD_ZERO(&inset);
        //This macro clears (removes all file descriptors from) fd set -> to inset.
        FD_SET(STDIN_FILENO, &inset);
        //vazei to stdin sto termatiko
        //STDIN_FILENO sxedon sigoura iso me 0 kai einai o fd tou stdin
        //vriskw max fd twn pipe out[i][0]
        int maxPipeFdNum = 0;
        for(int i=0;i<n;i++){
            FD_SET(out[i][0], &inset);
            if (out[i][0]>maxPipeFdNum)
                maxPipeFdNum = out[i][0];
        }
        
        // select only considers file descriptors that are smaller than maxfd
        maxfd = MAX(STDIN_FILENO, maxPipeFdNum) + 1; //to +1 einai gia na metraei Kai to max auto
        
        // wait until any of the input file descriptors are ready to receive
        int ready_fds = select(maxfd, &inset, NULL, NULL, NULL);
        if (ready_fds <= 0) {
            perror("select");
            continue;                                       // just try again
        }
        
        //meta sunexws tetsarei (FD_ISSET) an anoikse kapoio arxeio
        
        // user has typed something, we can read from stdin without blocking
        if (FD_ISSET(STDIN_FILENO, &inset)) {
            char buffer[101];
             // Read at most 100 bytes, returns number of bytes read
            int n_read = read(STDIN_FILENO, buffer, 100);
            //n_read the number of bytes poy diavastikan
            // BUFFER_SIZE: The number of bytes to read before truncating the data. If the data to be read is smaller than nbytes, all data is saved in the buffer.
            // fd_in: The file descriptor of where to read the input. You can either use a file descriptor obtained from the open system call, or you can use 0, 1, or 2, to refer to standard input, standard output, or standard error, respectively.
            // void * buffer
            // void * einai generic pointer pou mporei na deixnei se otidipote
            // profanws exei noima na sumbainei kati tetoio edw gt o buffer 8a mporouse na einai int
            // size_t BUFFER_SIZE diavase tosa data
            
            if (n_read == -1) {
                perror("read");
                exit(-1);
            }//kati egine la8os bye!
            //an ola komple:
            buffer[n_read] = '\0';
            
            // New-line is also read from the stream, discard it.
            if (n_read > 0 && buffer[n_read-1] == '\n') {
                buffer[n_read-1] = '\0';
                //kane thn new line NUL
            }
            
            int isNumber = 0;
            char inputNumber[n_read];
            for(int i=0;i<n_read-1;i++){
                if (isdigit(buffer[i])){
                    inputNumber[i]=buffer[i];
                    isNumber = 1;
                }
                else{
                    isNumber = 0;
                    break;
                }
            }
            
            if (n_read == 5 && strncmp(buffer, "exit", 4) == 0) {
                // user typed 'exit', kill child and exit properly
                for(int i=0;i<n;i++){
                        if (kill(pid[i], SIGTERM) == -1){
                            perror("killError");
                            exit(-1);
                        }
                        // error checking!
                        if ( wait(NULL)==-1){
                            perror("waitError");
                            exit(-1);
                        }
                        printf(BLUE"Waiting for %d"WHITE"\n",(i+1));
                }
                printf(GREEN"All children temrinated"WHITE"\n");// error checking!
                exit(0);
            }
            else if (n_read == 5 && strncmp(buffer, "help", 4) == 0){
                printf(BLUE"Type a number to send job to a child!"WHITE"\n");
            }
            else if (isNumber){
                if(mode==1){
                    int child = randomNum(1,n);
                    int num = atoi(inputNumber);
                    printf(MAGENTA"[Parent] Assigned %d to child %d"WHITE"\n",num,child);
                    write(in[(child-1)][1],&num,sizeof(int));
                }
                else{
                    int child = cnt%(n);
                    int num = atoi(inputNumber);
                    printf(MAGENTA"[Parent] Assigned %d to child %d"WHITE"\n",num,(child+1));
                    write(in[child][1],&num,sizeof(int));
                    cnt++;
                }
            }
            else{
                printf(BLUE "Type a number to send job to a child!"WHITE"\n");
            }
        }
        
        for(int i=0;i<n;i++){
        // someone has written bytes to the pipe, we can read without blocking
            if (FD_ISSET(out[i][0], &inset)) {
                int val;
                int n_read_2 = read(out[i][0], &val, sizeof(int));
                if (n_read_2 == -1) {
                    perror("read");
                    exit(-1);
                }// error checking!
                printf(YELLOW "[Parent] Received result from child %d --> %d"WHITE"\n", (i+1), val);
        }
        }
    }
    
    return 0;
}
