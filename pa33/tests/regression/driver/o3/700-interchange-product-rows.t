// The product of two row-major matrices: rows swap in, their address hoists, lanes run.
const int L = 50;
double C[L*L], D[L*L], E[L*L];
void f() {
    for( int i=0; i<L; i++ )
	for( int j=0; j<L; j++ ) {
	    double sum = 0;
	    for( int k=0; k<L; k++ )
		sum += C[L*i+k]*D[L*k+j];
	    E[L*i+j] = sum;
	}
}
