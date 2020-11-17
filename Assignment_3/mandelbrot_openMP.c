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
#include <math.h>
#include <omp.h>

#define Nx_default 8000
#define Ny_default 4000
#define X_l_default -2.
#define Y_l_default 1
#define X_r_default 1.5
#define Y_r_default -1
#define Imax_default 255


#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)

#ifdef OUTPUT
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#define CPU_ID_ENTRY_IN_PROCSTAT 39
#define HOSTNAME_MAX_LENGTH      200
int read_proc__self_stat ( int, int * );
int get_cpu_id           ( void      );
void write_pgm_image( void *, int , int , int , const char *image_name);
int mandelbrot( int , double , double ,double ,double );

int main ( int argc , char *argv[ ] )
{ 

/*  -----------------------------------------------------------------------------                                                           
 *   initialize                                                                                                                             
 *  -----------------------------------------------------------------------------                                                           
 */

  int Nx    = Nx_default;
  int Ny    = Ny_default;
  double X_l = X_l_default;
  double Y_l = Y_l_default;
  double X_r = X_r_default;
  double Y_r = Y_r_default;
  int Imax  = Imax_default;

  if ( ( argc > 1 && argc < 8 ) || ( argc > 1 && argc > 8 ) )
    {
      printf("arguments should be passed as\n");
      printf("./mandelbrot.x n_x n_y X_l Y_l X_r Y_r I_max \n");
    }
  else if ( argc > 1 )
    {
      Nx        = atoi( *(argv + 1) );
      Ny        = atoi( *(argv + 2) );
      X_l       = atof( *(argv + 3) );
      Y_l       = atof( *(argv + 4) );
      X_r       = atof( *(argv + 5) );
      Y_r       = atof( *(argv + 6) );
      Imax      = atoi( *(argv + 7) );

    }

double tstart  = CPU_TIME; 
double delta_x = (X_r-X_l)/Nx;
double delta_y = (Y_r-Y_l)/Ny;
short int *array_in = malloc (sizeof ( int) * (Nx*Ny));
int Imax = 65535;


/*  -----------------------------------------------------------------------------                                                           
 *   calculate                                                                                                                              
 *  -----------------------------------------------------------------------------                                                           
 */

#pragma omp parallel for schedule(dynamic, 8) private(c_im,c_re, Z_re,Z_im) collapse(2)
for (int i=0;i<Ny+1;i++)
{
  for (int ii=0;ii<Nx+1;ii++)
  {
    double c_im = Y_l + i*delta_y;
    double c_re = X_l+ii*delta_x; 
    short int index=0;
    double Z_re=0,Z_im=0;
    double Z_reS, Z_imS;
    array_in[i*Nx+ii]=mandelbrot(Imax, Z_re, Z_im,c_im,c_re);  
  }
}
double tend = CPU_TIME;

/*  -----------------------------------------------------------------------------                                                           
 *   finalize                                                                                                                               
 *  -----------------------------------------------------------------------------                                                           
 */
printf("Process took %g of wall-clock time\n\n", tend - tstart);                                                                                                                                            
write_pgm_image( array_in, Imax, Nx, Ny, "image.pgm" );
free( array_in );
return 0;
}




/*  -----------------------------------------------------------------------------                                                             
 *   functions                                                                                                                               
 *  -----------------------------------------------------------------------------                                                             
 */



void write_pgm_image( void *image, int maxval, int xsize, int ysize, const char *image_name)
{
  FILE* image_file; 
  image_file = fopen(image_name, "w"); 
  
  // Writing header
  // The header's format is as follows, all in ASCII.
  // "whitespace" is either a blank or a TAB or a CF or a LF
  // - The Magic Number (see below the magic numbers)
  // - the image's width
  // - the height
  // - a white space
  // - the image's height
  // - a whitespace
  // - the maximum color value, which must be between 0 and 65535
  //
  // if he maximum color value is in the range [0-255], then
  // a pixel will be expressed by a single byte; if the maximum is
  // larger than 255, then 2 bytes will be needed for each pixel
  //

  int color_depth = 1+((maxval>>8)>0);       // 1 if maxval < 256, 2 otherwise

  fprintf(image_file, "P5\n%d %d\n%d\n", xsize, ysize, maxval);
  
  // Writing file
  fwrite( image, color_depth, xsize*ysize, image_file);  

  fclose(image_file); 
  return ;

  /* ---------------------------------------------------------------

     TYPE    MAGIC NUM     EXTENSION   COLOR RANGE
           ASCII  BINARY

     PBM   P1     P4       .pbm        [0-1]
     PGM   P2     P5       .pgm        [0-255]
     PPM   P3     P6       .ppm        [0-2^16[
  
  ------------------------------------------------------------------ */
}


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


int mandelbrot( int Imax, double Z_re, double Z_im,double c_im,double c_re)
{
  for(int n=0; n<Imax; ++n)
  { 
    unsigned char color_val=0;
    double Z_reS, Z_imS;  
    Z_reS = Z_re*Z_re;
    Z_imS = Z_im*Z_im;
    Z_im = 2*Z_re*Z_im + c_im;
    Z_re = Z_reS - Z_imS + c_re;    
    if(Z_reS + Z_imS > 4)
    { 
      color_val=n;
      break;
    }
  }
  return color_val;
}
