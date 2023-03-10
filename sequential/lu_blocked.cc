#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fstream>
#include <ctime>
#include <chrono>
#include <sys/time.h>
#include "blocked.h"

using namespace std;
using std::chrono::duration_cast;
using std::chrono::milliseconds;
using std::chrono::seconds;
using std::chrono::system_clock;

void print_matrix(double *A, int N, int n)
{
  for (int i = 0; i < N; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      printf("%.4f ", A[i * n + j]);
    }
    printf("\n");
  }
}

int main(int argc, char **argv)
{
  if (argc != 4)
  {
    fprintf(stderr, "must provide exactly 3 arguments N block_size output_filename\n");
    return 1;
  }
  typedef std::chrono::milliseconds ms;
  auto total_starttime = duration_cast<ms>(system_clock::now().time_since_epoch()).count();

  // parsing argument
  int N = atoi(argv[1]);
  int B = atoi(argv[2]);
  char *out_filename = argv[3];

  // generate matrix
  // srand((unsigned)time(NULL));
  int n = N;
  if ((N % B) != 0)
    n = B * (N / B) + B;

  double *A = (double *)malloc(n * n * sizeof(double));
  double *L = (double *)malloc(N * N * sizeof(double));
  for (int i = 0; i < N; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      A[i * n + j] = 1 + (rand() % 10000);
      L[i * N + j] = 0;
    }
    // ensure diagonally dominant
    A[i * n + i] = A[i * n + i] + 10000 * N;
  }

  // do the padding
  if ((N % B) != 0)
  {
    for (int i = N; i < n; ++i)
    {
      for (int j = 0; j < n; ++j)
      {
        A[i * n + j] = 1000 + i + j;
      }
      A[i * n + i] = A[i * n + i] + 10000 * n;
    }
    for (int j = N; j < n; ++j)
    {
      for (int i = 0; i < n - N; ++i)
      {
        A[i * n + j] = 1000 + i + j;
      }
    }
  }

  // print matrix before lu factorization
  if (N < 11)
  {
    printf("the matrix before lu factorization is\n");
    print_matrix(A, N, n);
  }

  // blocked lu factorization
  blocked_lu(B, N, A, L);

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
    print_matrix(A, N, n);
    printf("L is\n");
    print_matrix(L, N, N);
  }

  // write result to output file
  ofstream out_file(out_filename);
  for (int i = 0; i < N; ++i)
  {
    for (int j = 0; j < N; ++j)
    {
      out_file.write((char *)&A[i * n + j], sizeof(double));
    }
  }
  for (int i = 0; i < N * N; ++i)
  {
    out_file.write((char *)&L[i], sizeof(double));
  }
  out_file.close();
  free(A);
  free(L);

  // calculate total spent time
  auto total_endtime = duration_cast<ms>(system_clock::now().time_since_epoch()).count();
  printf("total time spent for blocked lu %ld ms\n", (total_endtime - total_starttime));
}