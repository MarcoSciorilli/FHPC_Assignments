The program is equiped with a makefile which provides all the necessary flags for a correct compiling using openmp on Ulysse.
In order to use icc as a compiler, module intel/18.4 is required.
It can be easyly load using the command: "module load intel/18.4"
In order to be sure to load a thread for each core, avoiding hyperthreading, it is recommended to use the command: "export KMP_AFFINITY=granularity=fine,scatter"
Finally, if using the bash command "time" to collect data concerning the program (insted of /usr/bin/time),in order handle them more easyly it is recommended to use:"export TIMEFORMAT='%3S %3U'", 
if only user and system times are needed. 

