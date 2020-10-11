#include <stdio.h>
#include <stdlib.h> // atoi
#include <signal.h> // sunartiseis simatwn kai macros
#include <unistd.h> //gia to ALARM, fork
#include <sys/types.h> //gia to dataType pid_t, fork
#include <sys/wait.h>


int usr1=0;
void functionSIGUSR1(int signal){
    usr1=1;
}

int usr2=0;
void functionSIGUSR2(int signal){
    usr2=1;
}

int term=0;
void functionSIGTERM(int signal){
    term=1;
}

void functionSIGINT(int signal){
    term=1;
}

int mypipe=0;
void functionSIGPIPE(int signal){
    mypipe=1;
}

int ala=0;
void functionSIGALRM(int signal){
    ala=1;
}

int troll=0;
void functionSIGQUIT(int signal){
    troll=1;
}
//deiktis se deiktes apo char

int main(int argc, const char * argv[]) {
    int n = argc-1; //arc-1 epeidi to ./ask2 metraei gia 1
    int activeChilds = n;
    int times[n];
    
    for(int i=1;i<=n;i++){
        if (atoi(argv[i])>0) times[i-1]=atoi(argv[i]);
        else{
            printf("Please give me a valid input: possInt1, ... , possIntN \n");
            exit(EXIT_FAILURE);
        }
    }
    printf("\nMaximum execution time of children is set to 100 secs\n\n");
    alarm(100);
    printf("Fathers pid is = %d\n",getpid());
    pid_t pid[n];
    int father=1;
    //mono o pateras einai 1 ta paidia 8a ginoun 0 (voh8htiko)
    for(int i=0;i<n;i++){
        pid[i]=fork();      //EDW GENIOUNTAI TA TEKNA
        if (pid[i]==-1){
            perror("fork");
        }else if (pid[i]==0){
            //sundesei handlers me simata
            signal(SIGUSR1, functionSIGUSR1);
            signal(SIGUSR2, functionSIGUSR2);
            signal(SIGTERM, functionSIGTERM);
            signal(SIGALRM,functionSIGALRM);
            signal(SIGQUIT,functionSIGQUIT);
            signal(SIGCONT,SIG_DFL);//unpause
            signal(SIGSTOP,SIG_DFL);//pause
            father=0;
            printf("[Child Process %d: %d] Was created and will pause!\n",i+1,getpid());
            raise(SIGSTOP);  //TA PAIDIA PARAMENOUN AKINITA EDW MEXRI NA LAVOUN SIMA APO MPAMPA
            
            printf("[Child Process %d: %d] Is starting!\n",i+1,getpid()); //LABAN SIMA KAI SUNEXIZOUN XAROUMENA
            
            int cnt=0;//to ka8e paidi exei to diko tou cntr
            while(1){
                cnt++;
                sleep(times[i]);
                if (usr1){
                    printf("[Child Process %d: %d] Value: %d!\n",i+1,getpid(),cnt);
                    usr1=0;
                }
                if (usr2){
                    printf("[Child Process %d] Echo!\n",getpid());
                    usr2=0;
                }
                if (term){
                    term=0;
                    printf("[Child Process: %d] will die!\n",getpid());
                    kill(getppid(),SIGPIPE); //stelnei sima ston mpampa oti pe8ainei
                    //xrismiotita: otan pe8aineis ena ena ola ta paidia na mporei na teleisei to while loop
                    raise(SIGKILL);
                }
                if(ala){
                    //ta paidia ekteloun to alarm pou exei stal8ei apo ton patera tous, enhmerwnoun kai pe8ainoun
                    ala=0;
                    printf("[Child process %d: %d] Time Expired! Final Value: %d.\n",i+1,getpid(),cnt);
                    exit(-1);
                }
                if(troll){
                    //for fun
                    //auti i energeia kaleitai otan o FATHERS SIGTERM  kai kalei se ola ta zontana paidia tou auto
                    //uparxei enhmerwsh kai meta pai8ainoun.
                    troll=0;
                    printf("[Father process %d] Will terminate child procese: %d: %d.\n",getppid(),i+1,getpid());
                    exit(-1);
                }
            }
            exit(0);
        }
    }
    
    if (father){
        sleep(1); //auto to vazoume gia na eimaste sigouroi oti prwta 8a ginei o pateras
        signal(SIGUSR1, functionSIGUSR1);
        signal(SIGUSR2, functionSIGUSR2);
        signal(SIGTERM, functionSIGTERM);
        signal(SIGINT, functionSIGTERM);
        signal(SIGPIPE, functionSIGPIPE);
        signal(SIGALRM,functionSIGALRM);
        
        for (int i=0;i<n;i++)
            kill(pid[i],SIGCONT); //STELNEI SIMA O MPAMPAS NA SUNEXISOUN
        
        while(activeChilds>0){
            sleep(100); //auto to valame gia na mhn trexei askopa to loop, kai to signal to diazeirizetai mia xara
            //na kanw mia global metavliti kai opote pe8ainei ena paidi na thn meiwnw gia na vgainw apo to while(1)
            if (usr1) {
                printf("[Father process: %d] Will ask current values (SIGUSR1) from all childrent processes.\n",getpid());
                for (int i=0; i<n;i++)
                    kill(pid[i],SIGUSR1);
                usr1 = 0;
            }
            if (usr2) {
                printf("[Father Process: %d] Echo!\n",getpid());
                usr2 = 0;
            }
            if (term){
                term=0;
                for(int i=0;i<n;i++){
                if (kill(pid[i],0)==0) {
                    //kalei se ola ta zontana paidia tou tin diadikasia tou
                    kill(pid[i],SIGQUIT);
                }
                }
                sleep(1);
                printf("[Father process: %d] Father process will die now.Bye!\n",getpid());
                raise(SIGKILL);
            }
            
            if (mypipe){
                //o mpampas ma8ainei oti exase ena paidi kai meinwei ta energa paidia kata ena
                //otan meinei xwri paidia vgainei apo to while loop :auto to vlepoyme apo to while(activeChilds>0)
                mypipe=0;
                activeChilds--;
            }
            if(ala){
                //stelnei sima se ola ta paidia ena pros ena!
                for (int i=0; i<n;i++)
                    kill(pid[i],SIGALRM);
                break;
            }
            
        }
        //for (int i=0;i<n;i++) wait(NULL);
    }
  
    return 0;
}
