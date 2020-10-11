#include <stdio.h>
#include <stdlib.h>//atoi
#include <unistd.h>//read, write, gia to dataType pid_t
#include <sys/types.h>//gia to dataType pid_t
#include <sys/stat.h>
// ta anw duo mporei na xrieazontai se diaforetika unix gia ta read,write
#include <sys/wait.h>
//gia to wait()
#include <fcntl.h>
//aparaititi gia to O_RDONLY, O_WRONLY, O_APPEND,open()

#include <string.h>
// aparaititi gia thn synartisi strncmp
#define STREQUAL(x, y) (strncmp((x), (y), strlen(y) ) == 0 )

// sigkrinei num = strlen(y) characters tou string x me num = strlen(y) characters to string y
// an kai oi strlen(y) char einai idioi epistrefei true
//epistrefei: <0 if the first character that does not match has a lower value in str1 than in str2
// epistrefei: 0 an ta x kai y einai idia ara tote epistrefei h STREQUAL 1


#define BUFFER_SIZE 64
#define FD_STDOUT 1
//to unix diavazei ta panta ws arxeia, to standard output exei ton kwdiko 1 sto write()

typedef enum {
    ENCRYPT,
    DECRYPT
} encrypt_mode;

char caesar(unsigned char ch, encrypt_mode mode, int key)
{
    if (ch >= 'a' && ch <= 'z') {
        if (mode == ENCRYPT) {
            ch += key;
            if (ch > 'z') ch -= 26;
            //auto einai gia ta kefalaia
        } else {
            ch -= key;
            if (ch < 'a') ch += 26;
        }
        return ch;
    }
    
    if (ch >= 'A' && ch <= 'Z') {
        if (mode == ENCRYPT) {
            ch += key;
            if (ch > 'Z') ch -= 26;
        } else {
            ch -= key;
            if (ch < 'A') ch += 26;
        }
        return ch;
    }
    
    return ch;
}

