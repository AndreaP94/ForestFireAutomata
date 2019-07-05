#include <iostream>
#include <ctime>
#include <cstdlib>
#include <mpi.h>

using namespace std;

#define COLS  16
#define ROWS  16

int main(int argc, char **argv) {

    int p, rank;

    MPI_Init(&argc, &argv);

    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    char i;

    int a[ROWS][COLS];
    const int NPROWS = p;  /* number of rows in _decomposition_ */
    //const int NPCOLS=3;  /* number of cols in _decomposition_ */
    const int BLOCKROWS = ROWS/NPROWS;  /* number of rows in _block_ */
    const int BLOCKCOLS = COLS; /* number of cols in _block_ */

    if (rank == 0) {
        for (int ii=0; ii<ROWS; ii++) {
          for (size_t jj = 0; jj < COLS; jj++) {
            a[ii][jj] = ii;
          }
        }
    }

    /*
    if (p != NPROWS*NPCOLS) {
        fprintf(stderr,"Error: number of PEs %d != %d x %d\n", p, NPROWS, NPCOLS);
        MPI_Finalize();
        exit(-1);
    }
    */
    int b[BLOCKROWS*BLOCKCOLS];
    //for (int ii=0; ii<BLOCKROWS*BLOCKCOLS; ii++)
    //  b[ii] = 0;

    //MPI_Datatype blocktype;
    //MPI_Datatype blocktype2;
    //MPI_Type_vector(BLOCKROWS, BLOCKCOLS, COLS, MPI_CHAR, &blocktype2);
    //MPI_Type_create_resized( blocktype2, 0, sizeof(char), &blocktype);

    MPI_Datatype rowtype;
    MPI_Type_contiguous(BLOCKROWS*BLOCKCOLS, MPI_INT, &rowtype);
    MPI_Type_commit(&rowtype);

    MPI_Datatype singleRowType;
    MPI_Type_contiguous(BLOCKCOLS, MPI_INT, &singleRowType);
    MPI_Type_commit(&singleRowType);

    int processFirstRow[BLOCKCOLS];
    int processLastRow[BLOCKCOLS];

    MPI_Scatter(&a, 1, rowtype, b, BLOCKROWS*BLOCKCOLS, MPI_INT, 0, MPI_COMM_WORLD);

    /*
    if(rank == 2) {
      cout << "PROCESSO " << rank << endl;
      for (int i = 0; i < BLOCKROWS*BLOCKCOLS; i++) {
        cout << b[i] << " ";
      }
    }
    */

    MPI_Status status;
    MPI_Comm arrayTopology;
    int dim = 2, left, right, reorder = false;
    int periods[dim], topologyDimensions[dim], coords[dim];

    topologyDimensions[0] = p;
    topologyDimensions[1] = 1;
    periods[0] = 1, periods[1] = 0;

    MPI_Cart_create(MPI_COMM_WORLD, dim, topologyDimensions, periods, reorder, &arrayTopology);
    MPI_Cart_shift(arrayTopology, 0, 1, &left, &right);
    MPI_Cart_coords(arrayTopology, rank, dim, coords);

    cout << "Processo " << rank << " - coordinate [" << coords[0] <<", " << coords[1] << "] - i miei vicini sono: sinistra = " << left << " destra = " << right << "\n";

    MPI_Send(&b, 1, singleRowType, left, 0, arrayTopology);
    MPI_Send(&b[(BLOCKROWS*BLOCKCOLS)-BLOCKCOLS], 1, singleRowType, right, 0, arrayTopology);

    MPI_Recv(&processLastRow, BLOCKCOLS, MPI_INT, left, 0, arrayTopology, &status);
    MPI_Recv(&processFirstRow, BLOCKCOLS, MPI_INT, right, 0, arrayTopology, &status);



    if(rank == 0){

      for(int i = 0; i < BLOCKCOLS; i++)
      cout<<processFirstRow[i] << " ";

      cout << endl;

      for(int i = 0; i < BLOCKCOLS; i++)
      cout<<processLastRow[i] << " ";
    }

    /*


    int disps[NPROWS*NPCOLS];
    int counts[NPROWS*NPCOLS];
    for (int ii=0; ii<NPROWS; ii++) {
        for (int jj=0; jj<NPCOLS; jj++) {
            disps[ii*NPCOLS+jj] = ii*COLS*BLOCKROWS+jj*BLOCKCOLS;
            counts [ii*NPCOLS+jj] = 1;
        }
    }

    MPI_Scatterv(a, counts, disps, blocktype, b, BLOCKROWS*BLOCKCOLS, MPI_CHAR, 0, MPI_COMM_WORLD);
    */
    /* each proc prints it's "b" out, in order */
  /*  for (int proc=0; proc<p; proc++) {
        if (proc == rank) {
            printf("Rank = %d\n", rank);
            if (rank == 0) {
                printf("Global matrix: \n");
                for (int ii=0; ii<ROWS; ii++) {
                    for (int jj=0; jj<COLS; jj++) {
                        printf("%3d ",(int)a[ii*COLS+jj]);
                    }
                    printf("\n");
                }
            }
            printf("Local Matrix:\n");
            for (int ii=0; ii<BLOCKROWS; ii++) {
                for (int jj=0; jj<BLOCKCOLS; jj++) {
                    printf("%3d ",(int)b[ii*BLOCKCOLS+jj]);
                }
                printf("\n");
            }
            printf("\n");
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    */
    MPI_Finalize();

    return 0;
}
