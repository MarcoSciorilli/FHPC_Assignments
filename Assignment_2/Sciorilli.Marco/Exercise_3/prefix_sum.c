#if defined(__STDC__)
#  if (__STDC_VERSION__ >= 199901L)
#     define _XOPEN_SOURCE 700
#  endif
#endif
#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sched.h>
#include <omp.h>
#include <math.h>

#define N_default 1000000000

#if defined(_OPENMP)
#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)

#else

#define CPU_TIME (clock_gettime( CLOCK_PROCESS_CPUTIME_ID, &ts ), (double)ts.tv_sec + \
                  (double)ts.tv_nsec * 1e-9)
#endif

#ifdef OUTPUT
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#define CPU_ID_ENTRY_IN_PROCSTAT 39
#define HOSTNAME_MAX_LENGTH      200

int read_proc__self_stat ( int, int * );
int get_cpu_id           ( void      );
void parallel_summer(double * restrict, double * restrict ,int,int);


int main( int argc, char **argv )
{
  int     N        = N_default;
  int     nthreads = 1;
  struct  timespec ts;
  double *array_in = malloc (sizeof (int) * N);
  double *array_out = malloc (sizeof (int) * N);


/*  -----------------------------------------------------------------------------                                                           
   *   initialize                                                                                                                             
   *  -----------------------------------------------------------------------------                                                           
   */
  // check whether some arg has been passed on                                                                                                
  if ( argc > 1 )
    N = atoi( *(argv+1) );

  if ( (array_in = (double*)malloc( N * sizeof(double) )) == NULL ) {
    printf("I'm sorry, on some thread there is not"
           "enough memory to host %lu bytes\n",
           N * sizeof(double) ); return 1; }
  // just give notice of what will happen and get the number of threads used
#if defined(_OPENMP)
#pragma omp parallel
  {
#pragma omp master
    {
      nthreads = omp_get_num_threads();
      PRINTF("omp summation with %d threads\n", nthreads );                                                                                 
    }
    int me = omp_get_thread_num();

    #pragma omp critical                                                                                                                    
       PRINTF("thread %2d is running on core %2d\n", me, get_cpu_id() );                                                                    
}

#endif															//initialize the array
  for ( int ii = 0; ii < N; ii++ )								
    {
      array_in[ii] =(double) 1;
    }

  /*  -----------------------------------------------------------------------------                                                           
   *   calculate                                                                                                                              
   *  -----------------------------------------------------------------------------                                                           
   */
  double tstart  = CPU_TIME;								//start taking time

#if !defined(_OPENMP)
  array_out[0]=array_in[0];
  parallel_summer(array_out,array_in,0,N);					//do the prefix sum for the serial case
#else
 int *array_med = malloc (sizeof (int) * nthreads);
#pragma omp parallel 										//open the parallel zone
  {
    int frac=N/nthreads; 									//split the array among the threads, calculating for each  
    int resto=N-frac*nthreads;								// one of then where to begin and where to stop
    int start,stop;
    int me = omp_get_thread_num();
    if(me<resto)
      {
        frac=frac+1;
        start=me*frac;
      }
    else
      {														//it also take in consideration the cases in which
        start=resto*(frac+1)+(me-resto)*frac;				// N is not divisible by the number of threads
      }
    stop=start+frac;
    array_out[start]=array_in[start];
    parallel_summer(array_out,array_in, start,stop);		//each array perform the prefix sum on its share of the array
    array_med[me]=stop;										// every threads save its ending element in an array
  }
  long double temp;
  int start2;
  int stop2;
  for(int i=0;i<nthreads-1;i++)								//skipping the first bit of the array, which is already correct,
  {
    start2=array_med[i];									//every part of the array has to be corrected taking in consideration
    stop2=array_med[i+1];									//the missing part evalueted by the thread in the previous section.
    temp=array_out[start2-1];								//So the ends of each section is initialize for each threads, and 
#pragma omp parallel for firstprivate(temp)					//they alltogheter correct each part, once at a time, in order.
    for (int ii=start2;ii<stop2;ii++)
    array_out[ii]=array_out[ii]+temp;
    }
#endif
  double tend = CPU_TIME;									//finish taking time


  /*  -----------------------------------------------------------------------------                                                           
   *   finalize                                                                                                                               
   *  -----------------------------------------------------------------------------                                                           
   */
                                                                                                                                        
  printf("Sum is %g, process took %g of wall-clock time\n\n"     //print out the results                                                                                                                                                                                  
     S, tend - tstart);                                                                                   
  
free( array_in );
free( array_out );
#if defined(_OPENMP)
 free( array_med);
#endif
return 0;
}




/*  -----------------------------------------------------------------------------                                                             
 *   functions                                                                                                                               
 *  -----------------------------------------------------------------------------                                                             
 */





int get_cpu_id( void )
{

#if defined(_GNU_SOURCE)                              // GNU SOURCE ------------                                                              

  return  sched_getcpu( );

#else

#ifdef SYS_getcpu                                     //     direct sys call ---                                                              

  int cpuid;
  if ( syscall( SYS_getcpu, &cpuid, NULL, NULL ) == -1 )
    return -1;
  else
    return cpuid;

#else

  unsigned val;
  if ( read_proc__self_stat( CPU_ID_ENTRY_IN_PROCSTAT, &val ) == -1 )
    return -1;
  return (int)val;

#endif                                                // -----------------------                                                              
#endif

}


int read_proc__self_stat( int field, int *ret_val )
/*                                                                                                                                            
  Other interesting fields:                                                                                                                   
                                                                                                                                              
  pid      : 0                                                                                                                                
  father   : 1                                                                                                                                
  utime    : 13                                                                                                                               
  cutime   : 14                                                                                                                               
  nthreads : 18                                                                                                                               
  rss      : 22                                                                                                                               
  cpuid    : 39                                                                                                                               
                                                                                                                                              
  read man /proc page for fully detailed infos                                                                                                
 */
{

  // not used, just mnemonic                                                                                                                  
  // char *table[ 52 ] = { [0]="pid", [1]="father", [13]="utime", [14]="cutime", [18]="nthreads", [22]="rss", [38]="cpuid"};                  

  *ret_val = 0;

  FILE *file = fopen( "/proc/self/stat", "r" );
  if (file == NULL )
    return -1;

  char   *line = NULL;
  int     ret;
  size_t  len;
  ret = getline( &line, &len, file );
  fclose(file);

  if( ret == -1 )
    return -1;

  char *savetoken = line;
  char *token = strtok_r( line, " ", &savetoken);
  --field;
  do { token = strtok_r( NULL, " ", &savetoken); field--; } while( field );

  *ret_val = atoi(token);

  free(line);

  return 0;
}


void parallel_summer(double * restrict a, double * restrict b, int c, int d)
{
  for(int ii=c+1; ii<d; ii++)
    {
      a[ii]=b[ii]+a[ii-1];

    }

}