int validNum(int num){
    if (num >= 0 && num <= 25) return 1;
    else return 0;
}
//argv is a pointer to an aray of pointers to char
//./main --input FILEPath --key NUMBER
//argc:
//  0       1      2         3     4
int main(int argc, const char * argv[]) {
    
    int foundError = 0;
    // o pateras elegxei ola ta errors ektos apo
    // to an uparxei to filepath pou prepei na diavasei.
    // auto to kanei to C1
    int isValid1 = STREQUAL(argv[1],"--input");
    int isValid3 = STREQUAL(argv[3],"--key");
    if  (!isValid1 || !isValid3){
        foundError = 1;
        printf("Please give me a valid input: [./task --input FILENAME --key NUMBER]\n");
    }
    
    int key = atoi(argv[4]);
    // Auto to kanw gia na einai prosvasimo to key kai apo to key kai apo ta paidia)CH1,CH2) kai apo ton patera gia che
    int isValid4 = validNum(key);
    if (!isValid4){
        foundError = 1;
        printf("Please give me a valid number:0-25 as the fifth argument\n");
    }
    
    if (foundError) exit(EXIT_FAILURE);
    // an kati den egrapses kala sto termatiko exit
    // to EXIT_FAILURE einai ena macro pou mpainei os argument sto exit kai dilwnei oti
    // to programma den termatise opws 8w eprepe.
    
    //an den exeis kanei kapoio la8ws sunexise
    else{
    
    int status1;
    pid_t pid1 = fork();
    if (pid1 == -1){
        perror("fork");
    }else if (pid1 == 0 ){
        //child1 code, kriptografw to arxeio
        int n_read,n_write;
        char buffer[BUFFER_SIZE];
         //int open(const char *path, int oflags);
        // const char *path: Edw valame relative,The relative or absolute path to the file that is to be opened.
        //O_RDONLY (int) apla diavase apo ekei
        // epistrefei enan akeraio The open() function shall return a file descriptor for the named file that is the lowest file descriptor not currently open for that process.
        //suxna epestrefe to 3 giati???
     
        int fd_in = open(argv[2], O_RDONLY);
        if (fd_in == -1) {
            printf("Please give me a wroking filepath as the third argument\n");
            perror("open");
            exit(-1);
        }
     
        //an uparxei to arxeio grapse ekei
        //alliws dimiourghse to arxeio kai onomaseto encrypted kai grapse se auto
        //to 0644 einai to mode ths O_CREAT
        //einai se oktadiko systima to chmod 0644 dilwnei ta ekseis:
        // User can read,write cant execute
        // Group can read cant write cant execute
        // Othes can read cant write and cant exceute
        
        int filedesc = open("encrypted.txt", O_WRONLY | O_CREAT, 0644);
        // open Returns the file descriptor for the new file. The file descriptor returned is always the smallest integer greater than zero that is still available. If a negative value is returned, then there was an error opening the file.
        //printf("           %d\n",filedesc);
        // typwse ton ari8mo 4
        if (filedesc < 0) {
            return -1;
        }
        
        do {
            // Read at most BUFFER_SIZE bytes, returns number of bytes read
            n_read = read(fd_in, buffer, BUFFER_SIZE);
            
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
            }
            
            for(int i = 0; i<BUFFER_SIZE;i++){
                buffer[i] = caesar(buffer[i], ENCRYPT, key);
            }
            
            // Write at most n_read bytes:
            //PROSOXI den grafw na grafei BUFFER_SIZE giati mporei
            // sthn periptosi pou mia read epistepei 34 byte
            // ta upoloipa 64-34 byte 8a einai skoupidia apo to proigoumeno call tis read
            n_write = write(filedesc, buffer, n_read);
            if (n_write < n_read) {
                perror("write");
                exit(-1);
            }
        } while (n_read > 0); // kanto oso exeis data na diavaseiw apo to arxeio, mpore
        //printf("CHILD1: My pid is %d, my father is %d\n", getpid(), getppid());
        int c1 = close(fd_in);
        if (c1 < 0)
        {
            perror("c1");
            exit(-1);
        }
        int c2 = close(filedesc);
        if (c2 < 0)
        {
            perror("c2");
            exit(-1);
        }
        //kleinw ta arxeia pou anoiksa
        exit(0);
    }else{
        //fathers code
        wait(&status1); //edw perimenei na termatisei to paidi Child1?
        int status2;
        pid_t pid2 = fork();
        if (pid2 == -1){
            perror("fork");
        }else if (pid2 == 0){
            //Child2 code,
            //apokriptografw to arxeio
            
            int n_read,n_write;
            char buffer[BUFFER_SIZE];
            
            int fd_in = open("encrypted.txt",O_RDONLY);
            if (fd_in == -1) {
                perror("open");
                exit(-1);
            }
            do {
                // Read at most BUFFER_SIZE bytes, returns number of bytes read
                n_read = read(fd_in, buffer, BUFFER_SIZE);
                if (n_read == -1) {
                    perror("read");
                    exit(-1);
                }
                
                for(int i = 0; i<BUFFER_SIZE;i++){
                    buffer[i] = caesar(buffer[i], DECRYPT, key);
                }
                
                // Write at most n_read bytes (why?), returns number of bytes written
                n_write = write(FD_STDOUT, buffer, n_read);
                if (n_write < n_read) {
                    perror("write");
                    exit(-1);
                }
            } while (n_read > 0); // (why?)
            
            int c3 = close(fd_in);
            if (c3 < 0)
            {
                perror("c3");
                exit(-1);
            }
            //kleinw to arxeio pou anoiksa
            //8a mporousa kai na to diagrapsw
            //printf("CHILD2: My pid is %d, my father is %d\n", getpid(), getppid());
            exit(0);
        }else{
            //fathers code
            //printf("PARENT: My pid is %d, my father is %d\n", getpid(), getppid());
            wait(&status2);//edw perimenei na termatisei to paidi Child2?
            //sigoura edw 8a exoun termatisei kai ta duo paidia m ara den 8a exw kapoio zombi
        }
        
    }
        return 0;
    }
}
