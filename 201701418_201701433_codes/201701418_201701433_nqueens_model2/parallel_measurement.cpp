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

int totalSolutionCount = 0;			//stores total number of solutions

bool checkDefence(int board[],int row,int col)
{
  for(int i=0;i<row;i++)
  {
    //vertical attacks on the queen
    if(board[i] == col)
    {
      return false;
    }

    //diagonal attacks on the queen
    if(abs(board[i] - col) == (row - i))
    {
      return false;
    }
  }
  return true;
}

void solveBoard(int board[],int boardSize,int row,int col)
{
  if(!checkDefence(board,row,col))		//checks whether (row,col) is a valid placement or not
  {
    return;
  }

  board[row] = col;
  if(row == boardSize - 1)			//is true when N x N board is completely traversed
  {
    #pragma omp critical			//using critical to avoid the recalculation of solution
    {
      totalSolutionCount++;
    }
    return;
  }

  for(int i=0;i<boardSize;i++)
  {
    solveBoard(board,boardSize,row+1,i);		//recursive call to next row
  }
}

void solveNQueens(int n,int p)
{
  omp_set_num_threads(p);			//using p processors to execute the code parallelly
  #pragma omp parallel
  #pragma omp single
  {
    for(int i=0;i<n;i++)
    {
			#pragma omp task		//distributing task with each column of first row as the start
			{
				int *board = new int[n];
	      		solveBoard(board,n,0,i);
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
	char *approach_name = "model2";
//	char buffer[10];
//	FILE* inputFile;
	FILE* outputFile;
	//	inputFile = fopen(argv[3],"r");

	char outputFileName[50];
	sprintf(outputFileName,"output/%s_%s_%s_%s_output.txt",problem_name,approach_name,argv[1],argv[2]);

	clock_gettime(CLK, &start_alg);	/* Start the algo timer */

	/*----------------------Core algorithm starts here----------------------------------------------*/

  	solveNQueens(n,p);

	/*----------------------Core algorithm finished--------------------------------------------------*/

	clock_gettime(CLK, &end_alg);	/* End the algo timer */
	/* Ensure that only the algorithm is present between these two
	   timers. Further, the whole algorithm should be present. */


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
