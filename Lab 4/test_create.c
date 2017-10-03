/**************************************************************/
// gcc -pthread -o test_create test_create.c

// Simple thread create and exit test

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define NUM_THREADS 5

/***************************************************************/
void *work(void *i)
{
 //sleep(3);
  printf(" Hello World! from child thread %lu\n",
	 pthread_self());

  pthread_exit(NULL);
}

/****************************************************************/
int main(int argc, char *argv[])
{
  int arg,i,j,k,m;   	              /* Local variables. */
  pthread_t * id = (pthread_t *) malloc(NUM_THREADS);
	
  for (i = 0; i < NUM_THREADS; ++i) {
    if (pthread_create(&(*(id+i)), NULL, work, NULL)) {
      printf("ERROR creating the thread\n");
      exit(19);
    }
  }
  sleep(3);
  printf("\n main() after creating the thread.  My id is %lu\n",
	 pthread_self());
  return(0);

}/* end main */
