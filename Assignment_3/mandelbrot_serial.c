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



#define CPU_TIME (clock_gettime( CLOCK_REALTIME, &ts ), (double)ts.tv_sec + (double)ts.tv_nsec * 1e-9)

#ifdef OUTPUT
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif
#define CPU_ID_ENTRY_IN_PROCSTAT 39
#define HOSTNAME_MAX_LENGTH      200
void write_pgm_image( void *, int , int , int , const char *image_name);

int main()
{

int Ny = 4000;
int Nx = 8000;
double X_l = -2;
double Y_l = -1;
double X_r = 2;
double Y_r = 1;
double delta_x = (X_r-X_l)/Nx;
double delta_y = (Y_r-Y_l)/Ny;
short int *array_in = malloc (sizeof ( int) * (Nx*Ny));

int Imax = 65535;
/*  -----------------------------------------------------------------------------                                                           
   *   initialize                                                                                                                             
   *  -----------------------------------------------------------------------------                                                           
   */
for (int i=0;i<Ny+1;i++){
  for (int ii=0;ii<Nx+1;ii++){
    double c_im = Y_l + i*delta_y;
    double c_re = X_l+ii*delta_x;
    //printf(" questo è c: %f +i%f \n",c_re, c_im );  
    short int index=0;
    double Z_re=0,Z_im=0;
    double Z_reS, Z_imS;
    //printf("indice riga:%d indice colonna:%d", i,ii); 
    for(int n=0; n<Imax; ++n)
      { //printf(" questo è n: %d \n",n );
        Z_reS = Z_re*Z_re;
        Z_imS = Z_im*Z_im;
        //printf("%d %d %d",n, i,ii); 
        Z_im = 2*Z_re*Z_im + c_im;
        Z_re = Z_reS - Z_imS + c_re;
       // printf(" questo è z: %f %f\n",Z_re, Z_im );        
        if(Z_reS + Z_imS > 4)
        { index=n;
          break;
        }
      }
      array_in[i*Nx+ii]=index;
    //printf("%d ",index);    
  }
   //printf("\n");   
  }

  /*  -----------------------------------------------------------------------------                                                           
   *   calculate                                                                                                                              
   *  -----------------------------------------------------------------------------                                                           
   */

  /*  -----------------------------------------------------------------------------                                                           
   *   finalize                                                                                                                               
   *  -----------------------------------------------------------------------------                                                           
   */
                                                                                                                                        
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


