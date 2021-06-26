// #include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv)
{
    clock_t begin = clock();
    int m = atoi(argv[1]);
    int n = atoi(argv[2]);
    int o = atoi(argv[3]);
    int i, j, k, rc;    /* misc */

    double **a=malloc(sizeof(double *) * m);
    for (int i=0;i < n;i++)
        a[i]=malloc(sizeof(double)*n); /* matrix A to be multiplied */

    double **b=malloc(sizeof(double *) * n);
    for (int i=0;i < o;i++)
        b[i]=malloc(sizeof(double)*o);

    double **c=malloc(sizeof(double *) * m);
    for (int i=0;i<o;i++)
        c[i]=malloc(sizeof(double)*o);

    printf("Initializing arrays...\n");
    for (i = 0; i < m; i++)
    {
        for (j = 0; j < n; j++)
            a[i][j] = i + j; /* Initialize array a */
    }
    for (i = 0; i < n; i++)
    {
        for (j = 0; j < o; j++)
            b[i][j] = i * j; /* Initialize array b */
    }

    for (k = 0; k < m; k++)
        for (i = 0; i < o; i++)
        {
            c[i][k] = 0.0;
            for (j = 0; j < n; j++)
                /* Calculate the product and store result in C */
                c[i][k] = c[i][k] + a[i][j] * b[j][k];
        }

    printf("Result Matrix:\n");
    for (i = 0; i < m; i++)
    {
        printf("\n");
        for (j = 0; j < o; j++)
            printf("%6.2f ", c[i][j]);
    }
    printf("\n******************************************************\n");
    printf("Done.\n");
    clock_t end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    printf("Total elapsed time: %10.6fs\n", time_spent);
    return 0;
}
