The programs *mpi_summer.cc* and *collective_summer.cc* presented have been written in C and compiled and run on Ulysses cluster.

After loading openmpi module through ‘’‘module load openmpi’‘’, they’ve been compiled with

```mpicc mpi_summer.c -o mpi_summer```

and run with

```mpirun -np P mpi_summer```

where P is the desired number of cores.

