#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#define USE MPI
#define SEED 35791246

int main ( int argc , char *argv[ ] )
{ unsigned long long int M, local_M ;                                                                                                                                     
  double r_start_time,r_end_time,comp_start_time,comp_end_time,start_time, end_time,comp_total_time,comm_start_time,comm_end_time,comm_total_time; // times 
  int myid , numprocs , proc ;
  MPI_Status status;
  MPI_Request request;                                                                                                                          
  int master = 0;// master process 
  int tag = 123;
  long long int N, myN;
  MPI_Init(&argc,&argv);//start MPI
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  if ( argc <=1) {
    fprintf (stderr , " Usage : mpi -np n %s number_of_iterations \n", argv[0] ) ;
    MPI_Finalize() ;
    exit(-1) ;
  }
  //read N from line
  r_start_time=MPI_Wtime()
  long long int K = atoll(argv[1])%numprocs;
  r_end_time=MPI_Wtime()
  start_time = MPI_Wtime();
  //divide N for the number of processors, giving uneven work if N is not divisible by P
  if (K==0){
    N = atoll(argv[1])/numprocs;
    myN =myid*N+1;
  }
  else{
    if(myid<K){
      N = atoll(argv[1])/numprocs+1;
      myN =myid*N+1;
    }
    else{
      N = atoll(argv[1])/numprocs;
      myN=(K)*(N+1)+(myid-K)*(N)+1;
    }
  }
  local_M=myN;
  unsigned long long int i;
  //star computation
  comp_start_time=MPI_Wtime()
  for (i=myN+1; i<myN+N ; i++) {
    local_M = local_M + i;
  }
  comp_end_time=MPI_Wtime()
  if (myid ==0) { //if I am the master process gather results from others                                                                     
    M = local_M ;
    printf ( "\n # partial sum on master processor : %llu \n", local_M) ;
    comm_start_time=MPI_Wtime()
    for (proc=1; proc<numprocs ; proc++) {
      MPI_Recv(&local_M,1,MPI_UNSIGNED_LONG_LONG,proc,tag,MPI_COMM_WORLD,&status ) ;
      M += local_M ;
    }
    comm_end_time=MPI_Wtime()
    end_time=MPI_Wtime();
    printf ( "\n # to sum = %llu , results %llu \n", N*numprocs, M ) ;
    printf ( "\n # walltime on master processor : %10.8f \n", end_time - start_time ) ;
    unsigned long long int realnum = (atoll(argv[1])+1)*atoll(argv[1])/2 ;
    printf ( "\n # real final number : %llu \n",realnum) ;

  else {
     // for all the slave processes send results to the master /                                                                        
    MPI_Ssend(&local_M , 1 ,MPI_UNSIGNED_LONG_LONG, master , tag ,MPI_COMM_WORLD) ;
    end_time=MPI_Wtime();
    printf ( "\n # walltime on processor %i : %10.8f \n",myid, end_time - start_time) ;
    printf ( "\n # partial sum on processor %d : %llu \n",myid, local_M) ;
  }
      // calculation of the total times
  r_total_time= r_end_time - r_start_time;
  comm_total_time= comm_end_time - comm_start_time;
  comp_total_time= comp_end_time - comp_start_time;

  //printing the results about time
  printf("[process %d] reading time: %10.10f \n", myid, r_total_time);
  printf("[process %d] communication time: %10.10f \n", myid, comm_total_time);
  printf("[process %d] computation time: %10.10f \n", myid, comp_total_time);
  }
  MPI_Finalize() ; // let MPI finish up /                                                                                                     

}
