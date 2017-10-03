//i am a comment
//this is part 2
#include <cstdio>
#include <cstdlib>
#include <math.h>
#include <time.h>
//edit
// Assertion to check for errors
#define CUDA_SAFE_CALL(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, char *file, int line, bool abort=true)
{
	if (code != cudaSuccess) 
	{
		fprintf(stderr,"CUDA_SAFE_CALL: %s %s %d\n", cudaGetErrorString(code), file, line);
		if (abort) exit(code);
	}
}

#define NUM_THREADS_PER_BLOCK 	256
#define NUM_BLOCKS 				16
#define PRINT_TIME 				1
#define ARR_LEN				2000
#define SM_ARR_LEN          4000000
#define TOL						1e-6
#define ITERS 2000
#define IMUL(a, b) __mul24(a, b)

void initializeArray2D(float *arr, int len, int seed);


__global__ void kernal_SOR(int arrLen, float* x, float* result)
{
	const int row =  threadIdx.x;
	const int column =  threadIdx.y;
	float delta;
	int i,j,count;
	for(count=0;count<=ITERS;count++){//the iterations for loop
		// elemDim/numofThreads  2000/16 
		for(i=row*125;i<(row*125)+125;i++){
			for(j=column*125;j<(column*125)+125;j++){
				if(i>0 && j>0 && i<(ARR_LEN-1) && j<(ARR_LEN-1)){
					delta = x[i * ARR_LEN + j] - 0.25 * (x[(i-1) * ARR_LEN + j] + x[(i+1) * ARR_LEN + j] + x[(i) * ARR_LEN + j - 1] + x[(i) * ARR_LEN+ j + 1]);
					result[i*ARR_LEN + j] = delta;
				}
				else result[i*ARR_LEN+j] = x[i*ARR_LEN + j];
			}
		}
	}
	__syncthreads();

}

int main(int argc, char** argv) {
	int arrLen = 0;
	
	// GPU Timing variables
	cudaEvent_t GPU_start, GPU_stop, CPU_start, CPU_stop;
	float elapsed_gpu,elapsed_cpu;
	
	//Arrays on GP global memory
	float *d_x;
	float *d_y;
	float *d_result;

	//Arrays on the host mem
	float *h_x;
	float *h_y;
	float *h_result;
	float *h_result_gold;

	int i, j, errCount = 0, zeroCount = 0;
	

	
	if(argc>1) {
		arrLen = atoi(argv[1]);
	}
	else	{
		arrLen = SM_ARR_LEN;
	}

	CUDA_SAFE_CALL(cudaSetDevice(0));
	
	//Allocate GPU memory
	printf("arrlen = %d", arrLen);
	size_t allocSize = arrLen * sizeof(float);
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_x, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_y, allocSize));
	CUDA_SAFE_CALL(cudaMalloc((void **)&d_result, allocSize));

	//Allocate arrays on host memory
	h_x							= (float *) malloc(allocSize);
	//h_y							= (float *) malloc(allocSize);
	h_result					= (float *) malloc(allocSize);
	h_result_gold				= (float *) malloc(allocSize);

	//Init the host arrays
	printf("\nInitializing the arrays ...");
	//Arrays are initialized with a known seed for reproducability
	initializeArray2D(h_x, arrLen, 2453);
	//initializeArray2D(h_y, arrLen, 1467);
	printf("\t... done\n\n");

#if PRINT_TIME
	//CREATE THE CUDA EVENTS
	cudaEventCreate(&GPU_start);
	cudaEventCreate(&GPU_stop);
	//record event on the default stream
	cudaEventRecord(GPU_start, 0);
#endif

	dim3 dimblock(16,16);
	//Transfer the arrays to the GPU memory
	CUDA_SAFE_CALL(cudaMemcpy(d_x,h_x, allocSize, cudaMemcpyHostToDevice));
	//CUDA_SAFE_CALL(cudaMemcpy(d_y,h_y, allocSize, cudaMemcpyHostToDevice));

	//Launch the kernel
	kernal_SOR<<<1, dimblock>>>(arrLen,d_x,d_result);
	
	//check for errors during launch
	CUDA_SAFE_CALL(cudaPeekAtLastError());

	//Transfer the results back to the host
	CUDA_SAFE_CALL(cudaMemcpy(h_result, d_result, allocSize, cudaMemcpyDeviceToHost));

#if PRINT_TIME
	//stop and destroy timer
	cudaEventRecord(GPU_stop,0);
	cudaEventSynchronize(GPU_stop);
	cudaEventElapsedTime(&elapsed_gpu, GPU_start, GPU_stop);
	printf("\nGPU time: %f (msec)\n", elapsed_gpu);
	cudaEventDestroy(GPU_start);
	cudaEventDestroy(GPU_stop);
#endif

	//put sor stuff
	float diff;
	int cnt;
	//time the CPU
	cudaEventCreate(&CPU_start);
	cudaEventCreate(&CPU_stop);
	cudaEventRecord(CPU_start,0);
	
	//Compare results
	for(cnt=0;cnt<=ITERS;cnt++){
		for(i=0;i<ARR_LEN;i++){
			for(j=0;j<ARR_LEN;j++){
				if(i>0 && j>0 &&  i < (ARR_LEN-1) && j < (ARR_LEN-1)){//if not on edge
					diff =  h_x[i * ARR_LEN + j] - 0.25 * (h_x[(i-1) * ARR_LEN + j] + h_x[(i+1) * ARR_LEN + j] + h_x[(i) * ARR_LEN + j - 1] + h_x[(i) * ARR_LEN + j + 1]);
                	h_result_gold[i * ARR_LEN + j] = diff;
				}
				else  h_result_gold[i * ARR_LEN + j] = h_x[i * ARR_LEN + j];
			}
		}
	}
#if PRINT_TIME
	//stop and destroy timer
	cudaEventRecord(CPU_stop,0);
	cudaEventSynchronize(CPU_stop);
	cudaEventElapsedTime(&elapsed_cpu, CPU_start, CPU_stop);
	printf("\nCPU time: %f (msec)\n", elapsed_cpu);
	cudaEventDestroy(CPU_start);
	cudaEventDestroy(CPU_stop);
#endif
	//compare the results
	for(i=0;i<arrLen;i++)
	{
		if(abs(h_result_gold[i] - h_result[i]) > TOL)
			errCount++;
		if(h_result[i] == 0) 
			zeroCount++;
	}
	
	if(errCount>0){
		printf("\nERROR: Failure: %d results didn't match\n", errCount);
	}
	else if(zeroCount>0) printf("\n ERROR: Failure: %d results are zero\n", zeroCount);
	else printf("\nTest Success: All Results Good\n");
	return 0;
}

void initializeArray2D(float *arr, int len, int seed){
	int i,j;
	float randNum;
	srand(seed);
	for(i=0; i<len;i++){
		randNum = (float) rand();
		arr[i] = randNum;
	}

}
