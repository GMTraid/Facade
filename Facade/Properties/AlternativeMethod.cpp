#include "stdafx.h"
#include "mpi.h"
#include <cstdlib>
#include <fstream>
#include <vector>

using namespace std;
#ifdef MATHFUNCSDLL_EXPORTS
#define MATHFUNCSDLL_API __declspec(dllexport) 
#else
#define MATHFUNCSDLL_API __declspec(dllimport) 
#endif

namespace MathFuncs
{
	// This class is exported from the MathFuncsDll.dll
	class MyMathFuncs
	{
	public:
		// Returns a + b
		static MATHFUNCSDLL_API double Calculate(int argc, char* argv[]);
	};
}

namespace MathFuncs
{
	double MyMathFuncs::Calculate(int argc, char* argv[])
	{
		double t_start, t_end; // time measure
		int n = 1000; // size of matrix
		double* A = new double[n*(n + 1)]; // matrix
		double* X = new double[n]; // variables
		double buf;
		double* rbuf = new double[n*(n + 1)];
		double* row_buf = new double[n + 1];
		int i, j, k;
		ofstream fout;
		int *pSendNum;    // Количество элементов, посылаемых процессу
		int *pSendInd;    // Индекс первого элемента данных, посылаемого процессу
		int procs_rank, procs_count;
		MPI_Init(&argc, &argv);
		MPI_Comm_size(MPI_COMM_WORLD, &procs_count);
		MPI_Comm_rank(MPI_COMM_WORLD, &procs_rank);

		// Init
		if (procs_rank == 0)
		{
			fout.open("output.txt");
			t_start = MPI_Wtime();
			const double zeroEps = 0.0000000001;
			// generate matrix
			for (i = 0; i < n; i++)
			{
				for (j = 0; j < n + 1; j++)
					A[i*(n + 1) + j] = -500 + rand() % 1001; //i*(n + 1) + j; //
				if (abs(A[i*(n + 1) + i]) < zeroEps)
					A[i*(n + 1) + i] = 1;
			}
		}
		// Gauss
		for (k = 0; k < n; k++)
		{
			if (procs_rank == 0)
				for (j = 0; j < n + 1; j++)
					row_buf[j] = A[k*(n + 1) + j];
			MPI_Bcast(row_buf, n + 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			pSendInd = new int[procs_count];
			pSendNum = new int[procs_count];
			int RowsLeft = n - k - 1;
			int Rows = (n - k - 1) / procs_count;
			pSendNum[0] = Rows * (n + 1);
			pSendInd[0] = 0;
			for (j = 1; j < procs_count; j++)
			{
				RowsLeft -= Rows;
				Rows = RowsLeft / (procs_count - j);
				pSendNum[j] = Rows * (n + 1);
				pSendInd[j] = pSendInd[j - 1] + pSendNum[j - 1];
			}
			MPI_Scatterv(&A[(k + 1)*(n + 1)], pSendNum, pSendInd, MPI_DOUBLE, rbuf,
				pSendNum[procs_rank], MPI_DOUBLE, 0, MPI_COMM_WORLD);
			for (i = 0; i < pSendNum[procs_rank] / (n + 1); i++)
			{
				buf = rbuf[i*(n + 1) + k] / row_buf[k];
				for (j = k; j < n + 1; j++)
					rbuf[j] = rbuf[j] - buf * row_buf[j];
			}
			MPI_Gatherv(rbuf, pSendNum[procs_rank], MPI_DOUBLE,
				&A[(k + 1)*(n + 1)], pSendNum, pSendInd, MPI_DOUBLE, 0, MPI_COMM_WORLD);
			delete[] pSendInd, pSendNum;
		}

		if (procs_rank == 0)
		{
			X[n - 1] = A[(n - 1)*(n + 1) + n] / A[(n - 1)*(n + 1) + n - 1];
			for (i = n - 2; i >= 0; i--)
			{
				buf = 0;
				for (j = i + 1; j < n; j++)
					buf = buf + A[i*(n + 1) + j] * X[j];
				X[i] = 1.0 / A[i*(n + 1) + i] * (A[i*(n + 1) + n] - buf);
			}
			t_end = MPI_Wtime();
		}
		// Final
		if (procs_rank == 0)
		{
			fout << "Time elapsed: " << t_end - t_start << '\n';
			for (i = 0; i < n; i++)
				fout << "X" << i + 1 << ": " << X[i] << '\n';
			fout.close();
			delete[] A, X;
		}
		MPI_Finalize();
		return 0;		
	}	
}
