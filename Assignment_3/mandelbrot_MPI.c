#include <sched.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <mpi.h>
#define USE MPI
#define SEED 35791246

#define Nx_default 8000
#define Ny_default 4000
#define X_l_default -2.75
#define Y_l_default 1
#define X_r_default 1.25
#define Y_r_default -1
#define Imax_default 255

int mandelbrot( int , double , double ,double ,double );
int start_image( int , int , int , const char *  );
int main ( int argc , char *argv[ ] )
{ int Nx    = Nx_default;
  int Ny    = Ny_default;
  double X_l = X_l_default;
  double Y_l = Y_l_default;
  double X_r = X_r_default;
  double Y_r = Y_r_default;
  int Imax  = Imax_default;

  if ( ( argc > 1 && argc < 8 ) || ( argc > 1 && argc > 8 ) )
    {
      printf("arguments should be passed as\n");
      printf("./mandelbrot.x n_x n_y x_L y_L x_R y_R I_max n_threads\n");
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

                                                                                                                                      
  double start_time, end_time; // times 
  int myid , numprocs , proc ;
  MPI_Status status;
  MPI_Request request;                                                                                                                          
  int master = 0;// master process 
  int tag = 123;
  MPI_Init(&argc,&argv);//start MPI
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);
  double delta_x = (X_r-X_l)/Nx;
  double delta_y = (Y_r-Y_l)/Ny;

  //read N from line
  start_time = MPI_Wtime();

  int header;
  if( myid == 0 ) header = start_image( Imax, Nx, Ny, "image.pgm" );
  MPI_Bcast( &header, 1, MPI_INT, 0, MPI_COMM_WORLD );
  MPI_Bcast( &X_l, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD );
  MPI_Bcast( &Y_l, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD );
  MPI_Bcast( &Imax, 1, MPI_INT, 0, MPI_COMM_WORLD );

  if (myid ==0) 
  { int i=0;                                                                   
    while (i<Ny+1)
    { index=i*Nx;
      MPI_Recv( &myid, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status );
      MPI_Send( &index, 2, MPI_INT, myid, 0, MPI_COMM_WORLD );
      i++;
    }
    end_time=MPI_Wtime();
    printf ( "\n # walltime on master processor : %10.8f \n", end_time - start_time ) ;
  }
  else 
  { MPI_Send( &myid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );
    MPI_Recv( &index, 2, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status ) ;
    unsigned char * proc_arr = (unsigned char *) malloc(  sizeof(unsigned char) * Nx );
    for(unsigned int ii, ii<Nx+1, ii++)
    { 
      int i=index/Ny
      double c_im = Y_l + i*delta_y;
      double c_re = X_l+ii*delta_x; 
      double Z_re=0,Z_im=0;
      proc_arr[ii]=mandelbrot(Imax, Z_re, Z_im,c_im,c_re)
    }
    int offset=header+index;
    MPI_File_write_at( file, offset, proc_arr, Nx, MPI_UNSIGNED_CHAR, &status );
    end_time=MPI_Wtime();
    free( proc_arr );
    printf ( "\n # walltime on processor %i : %10.8f \n",myid, end_time - start_time) ;
  }
  
  MPI_Finalize() ;
  return 0; // let MPI finish up /                                                                                                     
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


int start_image( int maxval, int xsize, int ysize, const char * image_name )
{

  FILE* image_file;

  image_file = fopen(image_name, "w");

  int color_depth = 1 + ( (maxval>>8)>0 );

  int len = fprintf( image_file, "P5\n%d %d\n%d\n", xsize, ysize, maxval );

  fclose( image_file );

  return len;

}