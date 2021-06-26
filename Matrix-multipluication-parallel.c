#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define MASTER 0      /* taskid of first task */
#define FROM_MASTER 1 /* setting a message type */
#define FROM_WORKER 2 /* setting a message type */

int main(int argc, char **argv)
{
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int o = atoi(argv[3]);
    int numtasks;   /* number of tasks in partition */
    int taskid;     /* a task identifier */
    int numworkers; /* number of worker tasks */
    int source;     /* task id of message source */
    int dest;       /* task id of message destination */
    int mtype;      /* message type */
    int rows;       /* rows of matrix A sent to each worker */
    int averow;
    int extra;
    int offset;         /* used to determine rows sent to each worker */
    int i, j, k, rc;    /* misc */
    double a[m][n]; /* matrix A to be multiplied */
    double b[n][o]; /* matrix B to be multiplied */
    double c[m][o]; /* result matrix C */
    MPI_Status status;
    /* Initialize MPI Environment */
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &taskid);
    MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
    double  elapsed_time = -MPI_Wtime();
    if (numtasks < 2)
    {
        printf("Need at least two MPI tasks. Quitting...\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
        exit(1);
    }
    numworkers = numtasks - 1;
    /* Master block*/
    if (taskid == MASTER)
    {
        printf("mpi_mm has started with %d tasks.\n", numtasks);
        printf("Initializing arrays...\n");
        for (i = 0; i < m; i++)
            for (j = 0; j < n; j++)
                a[i][j] = i + j; /* Initialize array a */
        for (i = 0; i < n; i++)
            for (j = 0; j < o; j++)
                b[i][j] = i * j; /* Initialize array b */
        /* Send matrix data to the worker tasks */
        averow = m / numworkers; /* determining fraction of array to be processed by “workers” */
        extra = m % numworkers;
        offset = 0;
        mtype = FROM_MASTER; /* Message Tag */
        for (dest = 1; dest <= numworkers; dest++)
        { /* To each worker send : Start point, number of rows to process, and sub-arrays to process */
            rows = (dest <= extra) ? averow + 1 : averow;
            printf("Sending %d rows to task %d offset=%d\n", rows, dest, offset);
            MPI_Send(&offset, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&rows, 1, MPI_INT, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&a[offset][0], rows * n, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
            MPI_Send(&b, n * o, MPI_DOUBLE, dest, mtype, MPI_COMM_WORLD);
            offset = offset + rows;
        }
        /* Receive results from worker tasks */
        mtype = FROM_WORKER; /* Message tag for messages sent by “workers” */
        for (i = 1; i <= numworkers; i++)
        {
            source = i;
            /* offset stores the (processing) starting point of work chunk */
            MPI_Recv(&offset, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
            MPI_Recv(&rows, 1, MPI_INT, source, mtype, MPI_COMM_WORLD, &status);
            /* The array C contains the product of sub-array A and the array B */
            MPI_Recv(&c[offset][0], rows * o, MPI_DOUBLE, source, mtype, MPI_COMM_WORLD, &status);
            printf("Received results from task %d\n", source);
        }
        printf("******************************************************\n");
        printf("Result Matrix:\n");
        for (i = 0; i < m; i++)
        {
            printf("\n");
            for (j = 0; j < o; j++)
                printf("%6.2f ", c[i][j]);
        }
        printf("\n******************************************************\n");
        printf("Done.\n");
    }
    /**************************** worker task ************************************/
    if (taskid > MASTER)
    {
        mtype = FROM_MASTER;
        MPI_Recv(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&a, rows * n, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
        MPI_Recv(&b, n * o, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD, &status);
        for (k = 0; k < o; k++)
            for (i = 0; i < rows; i++)
            {
                c[i][k] = 0.0;
                for (j = 0; j < n; j++)
                    /* Calculate the product and store result in C */
                    c[i][k] = c[i][k] + a[i][j] * b[j][k];
            }
        mtype = FROM_WORKER;
        MPI_Send(&offset, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
        MPI_Send(&rows, 1, MPI_INT, MASTER, mtype, MPI_COMM_WORLD);
        /* Worker sends the resultant array to the master */
        MPI_Send(&c, rows * o, MPI_DOUBLE, MASTER, mtype, MPI_COMM_WORLD);

    }
    elapsed_time += MPI_Wtime();
    printf("Total elapsed time: %10.6fs\n", elapsed_time);
    MPI_Finalize();
}