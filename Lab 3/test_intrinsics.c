/**************************************************************/
// gcc -msse4.1 -O1 -o test_intrinsics test_intrinsics.c -lrt -lm -mavx

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <assert.h>
#include <xmmintrin.h>
#include <smmintrin.h>
#include <immintrin.h>              //changed

#define FALSE 0
#define TRUE 1

#define GIG 1000000000
#define CPG 2.9           // Cycles per GHz -- Adjust to your computer

#define ARRAY_SIZE 100000000 // 100,000,000

#define SIZE 10000000
#define ITERS 3000
#define DELTA 32
#define BASE 0

#define OPTIONS 5
#define IDENT 1.0
#define OP *

typedef float data_t;


/* Number of bytes in a vector */
#define VBYTES 32

/* Number of elements in a vector */
#define VSIZE VBYTES/sizeof(data_t)


typedef struct {
  long int len;
  data_t *data;
} vec_rec, *vec_ptr;

typedef data_t vec_t __attribute__ ((vector_size(VBYTES)));
typedef union {
  vec_t v;
  data_t d[VSIZE];
} pack_t;

/**************************************************************/
int main(int argc, char *argv[])
{
  int OPTION;
  struct timespec diff(struct timespec start, struct timespec end);
  struct timespec time1, time2;
  struct timespec time_stamp[OPTIONS][ITERS+1];
  int clock_gettime(clockid_t clk_id, struct timespec *tp);

  int i,j,k;       /* Local variables. */
  long long int time_sec, time_ns;
  long int MAXSIZE = BASE+(ITERS+1)*DELTA;

  double* var;
  int     ok;
  data_t*  pArray1;
  data_t*  pArray2;
  data_t*  pResult;
  long int nSize;
  void  InitArray(data_t* pA, long int nSize);
  void  InitArray_rand(data_t* pA, long int nSize);
  void  ZeroArray(data_t* pA, long int nSize);
  void  ArrayTest1(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
  void  ArrayTest2(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
  void elem_wise_add(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);
  void elem_wise_mult(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);  
//  void elemwise_add_attempt(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);  
	void dot_prod(data_t* pA1, data_t* pA2, data_t* pR, long int nSize);  
	
  printf("\nHello World!  SSE Test, %ld", nSize);


  ok = posix_memalign((void**)&pArray1, 64, ARRAY_SIZE*sizeof(data_t));
  ok = posix_memalign((void**)&pArray2, 64, ARRAY_SIZE*sizeof(data_t));
  ok = posix_memalign((void**)&pResult, 64, ARRAY_SIZE*sizeof(data_t));

  // initialize pArray1, pArray2
  InitArray_rand(pArray1,MAXSIZE);
  InitArray_rand(pArray2,MAXSIZE);
  ZeroArray(pResult,MAXSIZE);

  OPTION = 0;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest1(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    ArrayTest2(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }
  
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    elem_wise_add(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

 OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    elem_wise_mult(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }
  
  OPTION++;
  for (i = 0; i < ITERS; i++) {
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time1);
    dot_prod(pArray1, pArray2, pResult, BASE+(i+1)*DELTA);
    clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &time2);
    time_stamp[OPTION][i] = diff(time1,time2);
  }

  printf("\n size, ArrTest1, ArrTest2,add,mult,dotprod");
  /* output times */
  for (i = 0; i < ITERS; i++) {
    printf("\n%d, ", BASE+(i+1)*DELTA);
    for (j = 0; j < OPTIONS; j++) {
      if (j != 0) printf(", ");
      printf("%ld", (long int)((double)(CPG)*(double)
     (GIG * time_stamp[j][i].tv_sec + time_stamp[j][i].tv_nsec)));
    }
  }

  printf("\n Goodbye World!\n");
  return 0;
}/* end main */

/*************************************************/
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

/* initialize array to index */
void InitArray(data_t* v, long int len)
{
  long int i;

  for (i = 0; i < len; i++) v[i] = (data_t)(i);
}

/* initialize vector with another */
void InitArray_rand(data_t* v, long int len)
{
  long int i;
  double fRand(double fMin, double fMax);

  for (i = 0; i < len; i++)
    v[i] = (data_t)(fRand((double)(0.0),(double)(10.0)));
}

/* initialize vector with 0s */
void ZeroArray(data_t* v, long int len)
{
  long int i;

  for (i = 0; i < len; i++)
    v[i] = (data_t)(0);
}

//******************************************************************
vec_ptr new_vec(long int len)
{
  long int i;

  /* Allocate and declare header structure */
  vec_ptr result = (vec_ptr) malloc(sizeof(vec_rec));
  if (!result) return NULL;  /* Couldn't allocate storage */
  result->len = len;

  /* Allocate and declare array */
  if (len > 0) {
    data_t *data = (data_t *) calloc(len, sizeof(data_t));
    if (!data) {
	  free((void *) result);
	  return NULL;  /* Couldn't allocate storage */
	}
	result->data = data;
  }
  else result->data = NULL;

  return result;
}

/* Retrieve vector element and store at dest.
   Return 0 (out of bounds) or 1 (successful)
*/
int get_vec_element(vec_ptr v, long int index, data_t *dest)
{
  if (index < 0 || index >= v->len) return 0;
  *dest = v->data[index];
  return 1;
}

/* Return length of vector */
long int get_vec_length(vec_ptr v)
{
  return v->len;
}

/* Set length of vector */
int set_vec_length(vec_ptr v, long int index)
{
  v->len = index;
  return 1;
}

/* initialize vector */
int init_vector(vec_ptr v, long int len)
{
  long int i;

  if (len > 0) {
    v->len = len;
    for (i = 0; i < len; i++) v->data[i] = (data_t)(i);
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
    for (i = 0; i < len; i++)
      v->data[i] = (data_t)(fRand((double)(0.0),(double)(10.0)));
    return 1;
  }
  else return 0;
}

data_t *get_vec_start(vec_ptr v)
{
  return v->data;
}

/************************************************************************************************************************/
/* Simple distance calc */
void ArrayTest1(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int i;

  data_t* pSource1 = pArray1;
  data_t* pSource2 = pArray2;
  data_t* pDest = pResult;
  float sqrtf(float x);

  for (i = 0; i < nSize; i++){
    *pDest = sqrtf((*pSource1) * (*pSource1) +
      (*pSource2) * (*pSource2)) + 0.5f;
    pSource1++;
    pSource2++;
    pDest++;
  }
}

/* Simple distance calc w/ SSE *//* AKA hypotenus*/
void ArrayTest2(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/4;

  __m128   m1, m2, m3, m4;
  __m128   m0_5 = _mm_set_ps1(0.5f);

  __m128*  pSrc1 = (__m128*) pArray1;
  __m128*  pSrc2 = (__m128*) pArray2;
  __m128*  pDest = (__m128*) pResult;

  for (i = 0; i < nLoop; i++){
    m1 = _mm_mul_ps(*pSrc1, *pSrc1);
    m2 = _mm_mul_ps(*pSrc2, *pSrc2);
    m3 = _mm_add_ps(m1,m2);
    m4 = _mm_sqrt_ps(m3);
    *pDest = _mm_add_ps(m4,m0_5);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

/* Simple distance calc w/ SSE */
void ArrayTest3(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/8;

  __m256   m1, m2, m3, m4;
  __m256   m0_5 = _mm256_set1_ps(0.5f);   //changed

  __m256*  pSrc1 = (__m256*) pArray1;
  __m256*  pSrc2 = (__m256*) pArray2;
  __m256*  pDest = (__m256*) pResult;

  for (i = 0; i < nLoop; i++){
    m1 = _mm256_mul_ps(*pSrc1, *pSrc1);
    m2 = _mm256_mul_ps(*pSrc2, *pSrc2);
    m3 = _mm256_add_ps(m1,m2);
    m4 = _mm256_sqrt_ps(m3);
    *pDest = _mm256_add_ps(m4,m0_5);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void elem_wise_add(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/4;

  __m128*  pSrc1 = (__m128*) pArray1;
  __m128*  pSrc2 = (__m128*) pArray2;
  __m128*  pDest = (__m128*) pResult;

  for (i = 0; i < nLoop; i++){

    *pDest = _mm_add_ps(*pSrc1, *pSrc2);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void elem_wise_mult(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/4;

  __m128*  pSrc1 = (__m128*) pArray1;
  __m128*  pSrc2 = (__m128*) pArray2;
  __m128*  pDest = (__m128*) pResult;

  for (i = 0; i < nLoop; i++){
    *pDest = _mm_mul_ps(*pSrc1, *pSrc2);

    pSrc1++;
    pSrc2++;
    pDest++;
  }
}

void dot_prod(data_t* pArray1,       // [in] 1st source array
    data_t* pArray2,       // [in] 2nd source array
    data_t* pResult,       // [out] result array
    long int nSize)            // [in] size of all arrays
{
  int  i, nLoop = nSize/4;

  __m128*  pSrc1 = (__m128*) pArray1;
  __m128*  pSrc2 = (__m128*) pArray2;
  __m128*  pDest = (__m128*) pResult;

  for (i = 0; i < nLoop; i++){
    *pDest = _mm_dp_ps(*pSrc1, *pSrc2, 0xFF);

    pSrc1++;
    pSrc2++;
    pDest++;
  }

}

/*this isn't vectorized??*//*
void dotprod(data_t *pArr1, data_t *pArr2, data_t *pRes, long int nSize)
{
	int i;
	int nLoop = nSize/16;

  __m256 m1,m2,m3;
	//__m256 m5 = _mm256_set1_ps(0.5f);
  
  __m256* pSrc1 = (__m256*) pArr1;
  __m256* pSrc2 = (__m256*) pArr2;
  __m256* pDest  = (__m256*) pRes;

	for(i=0;i<nLoop;i+=2)
	{
		m1 = _mm256_mul_ps(*pSrc1, *pSrc2);
    m2 = _mm256_mul_ps(*(pSrc1+1), *(pSrc2+1));
    m3 += _mm256_add_ps(m1,m2);
    
    pSrc1+=2;
   	pSrc2+=2;
		pDest+=2;
	}

}*/

//void dot_prod_vec(

