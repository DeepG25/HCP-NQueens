#include<bits/stdc++.h>
#include<math.h>
#include<omp.h>
#include<time.h>
#include<string.h>
#include<stdlib.h>

using namespace std;

//  Using the MONOTONIC clock
#define CLK CLOCK_MONOTONIC

/* Function to compute the difference between two points in time */
struct timespec diff(struct timespec start, struct timespec end);

/*
   Function to computes the difference between two time instances
   Taken from - http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
   Further reading:
http://stackoverflow.com/questions/6749621/how-to-create-a-high-resolution-timer-in-linux-to-measure-program-performance
http://stackoverflow.com/questions/3523442/difference-between-clock-realtime-and-clock-monotonic
 */
struct timespec diff(struct timespec start, struct timespec end){
	struct timespec temp;
	if((end.tv_nsec-start.tv_nsec)<0){
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	}
	else{
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

/*************************** functional algorithm starts here *******************************************/

long long int totalSolutionCount = 0;		//stores total number of solutions
long long int totalNoofOdd = 0;				//stores total number of unsymmetric solutions
long long int threadCount[100];				//used to stores the solution count by each thread
const int LEVEL = 1;

void solveLinear(int board[],int boardSize,int row,int col,int col1)
{
	for(int i=0;i<row;i++)
	{
		//vertical attacks on the queen
		if(board[i] == col)
		{
			return;
		}

		//diagonal attacks on the queen
		if(abs(board[i] - col) == (row - i))
		{
			return;
		}
	}

	board[row] = col;
	if(row == boardSize-1 && col1 != (boardSize/2))
	{
		#pragma omp critical
		threadCount[omp_get_thread_num()]++;		//adds 1 solution to the threadCount[k] for kth processor
	}
	else if(row == boardSize-1 && col1 == boardSize/2)
	{
		#pragma omp critical
		totalNoofOdd++;
	}
	else
	{
		for(int i=0;i<boardSize;i++)
		{
			solveLinear(board,boardSize,row+1,i,col1);		//recursive call for the next row
		}
	}
}

void solveLevel(int board[],int boardSize,int row,int col,int col1)
{
	for(int i=0;i<row;i++)
	{
		//vertical attacks on the queen
		if(board[i] == col)
		{
			return;
		}

		//diagonal attacks on the queen
		if(abs(board[i] - col) == (row - i))
		{
			return;
		}
	}

	board[row] = col;
	if(row == boardSize-1 && col1 != (boardSize/2))
	{
		#pragma omp critical
		threadCount[omp_get_thread_num()]++;		//adds 1 solution to the threadCount[k] for kth processor
	}
	else if(row == boardSize-1 && col1 == boardSize/2)
	{
		#pragma omp critical
		totalNoofOdd++;
	}
	else
	{
		if(row <= LEVEL)
		{
			for(int i=0;i<boardSize;i++)
			{
				#pragma omp parallel
				#pragma omp single
				{
					#pragma omp task
					solveLevel(board,boardSize,row+1,i,col1);		//recursive call for next row
				}
			}
		}
		else
		{
			for(int i=0;i<boardSize;i++)
			{
				solveLevel(board,boardSize,row+1,i,col1);		//recursive call for next row
			}
		}
	}
}

void solveNQueens(int n,int p)
{
	omp_set_num_threads(p);			//using p processors will executing parallely
	if(n%2 == 0)					//when n is even has only symmetric solutions
	{
		#pragma omp parallel
		#pragma omp single
		{
			for(int i=0;i<n/2;i++)
			{
				#pragma omp task
				{
					int *board = new int[n];
					solveLinear(board,n,0,i,i);
				}
			}
		}
	}
	else						//when n is odd has only unsymmetric solutions
	{
		#pragma omp parallel
		#pragma omp single
		{
			for(int i=0;i<=n/2;i++)
			{
				#pragma omp task
				{
					int *board = new int[n];
					solveLinear(board,n,0,i,i);
				}
			}
		}
	}
}

/*************************** functional algorithm finished *********************************************/


int main(int argc, char* argv[])
{
	struct timespec start_e2e, end_e2e, start_alg, end_alg, e2e, alg;
	/* Should start before anything else */
	clock_gettime(CLK, &start_e2e);

	/* Check if enough command-line arguments are taken in. */
	if(argc < 3){
		printf( "Usage: %s n p \n", argv[0] );
		return -1;
	}

	int n=atoi(argv[1]);	/* size of input array */
	int p=atoi(argv[2]);	/* number of processors*/
	char *problem_name = "nqueens";
	char *approach_name = "model3";
//	char buffer[10];
//	FILE* inputFile;
	FILE* outputFile;
	//	inputFile = fopen(argv[3],"r");

	char outputFileName[50];
	sprintf(outputFileName,"output/%s_%s_%s_%s_output.txt",problem_name,approach_name,argv[1],argv[2]);


	for(int i=0;i<100;i++)
	{
		threadCount[i] = 0;
	}

	clock_gettime(CLK, &start_alg);	/* Start the algo timer */

	/*----------------------Core algorithm starts here----------------------------------------------*/

    solveNQueens(n,p);

	/*----------------------Core algorithm finished--------------------------------------------------*/

	clock_gettime(CLK, &end_alg);	/* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	   timers. Further, the whole algorithm should be present. */

    int max_ele = -1;
    for(int i=0;i<100;i++)
    {
    	if(threadCount[i] != 0)
    	{
    		max_ele = max(max_ele,i);
    	}
    	totalSolutionCount += threadCount[i];		//adds solutions obtained by all threads
    }

    if(n%2 == 0)
    {
    	totalSolutionCount *= 2;			//multiplying by 2 because of symmetry
    }
    else
    {
    	totalSolutionCount = totalSolutionCount*2 + totalNoofOdd;		//multiplying by 2 for symmetric solutions + adding the unsymmetric solutions
    }

	/* Should end before anything else (printing comes later) */
	clock_gettime(CLK, &end_e2e);
	e2e = diff(start_e2e, end_e2e);
	alg = diff(start_alg, end_alg);

	//outputFile = fopen(outputFileName,"w");
	//fprintf(outputFile,"%.8f\n",pi);
    cout << totalSolutionCount << endl;

	/* problem_name,approach_name,n,p,e2e_sec,e2e_nsec,alg_sec,alg_nsec
	   Change problem_name to whatever problem you've been assigned
	   Change approach_name to whatever approach has been assigned
	   p should be 0 for serial codes!!
	 */
	printf("%s,%s,%d,%d,%d,%ld,%d,%ld\n", problem_name, approach_name, n, p, e2e.tv_sec, e2e.tv_nsec, alg.tv_sec, alg.tv_nsec);

	return 0;

}
