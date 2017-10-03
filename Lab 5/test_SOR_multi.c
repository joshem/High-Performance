/*****************************************************************************/
// gcc -O1 -o test_SOR test_SOR_multi.c -lrt -lm


#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define BASE  2
#define ITERS 1
#define DELTA 258

#define OPTIONS 2
#define BLOCK_SIZE 8     // TO BE DETERMINED

#define MINVAL   0.0
#define MAXVAL  10.0

#define TOL 0.00001
#define OMEGA 1.97       // TO BE DETERMINED

typedef double data_t;

/* Create abstract data type for vector -- here a 2D array */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

/* Number of bytes in a vector (SSE sense) */
#define VBYTES 32

/* Number of elements in a vector (SSE sense) */
#define VSIZE VBYTES/sizeof(data_t)

typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef union {
  vec_t v;
  data_t d[VSIZE];
} pack_t;

int NUM_THREADS = 4;

struct thread_data{
	int thread_id;
	vec_ptr v;
	int* iterations;
}

/*****************************************************************************/
main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int convergence[OPTIONS][ITERS+1];
  vec_ptr new_vec(long int len);
  int set_vec_length(vec_ptr v, long int index);
  long int get_vec_length(vec_ptr v);
  int init_vector(vec_ptr v, long int len);
  int init_vector_rand(vec_ptr v, long int len);
  int print_vector(vec_ptr v);
  int *iterations;

	void pt_cb(vec_ptr v, int *iterations);

  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  printf("\n Hello World -- SOR serial variations \n");

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  iterations = (int *) malloc(sizeof(int));

  OPTION = 0;
	NUM_THREADS = 4;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
    set_vec_length(v0, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
		//Multithreaded SOR with blocking, 4 threads.
    pt_cb(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;

  }

  OPTION++;
	NUM_THREADS = 2;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
		//Multithreaded SOR with blocking, 2 threads
    pt_cb(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

  for (i = 0; i < ITERS; i++) {
    printf("\n%d, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      printf("%ld", (long int)((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
      printf(", %d", convergence[j][i]);
    }
  }
  printf("\n");
  
}/* end main */
/*********************************/

/* Create 2D vector of specified length per dimension */
vec_ptr new_vec(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len*len, sizeof(data_t));
    if (!data) {
	  free((void *) result);
	  printf("\n COULDN'T ALLOCATE STORAGE \n", result->len);
	  return NULL;  /* Couldn't allocate storage */
	}
	result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
  v->len = index;
  return 1;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
  return v->len;
}

/* initialize 2D vector */
int init_vector(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*len; i++) v->data[i] = (data_t)(i);
    return 1;
  }
  else return 0;
}

/* initialize vector with another */
int init_vector_rand(vec_ptr v, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len*len; i++)
      v->data[i] = (data_t)(fRand((double)(MINVAL),(double)(MAXVAL)));
    return 1;
  }
  else return 0;
}

/* print vector */
int print_vector(vec_ptr v)
{
  long int i, j, len;

  len = v->len;
  printf("\n length = %ld", len);
  for (i = 0; i < len; i++) {
    printf("\n");
    for (j = 0; j < len; j++)
      printf("%.4f ", (data_t)(v->data[i*len+j]));
  }
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/************************************/

struct timespec diff(struct timespec start, struct timespec end)
{
  struct timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

double fRand(double fMin, double fMax)
{
    double f = (double)random() / RAND_MAX;
    return fMin + f * (fMax - fMin);
}

/************************************/

/* SOR w/ blocking: Multithreaded work function*/
void *SOR_blocked(void *threadarg)//work function
{
  long int i, j, ii, jj;
	
	//extract the data from the void pointer passed in.
  struct thread_data *my_data;
	my_data = (struct thread_data *) threadarg;
	int taskid = my_data->thread_id;
	vec_ptr v = my_data->v;
	int* iterations = my_data->iterations;
	
	//the "block" that each thread works on is a rectangle, width of length and height of (length/NUM_THREADS).
	long int width = get_vec_length(v);
	long int height = ( width/NUM_THREADS );//figures out bound for each thread, each thread does different bound of matrix

  //long int length = get_vec_length(v);
	//start point is the beginning point of each block that a thread is supposed to work on.
  data_t *data = ( get_vec_start(v) + (width * taskid) );//determines where it starts
  double change, mean_change = 100;
  int iters = 0;
  int k;
//sor function below
  while ((mean_change/(double)(width*height)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < width-1; ii+=BLOCK_SIZE) 
      for (jj = 1; jj < height-1; jj+=BLOCK_SIZE)
				for (i = ii; i < ii+BLOCK_SIZE; i++)
					for (j = jj; j < jj+BLOCK_SIZE; j++) {
						change = data[i*width+j] - .25 * (data[(i-1)*width+j] +
											data[(i+1)*width+j] +
											data[i*width+j+1] +
											data[i*width+j-1]);
						data[i*width+j] -= change * OMEGA;
						if (change < 0){
							change = -change;
						}
						mean_change += change;
					}
    if (abs(data[(width-2)*(width-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}


/* pthread calling function for SOR_Blocking */
void pt_cb(vec_ptr v, int *iterations)
{
  long int i, j, k;
  pthread_t threads[NUM_THREADS];
  struct thread_data thread_data_array[NUM_THREADS];
  int rc;
  long t;

  for (t = 0; t < NUM_THREADS; t++) {
		//building void pointer arrays to pass in the variables
    thread_data_array[t].thread_id = t;
    thread_data_array[t].v = v;
    thread_data_array[t].iterations = iterations;
		//create the pthreads
    rc = pthread_create(&threads[t], NULL, SOR_blocked,
			(void*) &thread_data_array[t]);//makes pthreads
    if (rc) {
      printf("ERROR; return code from pthread_create() is %d\n", rc);
      exit(-1);
    }
  }

  for (t = 0; t < NUM_THREADS; t++) {
    if (pthread_join(threads[t],NULL)){ 
      printf("ERROR; code on return from join is %d\n", rc);
      exit(-1);
    }
  }
}
 
