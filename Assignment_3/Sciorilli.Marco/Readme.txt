flags required in order to correctly execute the file, has to be given in this way:
./executable n_x n_y x_L y_L x_R y_R I_max
Anyway, in nothing at all is given, default options are already set.
If using the bash command "time" to collect data concerning the program (insted of /usr/bin/time),in order handle them more easyly it is recommended to use:"export TIMEFORMAT='%3S %3U'", 
if only user and system times are needed. 

|OpenMP|
The suggested compiler used is icc (flag required:
In order to use icc as a compiler, module intel/18.4 is required.
It can be easily load using the command: "module load intel/18.4"
In order to be sure to load a thread for each core, avoiding hyperthreading, it is recommended to use the command: "export KMP_AFFINITY=granularity=fine,scatter"

|MPI|
The suggested compiler used is mpicc (flag required:-std=c99 -lrt)
In order to use mpicc as a compiler, module openmpi is required.
It can be easily load using the command: "module load openmpi"
In order to run the program it is also required to call it in the command line like:
mpirun -np procs ./executable n_x n_y x_L y_L x_R y_R I_max
where procs are the number of processes we want to use in the execution.
