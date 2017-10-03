/*****************************************************************************/
// gcc -O1 -o test_SOR test_SOR.c -lrt -lm

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <pthread.h>

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define BASE  2
#define ITERS 1
#define DELTA 120

#define OMEGA 1.92       // TO BE DETERMINED
												 //change OMEGA with regards to array size to the one known to be most optimal??
#define OPTIONS 8

#define BLOCK_SIZE 8     // TO BE DETERMINED
#define BLOCK_SIZE2 16
#define BLOCK_SIZE3 24
#define BLOCK_SIZE4 50
#define BLOCK_SIZE5 100
#define BLOCK_SIZE6 150


#define MINVAL   0.0
#define MAXVAL  10.0

#define TOL 0.00001


typedef double data_t;

/* Create abstract data type for vector -- here a 2D array */
typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

/* Number of bytes in a vector (SSE sense) */
#define VBYTES 32//needed to change...

/* Number of elements in a vector (SSE sense) */
#define VSIZE VBYTES/sizeof(data_t)

typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef union {
  vec_t v;
  data_t d[VSIZE];
} pack_t;

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
  void SOR(vec_ptr v, int *iterations);
  void SOR_ji(vec_ptr v, int *iterations);
  void SOR_blocked(vec_ptr v, int *iterations);
  /*MORE BLOCKING METHODS DECLARED BELOW*/
	void SOR_blocked_16(vec_ptr v, int *iterations);
	void SOR_blocked_24(vec_ptr v, int *iterations);
	void SOR_blocked_50(vec_ptr v, int *iterations);
	void SOR_blocked_100(vec_ptr v, int *iterations);
	void SOR_blocked_150(vec_ptr v, int *iterations);
  /*END OF BLOCKING FUNCTIONS*/
  long int i, j, k;
  long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  printf("\n Hello World -- SOR serial variations \n");

  // declare and initialize the vector structure
  vec_ptr v0 = new_vec(MAXSIZE);
  iterations = (int *) malloc(sizeof(int));

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d OMEGA = %0.2f", i, BASE+(i+1)*DELTA, OMEGA);
    set_vec_length(v0, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    //print_vector(v0);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    SOR(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
    //print_vector(v0);
  }
/*reverse! reverse! ev'ry-body clap your hands*/
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_ji(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }
/*blocking*/
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }
/*MORE BLOCKING*/
 OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked_16(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked_24(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

	OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked_50(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

	OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked_100(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }

OPTION++;
  for (i = 0; i < ITERS; i++) {
    printf("\niter = %d length = %d", i, BASE+(i+1)*DELTA);
    init_vector_rand(v0, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    set_vec_length(v0,BASE+(i+1)*DELTA);
    SOR_blocked_150(v0,iterations);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
    convergence[OPTION][i] = *iterations;
  }








	printf("\n start of the printing of things! \n");
/*print out the timez*/

  for (i = 0; i < ITERS; i++) {
    printf("\nMAXSIZE = %d, \n", BASE+(i+1)*DELTA);
		printf("\nOMEGA = %0.2f, \n", OMEGA);
		printf("\n         	Time,   Iters,  Timer per Inner Loop\n");
    for (j = 0; j < OPTIONS; j++) {
      //if (j != 0) printf(", ");
			if(j==0) printf("NORMAL,      ");
			if(j==1) printf("REV,         ");
			if(j==2) printf("BLOCK = %d,   ", BLOCK_SIZE);
			if(j==3) printf("BLOCK = %d,   ", BLOCK_SIZE2);
			if(j==4) printf("BLOCK = %d,   ", BLOCK_SIZE3);
			if(j==5) printf("BLOCK = %d,   ", BLOCK_SIZE4);
			if(j==6) printf("BLOCK = %d,   ", BLOCK_SIZE5);
			if(j==7) printf("BLOCK = %d,   ", BLOCK_SIZE6);
			float times = (float)((double)(CPG)*(double)
		 (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec));
			float new_times = times/GIG*1000;
			float conv = (float) convergence[j][i];
			 printf("%f", new_times);
   		    printf(", %d", convergence[j][i]);
			float ratio = new_times/conv;
			printf(",    %f\n", ratio);
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

/* SOR */
void SOR(vec_ptr v, int *iterations)
{
  long int i, j;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;   // start w/ something big
  int iters = 0;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (i = 1; i < length-1; i++) 
      for (j = 1; j < length-1; j++) {
	change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					  data[(i+1)*length+j] +
					  data[i*length+j+1] +
					  data[i*length+j-1]);
	data[i*length+j] -= change * OMEGA;
	if (change < 0){
	  change = -change;
	}
	mean_change += change;
      }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %ld", iters);
      break;
    }
  }
   *iterations = iters;
  printf("\n iters = %d", iters);
}

/* SOR with reversed indices */
void SOR_ji(vec_ptr v, int *iterations)
{
  long int i, j;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;   // start w/ something big
  int iters = 0;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (j = 1; j < length-1; j++) 
      for (i = 1; i < length-1; i++) {
	change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					  data[(i+1)*length+j] +
					  data[i*length+j+1] +
					  data[i*length+j-1]);
	data[i*length+j] -= change * OMEGA;
	if (change < 0){
	  change = -change;
	}
	mean_change += change;
      }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}

/* SOR w/ blocking */
void SOR_blocked(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE)
	for (i = ii; i < ii+BLOCK_SIZE; i++)
	  for (j = jj; j < jj+BLOCK_SIZE; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}









/*MORE BLOCKING METHODS*/
void SOR_blocked_16(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE2) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE2)
	for (i = ii; i < ii+BLOCK_SIZE2; i++)
	  for (j = jj; j < jj+BLOCK_SIZE2; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}

void SOR_blocked_24(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE3) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE3)
	for (i = ii; i < ii+BLOCK_SIZE3; i++)
	  for (j = jj; j < jj+BLOCK_SIZE3; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}

void SOR_blocked_50(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE4) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE4)
	for (i = ii; i < ii+BLOCK_SIZE4; i++)
	  for (j = jj; j < jj+BLOCK_SIZE4; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}

void SOR_blocked_100(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE5) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE5)
	for (i = ii; i < ii+BLOCK_SIZE5; i++)
	  for (j = jj; j < jj+BLOCK_SIZE5; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}

void SOR_blocked_150(vec_ptr v, int *iterations)
{
  long int i, j, ii, jj;
  long int length = get_vec_length(v);
  data_t *data = get_vec_start(v);
  double change, mean_change = 100;
  int iters = 0;
  int k;

  while ((mean_change/(double)(length*length)) > (double)TOL) {
    iters++;
    mean_change = 0;
    for (ii = 1; ii < length-1; ii+=BLOCK_SIZE6) 
      for (jj = 1; jj < length-1; jj+=BLOCK_SIZE6)
	for (i = ii; i < ii+BLOCK_SIZE6; i++)
	  for (j = jj; j < jj+BLOCK_SIZE6; j++) {
	    change = data[i*length+j] - .25 * (data[(i-1)*length+j] +
					      data[(i+1)*length+j] +
					      data[i*length+j+1] +
					      data[i*length+j-1]);
	    data[i*length+j] -= change * OMEGA;
	    if (change < 0){
	      change = -change;
	    }
	    mean_change += change;
	  }
    if (abs(data[(length-2)*(length-2)]) > 10.0*(MAXVAL - MINVAL)) {
      printf("\n PROBABLY DIVERGENCE iter = %d", iters);
      break;
    }
  }
  *iterations = iters;
  printf("\n iters = %d", iters);
}
