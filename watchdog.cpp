#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <csignal>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
#include <string>


using namespace std;

/// Time unit to make watchdog process sleep when neccessary
struct timespec delta = {0 /*secs*/, 300000000 /*nanosecs*/}; //0.3 sec

void terminationSignal(int);

/// Helper map that helps watchdog process to keep track of the all pids and ids of its children
/// This map named m has keys consisting of pids and has values consisting of corresponding ids
map<long,int> m;
/// Helper map that helps watchdog process to keep track of the all pids and ids of its children
/// This map named mp_id_to_pid has keys consisting of ids and has values consisting of corresponding pids
map<int, pid_t> mp_id_to_pid;
/// address of wacthdog output
char * watchdog_output;
/// process id of the watchdof process
pid_t watchdogPID;
/// Control variable that helps watchdog process to terminate
bool ext=false;


///@author Mustafa Can Aydin
///@param argc Number of arguments watchdog takes
///@param argv[] Vector of arguments that the watchdog process takes, main() will take three arguments one showing number of processes that we are planning to create and two process_output.txt and three watchdog_output.txt with their addresses
///@note Main ideo of the project is to implement a process which will create children processes and will keep track of their status, well-being , etc. For instance, if some of the children were to watchdog process should know about it, prints out neccessary information to its output file, and restarts the children processes for the duration of the program. After all the signals handled and there are no more instructions to execute watchdog process kills itself and terminates gracefully. If watchdog process were to die before the instructions have not been fully executed, children processes will not be killed by watchdog process and watchdog process will print to its output a message informing that it is terminating and sends itself the termination signal.
///@note To sum it up, watchdog project implements a main process called watchdog process which will crete some children and will know all the information about them and is responsible for restarting them and printing neccessary information about them for the duration of instructions. After the instructions completed it will terminate gracefully.
int main(int argc, char* argv[]){
   
   
    int num_of_process,i, fd;
    char buf[30];
    pid_t childpid =0;
    // pid of the process of watchdog
    watchdogPID=getpid();

    // creating the namedPipe
	char* myfifo = (char*) "/tmp/myfifo";
    mkfifo(myfifo, 0666);

    // In order to clean watchdog_out.txt folder at hte start we open a initial folder in *w* mode instead of *a*
    FILE *fptr;
    watchdog_output = argv[3];
    fptr= fopen(watchdog_output, "w");
    char * process_output = argv[2];
    // In order to clean process_out.txt folder at hte start we open a initial folder in *w* mode instead of *a*
    FILE *fptrP;
    fptrP= fopen(process_output, "w");

    // first we need to check whether or not we have valid number of command line arguments
    // check for valid number of command-line arguments if it is not valid print to the console
    if (argc != 4){   /*      */
            fprintf(stderr, "Number of arguments is not valid! %s\n", argv[0]);
            return 1;
    }
	
    // Assign first argument to the number of processes
    num_of_process = atoi(argv[1]);
    
        // this is the part that we create child processes from the watchdog process itself
        for (i = 1; i <= num_of_process; i++) {
            if ((childpid = fork())==0) { //if enter that means it's childs copy
                m[(long) getpid()]= (i%(num_of_process+1));
                break;
            } else {//if enters here we know we execute the parent
                //and we assign child's pid to the parent now
                //now we need to write name of the pipe as
                
                m[(long) childpid]= (i%(num_of_process+1));
                mp_id_to_pid[(i%(num_of_process+1))] = childpid;
                nanosleep(&delta, &delta);
                nanosleep(&delta, &delta);
                nanosleep(&delta, &delta);
               
                
            }
        }
    pid_t headprocess_pid=0;
    if(i==1){
        headprocess_pid = getpid();
    }
            //  If process being executed is watchdog it prints to the watchdog_out.txt file that the child processes has started and prints their pids
            if(getppid()!=watchdogPID){
                for(int k=1; k<=num_of_process;k++){
                    FILE * fptr1;
                    fptr1 = fopen(watchdog_output, "a");
                    fprintf(fptr1,"P%d is started and it has pid %d\n",  k , mp_id_to_pid[k]);
                    fclose(fptr1);
                }
            }
            // If the process being executed is one of the child process than we must send it to 'process' executable program using execv() command
            else if (getppid()==watchdogPID){
                
                stringstream ss1;
                ss1 << i;
                string myString = ss1.str();
                //char* char_type = (char*) myString.c_str();
                char *char_type = &myString[0];
                char* char_array_for_process_executable =(char*) "./process" ;
                char *args[]={char_array_for_process_executable,process_output,char_type,NULL};
                execv(args[0],args);
                
            }
            
    // After checking that the process being executed is watchdog, watchdog process opens FIFO and writes processes with their ids and pids
    if(getppid()!=watchdogPID){
        mp_id_to_pid[0]=getpid();
        m[getpid()]=0;
        
            for(int h=0; h<=num_of_process; h++){
                fd=open(myfifo,O_WRONLY);
                snprintf(buf, 30, "P%d %ld" , h,(long) mp_id_to_pid[h]); // puts string into buffer
                write(fd, buf, 30); //strlen(buf)+1
                
            }
        
        
    }
   
    signal(SIGTERM, terminationSignal);
        
    pid_t killedChild;
    // Infinitite for-loop which will exit if the boolean variable *ext* equals to true
    for(;;){
         sleep(10);
        if (ext==true) {
            break;
        }
        // If any child process died watchdog must print necessary thing to its output file
        // In order to print it, watchdog reaps the killed child and prevents it from becoming a zombie process. Also wait() returns pid of the killed child so that
        // watchdog can print its id and pid to its output file.
        if((killedChild=wait(NULL))>0){
            int killedCIndex= m[killedChild];
            
            // Since id 1 corresponds to head process watchdog process must kill all the other child processes and restarts them
            if(m[killedChild]==1){
            	
                // We need to write here that  head process has died
                FILE* fdie;
                fdie = fopen(watchdog_output,"a");
                fprintf(fdie, "P%d is killed, all processes must be killed\nRestarting all processes\n",killedCIndex);
                fclose(fdie);
                int y;
                // all the child processes must be killed
                for(int u=2; u<=num_of_process;u++){
                    kill(mp_id_to_pid[u], SIGTERM);
                    nanosleep(&delta, &delta); //sinyallerin ard arda gitmesi sirali olmelerini sagliyor
                    
                }
                // In this part we restart all the killed child processes form watchdog process
                for (y = 1; y <= num_of_process; y++) {
                    if ((childpid = fork())==0) { //if enter that means it's childs copy
                        break;
                    } else {//if enters here we know we execute the parent
                        //and we assign child's pid to the parent now
                        //now we need to write name of the pipe as
                      	nanosleep(&delta,&delta);
                        m[(long) childpid]= (y%(num_of_process+1));
                        mp_id_to_pid[(y%(num_of_process+1))] = childpid;
                        char  bufferForRestartedProcess[30];
                        snprintf(bufferForRestartedProcess, 30, "P%d %ld" , (y%(num_of_process+1)),(long)childpid); // puts string into buffer
                        write(fd, bufferForRestartedProcess, 30); //writes the input to the fifo
                        
                        FILE* fdie2;
                        fdie2 = fopen(watchdog_output,"a");
                        fprintf(fdie2, "P%d is started and it has a pid of %ld\n",y%(num_of_process+1),(long) childpid);
                        fclose(fdie2);
                    }
                }
                
                // If the process is child process we must send it to *process* program using execv() command
                if(childpid==0){
                    //here all the children should be sent to execv() call
                    stringstream ss5;
                    ss5 << y;
                    string myString5 = ss5.str();
                    char* char_type5 = (char*) myString5.c_str();
                    char *args[]={(char*)"./process",process_output,char_type5,NULL};
                    execv(args[0],args);
                }
                // we now need to release killed children and prevent them from becoming zombie processes.
                // Since head child has already reaped we need to reap only other killed children
                for(int w=2; w<=num_of_process; w++){wait(NULL);}
            }
            // Here we know that killed child is not the head process to we will print its name to the output file of watchdog amd restart it
            else{
            FILE* fdie3;
            fdie3 = fopen(watchdog_output,"a");
            fprintf(fdie3, "P%d is killed\n",killedCIndex);
                fclose(fdie3);
                
            pid_t temp_pid= fork();
            if (temp_pid==0){
                stringstream ss2;
                ss2 << killedCIndex;
                string myString2 = ss2.str();
                char* char_type2 = (char*) myString2.c_str();
                char *argsq[]={(char*)"./process",process_output,char_type2,NULL};
                execv(argsq[0],argsq);
            }
            else{
            	//temp_pid equals to newborn childs pid
                mp_id_to_pid[m[killedChild]]= (long)temp_pid;
                m[temp_pid] = m[killedChild];
                
                //writing to watchdog_output.txt that we have killed the child
                FILE* fdie4;
                fdie4 = fopen(watchdog_output,"a");
                fprintf(fdie4, "Restarting P%d\nP%d is started and it has a pid of %ld\n",m[killedChild],m[killedChild],(long)temp_pid );
                fclose(fdie4);
                
                
                long lng_tmp= (long) killedChild;
                char  bufferForRestartedProcess[30];
                snprintf(bufferForRestartedProcess, 12, "P%d %ld" , (m[lng_tmp]),(long)temp_pid); // puts string into buffer
                write(fd, bufferForRestartedProcess, strlen(bufferForRestartedProcess)+1); //writes the input to the fifo
            }
            }
        }
        
    }
    FILE * fptr4;
    fptr4 = fopen(watchdog_output, "a");
    fprintf(fptr4,"Watchdog is terminating gracefully\n");
    fclose(fptr4);
    close(fd);
    sleep(3);
    wait(NULL);
    // watchdog process changes beahaviour of SIGTERM to default and send it to itself to terminate
    signal(SIGTERM, SIG_DFL);
    kill(getpid(), SIGTERM);
}
/// Signal handler for the signal SIGTERM
/// This signal handler changes the default behaviour of the SIGTERM to changing the value of **ext** to *true* so that watchdog process can exit gracefully
/// @param signum stands for the number of the signal that it will handle
void terminationSignal(int signum){
	sleep(10);
    if (watchdogPID==getpid()) {
        ext=true;
    }
        return;
    
    
    
}
