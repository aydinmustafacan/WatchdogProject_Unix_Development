# WatchdogProject_Unix_Development
#### Watchdog Project by Mustafa Can Aydin


## Explanation


Main idea of the project is to implement a process named watchdog which will create children processes and will keep track of their status, well-being , etc. For instance, if some of the children were to die watchdog process should know about it, prints out neccessary information to its output file, and restarts the children processes for the duration of the program. After all the signals handled and there are no more instructions to execute watchdog process kills itself and terminates gracefully. If watchdog process were to die before the instructions have not been fully executed, children processes will not be killed by watchdog process and watchdog process will print to its output a message informing that it is terminating and sends itself the termination signal. Project includes three .cpp files: executor.cpp, watchdog.cpp, process.cpp. Executor program will send various signals to the processes specified by the watchdog process. In order for the executor and watchdog to communicate a FIFO is used which will be found at `/tmp/myfifo` . After executor program is run,  watchdog should be run with the specified arguments that will decide number of processes and two output paths. After child processes were created by watchdog process they should be sent to *process* program using `execv()` command. During the execution time of the watchdog, it will show corresponding outputs to signals received by its children processes. At the same time each child process will print neccessary information to process_output.txt file.  


## Compiling and Running 


In order to run program first one has to compile three *.cpp files into executable files individually.
~~~~~~~~~~~~~~~{.cpp}
    g++ process.cpp -std=c++14 -o process
    g++ watchdog.cpp -std=c++14 -o watchdog
    g++ executor.cpp -std=c++14 -o executor
~~~~~~~~~~~~~~~
After compilation is done, one has to run executor in the background as shown below 

~~~~~~~~~~~~~~~{.cpp}
    ./executor num_of_processes instruction_path/instructions.txt &
~~~~~~~~~~~~~~~
Then running watchdog executable will give the desired outcome of the program to the output files

~~~~~~~~~~~~~~~{.cpp}
    ./watchdog num_of_processes process_output.txt watchdog_output.txt
~~~~~~~~~~~~~~~
## Conclusion


To sum it up, watchdog project implements a main process called watchdog process which will crete some children and will know all the information about them and is responsible for restarting them and printing neccessary information about them for the duration of instructions. After the instructions completed it will terminate gracefully. 
