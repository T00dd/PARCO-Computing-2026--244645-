#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "timer.h"
#include "mmio.h"

#define MAX_RAND 10000
#define MIN_RAND -10000

typedef struct{
	int* csr_vector;
	int* csr_col;
	double* csr_val;
}csr_matrix;

csr_matrix coo_to_csr(int M_, int nz_, int *I_, int* J_, double* val_);
double* vect_generator(int M_);

int main(int argc, char *argv[]){
	
	srand(time(NULL));

	if(argc != 2){
		printf("Usage: %s [path of the matrix]\n", argv[0], argv[1]);
		return -1;
	}

	double *val;
	int *I, *J, M, N, nz;
	//I = columns, J = rows

	if (mm_read_unsymmetric_sparse(argv[1], &M, &N, &nz, &val, &I, &J)){
		printf("Unable to read the Matrix!\n");
		return -1;
	}

	printf("Lines: %d, Columns: %d, Non zero values: %d\n", M, N, nz);
	//printf("%d, %d, %f\n", I[3], J[3], val[3]);

	printf("Translating the matrix from COO to CSR...\n");
	csr_matrix csr = coo_to_csr(M, nz, I, J, val);

	printf("Creating random vector...\n");
	double* random_vector = vect_generator(M);

	free(I);
	free(J);
	free(val);

	free(csr.csr_col);
	free(csr.csr_val);
	free(csr.csr_vector);

	return 0;
}

csr_matrix coo_to_csr(int M_, int nz_, int *I_, int* J_, double* val_){

	csr_matrix csr_mat;

	csr_mat.csr_col = (int*) malloc(nz_ * sizeof(int));
	csr_mat.csr_val = (double*) malloc(nz_ * sizeof(double));
	if (csr_mat.csr_col == NULL || csr_mat.csr_val == NULL) exit(1);
	
	csr_mat.csr_vector = (int *) calloc((M_ + 1), sizeof(int));
	if (csr_mat.csr_vector == NULL) exit(1);
	
	//counting the nz elements for each row

	for(int i = 0; i < nz_; i++){
		csr_mat.csr_vector[I_[i] + 1]++;
	}

	//prefix sum

	for (int i = 1; i <= M_; i++){
		csr_mat.csr_vector[i] += csr_mat.csr_vector[i-1];
	}

	//reordering 
	int* row_pos = (int*) malloc((M_ + 1) * sizeof(int));
    if (row_pos == NULL) exit(1);
    memcpy(row_pos, csr_mat.csr_vector, (M_ + 1) * sizeof(int));

    for (int i = 0; i < nz_; i++) {
        int row_index = I_[i];
        int dest_idx = row_pos[row_index];

        // Copia i dati COO nella posizione CSR corretta (riordinamento implicito)
        csr_mat.csr_col[dest_idx] = J_[i];
        csr_mat.csr_val[dest_idx] = val_[i];

        row_pos[row_index]++;
    }

    free(row_pos);
	
	return csr_mat;
}

double* vect_generator(int M_){
	
	double* vect = (double *) malloc(M_ * sizeof(double));
	for (int i = 0; i < M_; i++){ vect[i] = (double) (rand()% (MAX_RAND - MIN_RAND + 1) + MIN_RAND) / 1000.0; }
	return vect;	
}
