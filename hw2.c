#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <stdint.h>
#include <math.h>
#include <time.h>

#define MAXP 64 // maximum number of allowed processes
#define MAX  (1 << 18) // maximum of participant 
#define SIZE 4


int simulate_game(void) {
usleep(10);
return (rand() %2);
}


int main(int argc, char* argv[]) {

	// Initialize the MPI environment
	MPI_Init(&argc, &argv);
	int n; // number of legs in the tournament

	if(argc < 2)
	{
		printf("Input file is required\n");
		return -1;
	}
	n = atoi(argv[1]);
	int numTeams = (1 << n); // 2^n number of teams




	// Get the number of processes
	int world_size;
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	// Get the rank of the running process
	int world_rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);


	int arr[MAXP][2] = {0};
	if(world_rank == 0)
	{
	 // each process will receive a message consist of single element (size)
	int j;
	
	for(j=0;j<world_size;j++)
	{
			arr[j][0] = numTeams/world_size; // size of set
		  arr[j][1] = j*numTeams/world_size; // start of set
	}
	}

	srand(time(NULL) + world_rank);
	
	int recvbuf[2] = {0};
	int	sendcount =  2;
	int recvcount =  2;

	MPI_Scatter(arr,sendcount,MPI_INT,recvbuf,recvcount,MPI_INT,0,MPI_COMM_WORLD);

	double t0,tf;
	t0 = MPI_Wtime();
	int i;
	int Rem = recvbuf[0];
	int Set[MAX] = {0};
	int indx = 0;
	for(i=0;i<Rem;i++)
		Set[i] = i + recvbuf[1];

	while(Rem != 1)
	{
		for(i=0;i<Rem;i+=2)
		{
			Set[indx] = Set[i + simulate_game()] ;
			indx++;
		}
		indx = 0;
		Rem = (Rem >> 1);
	}

	//printf("process %d winner is %d \n",world_rank,Set[0]);
	MPI_Gather(Set,1,MPI_INT,Set,1,MPI_INT,0, MPI_COMM_WORLD);

	if(world_rank == 0)
	{
		Rem = world_size;
		indx = 0;
		while(Rem != 1)
		{
			for(i=0;i<Rem;i+=2)
			{
				Set[indx] = Set[i + simulate_game()] ;
				indx++;
			}
			indx = 0;
			Rem = (Rem >> 1);
		}
		tf = MPI_Wtime();
		printf("The_Winner  %d\n",Set[0]);
		printf("Total_Time %f\n",(tf-t0));
	}
	
	MPI_Finalize();

}


