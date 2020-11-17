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
#define X_l_default -2.5
#define Y_l_default 1
#define X_r_default 1.5
#define Y_r_default -1
#define Imax_default 255
                                                                                      
int mandelbrot( int , double , double ,double ,double );                              // Initialize functions
int header_image( int , int , int , const char *  );                                 
int main ( int argc , char *argv[ ] )                                                     
{ 
/*  -----------------------------------------------------------------------------                                                           
 *   initialize                                                                                                                             
 *  -----------------------------------------------------------------------------                                                           
 */
  int Nx    = Nx_default;                                                           // Set the input variables to their default value
  int Ny    = Ny_default; 
  double X_l = X_l_default;
  double Y_l = Y_l_default;
  double X_r = X_r_default;
  double Y_r = Y_r_default;
  int Imax  = Imax_default;

  if ( ( argc > 1 && argc < 8 ) || ( argc > 1 && argc > 8 ) )                       //If the number of arguments give is wrong, notify to 
    {                                                                               //the user.
      printf("arguments should be passed as\n");
      printf("./mandelbrot.x n_x n_y x_L y_L x_R y_R I_max n_threads\n");
    }
  else if ( argc > 1 )                                                              //get inputs from command line
    {
      Nx        = atoi( *(argv + 1) );
      Ny        = atoi( *(argv + 2) );
      X_l       = atof( *(argv + 3) );
      Y_l       = atof( *(argv + 4) );
      X_r       = atof( *(argv + 5) );
      Y_r       = atof( *(argv + 6) );
      Imax      = atoi( *(argv + 7) );

    }

                                                                                                                                      
  double start_time, end_time;                                                      //initialize times
  int myid , numprocs , proc ;
  MPI_File file;                                                                    //initialize the MPI file on which all processes are going to write
  MPI_Status status;                                                                // |Usual MPI inizializations
  MPI_Request request;                                                              // |                                                          
  int master = 0;                                                                   //set master process 
  MPI_Init(&argc,&argv);                                                            //start MPI
  MPI_Comm_size(MPI_COMM_WORLD,&numprocs);                                          //Get number of processes
  MPI_Comm_rank(MPI_COMM_WORLD,&myid);                                              //Get the rank or every process
  MPI_File_open(MPI_COMM_WORLD, "image.pgm", MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &file); //Open the file mantioned above
  double delta_x = (X_r-X_l)/Nx;                                                    // |calculate the distance between final pixels
  double delta_y = -(Y_r-Y_l)/Ny;                                                   // |
  //read N from line
  start_time = MPI_Wtime();                                                         //start tacking times

  int header;
  if( myid == 0 ) header = header_image( Imax, Nx, Ny, "image.pgm" );               //get the header from the image
  MPI_Bcast( &header, 1, MPI_INT, 0, MPI_COMM_WORLD );                              //communicate header to all processes
  if (myid ==0) 
  { 
    int name;
    int i=0;                                                            
    while (i<Ny)                                                                    //for every line in the image
  	{ 
  	  int index=i*Nx;                                                                
  	  MPI_Recv( &name, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status );   //when recive an availability message from a process 
  	  MPI_Send( &index, 1, MPI_INT, name, 1, MPI_COMM_WORLD );                     //send it the index on the array telling the index of beginning of the line
  	  i++;                                                                         //it has to work on
  	  // printf("%d \n",i);
  	}
    for ( int i = 1; i < numprocs; ++i )                                          // |when all lines is processed
    {                                                                             // |send to every process
      MPI_Recv( &name, 1, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &status );  // |a stop message
      MPI_Send( &i, 0, MPI_INT, name, 2, MPI_COMM_WORLD );                        // |

    }
  }
  else 
  { unsigned char * proc_arr = (unsigned char *) malloc( sizeof(unsigned char)* Nx );
    while (1)
    {
      int index;
      MPI_Send( &myid, 1, MPI_INT, 0, 0, MPI_COMM_WORLD );                       //every slave send a message telling its availability
      MPI_Recv( &index, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status ) ;  //recive the mantioned aboxe index
      //printf("my identity %d \n", myid);
      if( status.MPI_TAG == 2 ) break;                                           // break if tag=2, ending the work in the process
      for(unsigned int ii=0; ii<Nx; ii++)
      { 
      int i=index/Nx;
      double c_im = Y_l - i*delta_y;                                            // |find the component of the 
      double c_re = X_l+ii*delta_x;                                             // |complex number corresponing to the 
      double Z_re=0,Z_im=0;                                                     // |point on the image
      proc_arr[ii]=mandelbrot(Imax, Z_re, Z_im,c_im,c_re);                      //use the function to find if it is in the mandelbrot set
      }
      int offset=header+index;                                                  //calulate the writing offset required to write on the file
      MPI_File_write_at( file, offset, proc_arr, Nx, MPI_UNSIGNED_CHAR, &status );  //write the so obtained array on the file
    }
    free( proc_arr );                                                           //free the temporary array at the end of the process
  }
  end_time=MPI_Wtime();                                                         //take the end of the time
  printf ( "\n # walltime : %10.8f \n", end_time - start_time) ;
  MPI_File_close( &file );                                                      //close the file
  MPI_Finalize() ;
  return 0; // let MPI finish up /                                              //finalize MPI                                                     
}

int mandelbrot( int Imax, double Z_re, double Z_im,double c_im,double c_re)     // |calulate by the definition if the given complex number is 
{                                                                               // |in the mandelbrot set, diving as a return
  unsigned char colorind=0;                                                     // |the number of iteration require to know it
  for(unsigned char n=0; n<Imax; n++)
    { 
      colorind=0;
      double Z_reS, Z_imS;  
      Z_reS = Z_re*Z_re;
      Z_imS = Z_im*Z_im;
      Z_im = 2*Z_re*Z_im + c_im;
      Z_re = Z_reS - Z_imS + c_re;    
      if(Z_reS + Z_imS > 4)
	{ 
	  colorind = n;
	  break;
	}
    }
  return colorind;
}


int header_image( int Imax, int xsize, int ysize, const char * image_name )
{

  FILE* image_file;

  image_file = fopen(image_name, "w");

  int color_depth = 1 + ( (Imax>>8)>0 );

  int len = fprintf( image_file, "P5\n%d %d\n%d\n", xsize, ysize, Imax );

  fclose( image_file );

  return len;

}
