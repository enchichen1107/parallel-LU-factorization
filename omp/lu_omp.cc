#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <sys/time.h>
#include <omp.h>

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

void print_matrix(double *A, int N)
{
  for (int i = 0; i < N; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      printf("%.4f ", A[i * N + j]);
    }
    printf("\n");
  }
}

int main(int argc, char **argv)
{
  if (argc != 3)
  {
    fprintf(stderr, "must provide exactly 2 arguments N output_filename\n");
    return 1;
  }
  //typedef std::chrono::milliseconds ms;
  //auto total_starttime = duration_cast<ms>(system_clock::now().time_since_epoch()).count();
  double start; 
  double end; 
  start = omp_get_wtime(); 
  // parsing argument
  int N = atoi(argv[1]);
  char *out_filename = argv[2];

  // generate matrix
  // srand((unsigned)time(NULL));
  double *A = (double *)malloc(N * N * sizeof(double));
  double *L = (double *)malloc(N * N * sizeof(double));
  for (int i = 0; i < N; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      A[i * N + j] = 1 + (rand() % 10000);
      L[i * N + j] = 0;
    }
    // ensure diagonally dominant
    A[i * N + i] = A[i * N + i] + 10000 * N;
  }

  // print matrix before lu factorization
  if (N < 11)
  {
    printf("the matrix before lu factorization is\n");
    print_matrix(A, N);
  }

  // basic lu factorization
    #pragma omp parallel
    {
        for (int k = 0; k < N - 1; ++k) {
            #pragma omp for schedule(static)
            for (int i = k + 1; i < N; ++i) {
                L[i * N + k] = A[i * N + k] / A[k * N + k];
            }
            #pragma omp for schedule(static)
            for (int j = k; j < N; ++j) {
                for (int i = k; i < N; ++i) {
                    A[i * N + j] = A[i * N + j] - L[i * N + k] * A[k * N + j];
                }
            }
        }
    }

  // assign 1 to diagonal of L
  for (int i = 0; i < N; ++i)
  {
    L[i * N + i] = 1;
  }

  // print outcome
  if (N < 11)
  {
    printf("the lu factorization outcome is\n");
    printf("U is\n");
    print_matrix(A, N);
    printf("L is\n");
    print_matrix(L, N);
  }

  // write result to output file
  ofstream out_file(out_filename);
  for (int i = 0; i < N * N; ++i)
  {
    out_file.write((char *)&A[i], sizeof(float));
  }
  for (int i = 0; i < N * N; ++i)
  {
    out_file.write((char *)&L[i], sizeof(float));
  }
  out_file.close();
  free(A);
  free(L);

  // calculate total spent time
  //auto total_endtime = duration_cast<ms>(system_clock::now().time_since_epoch()).count();
  //printf("total time spent for basic lu %ld ms\n", (total_endtime - total_starttime));
  end = omp_get_wtime(); 
  printf("omp work took %f seconds\n", end - start);
}

