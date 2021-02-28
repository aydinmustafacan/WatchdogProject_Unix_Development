#include <cstring>
#include <sys/types.h>
#include <sys/wait.h>
#include <iostream>
#include <string>
#include <unistd.h>
#include <csignal>
#include <sstream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <map>
// Created by Mustafa Can Aydin on 14.12.2020.

using namespace std;
/// Helper map that helps child process to keep track of the pid and id of itselt
/// This map named mp has key consisting of id and has a value consisting of corresponding pid
map<long, int> mp;
/// address of child process' output
char* outputPath;
/// Control variable that helps child process to terminate if it is not adready terminated by SIGTERM
bool exited = false;

/// Signal handler for all the signals except the signal SIGTERM
/// This signal handler changes the default behaviour of the signals to printing neccesary messages to the output file
/// @param signum stands for the number of the signal that it will handle
void signalhandler(int signum){
    long process_id_for_this_signal =(long) getpid();
    FILE* fsignal;
    fsignal = fopen(outputPath,"a");
    fprintf(fsignal, "P%d recieved signal %d\n",mp[process_id_for_this_signal], signum);
    fclose(fsignal);
    return;
}

/// Signal handler for the signal SIGTERM
/// This signal handler changes the default behaviour of the SIGTERM to changing the value of **exited** to *true* so that child process can exit gracefully
/// Also it prints out to process_out.txt file *P%d recieved signal %d, terminating gracefully*
/// @param signum stands for the number of the signal that it will handle
void terminationSignal(int signum){
    int ii =mp[(long)getpid()];
    signal(SIGTERM, SIG_DFL);   
    kill(getpid(), SIGTERM);
    FILE* fterm;
    fterm = fopen(outputPath,"a");
    fprintf(fterm, "P%d recieved signal %d, terminating gracefully\n",ii, signum);
    fclose(fterm);
    exited= true;
    return;
}



/// Main *process* code implementing behaviours of each child process
///@param argc Number of arguments that the program takes
///@param argv[] argument vector which will be used to access arguments for process program
///@note This program implements control mechanisms for the child processes that will enable them to respond to the signals they receive with printing out corresponding messages to the process_out.txt file.
///@author Mustafa Can Aydin
int main(int argc, char *argv[]){
    // first we need to check whether or not we have valid number of command line arguments
    // check for valid number of command-line arguments if it is not valid print to the console
    if(argc != 3) {
        printf("number of arguments is wrong please correct it and try again \n");
        
    }
    // outputPath is set to first argument
    outputPath=argv[1];
    // no_of_i is set to second argument and after being converted from string to integer
    int no_of_i = stoi(argv[2]);
    // map value is set from pid to id
    mp[getpid()]= no_of_i;
            
    // File pointer to open process_output.txt file and append it the *P%d is waiting for a signal*
    FILE *fptrprocess;
    fptrprocess= fopen(outputPath, "a");
    fprintf(fptrprocess,"P%d is waiting for a signal\n",  no_of_i);
    fclose(fptrprocess);
    
    // Here I implement signal handlers using signal() command
    signal(SIGINT, signalhandler);
    signal(SIGHUP,signalhandler);
    signal(SIGILL, signalhandler);
    signal(SIGTRAP, signalhandler);
    signal(SIGBUS, signalhandler);
    signal(SIGFPE, signalhandler);
    signal(SIGSEGV, signalhandler);
    signal(SIGXCPU,signalhandler);
    signal(SIGTERM, terminationSignal);
    
    // Infinitite for-loop which will exit if the boolean variable *exited* equals to true
    for(;;){
        if(exited){
            break;
        }
        pause();
    }
    
    signal(SIGTERM, SIG_DFL);
    kill(getpid(), SIGTERM);
}



