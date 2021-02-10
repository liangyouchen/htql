#ifndef TMATRIX_H_CLY_2009_06_19
#define TMATRIX_H_CLY_2009_06_19
#include "stdio.h"

#undef max
#undef min

template <class T>
class tMatrix {
public:
	T* Cells; 
	long Rows; 
	long Cols;
public:
	int newDim(long rows, long cols){
		reset(); 
		Rows=rows; Cols=cols; 
		Cells=new T[Rows*Cols]; 
		for (long i=Rows*Cols-1; i>=0; i--) Cells[i]=0; 
		return 0; 
	}
	int expandDim(long rows, long cols){
		T* newcells=new T[rows*cols];
		long i, j;
		for (i=rows*cols-1; i>=0; i--) newcells[i]=0; 
		for (i=0; i<Rows; i++){
			for (j=0; j<Cols; j++){
				newcells[i*cols+j]=Cells[i*Cols+j];
			}
		}
		delete[] Cells; 
		Cells=newcells; 
		Rows=rows; 
		Cols=cols;
		return 0;
	}
	inline T& operator()(long i, long j){
		return Cells[i*Cols+j];
	}
	inline T& operator()(long i){
		return Cells[i];
	}
	inline tMatrix<T>& operator=(tMatrix<T>& m1){
		newDim(m1.Rows, m1.Cols);
		for (long i=Rows*Cols-1; i>=0; i--) Cells[i]=m1.Cells[i]; 
		return *this;
	}
	inline tMatrix<T>& operator=(T m1){
		for (long i=Rows*Cols-1; i>=0; i--) Cells[i]=m1; 
		return *this;
	}
	inline bool operator==(tMatrix<T>& m1){
		for (long i=Rows*Cols-1; i>=0; i--) {
			if (Cells[i]!=m1.Cells[i]){
				return false;
			}
		}
		return true;
	}
	inline bool operator!=(tMatrix<T>& m1){
		for (long i=Rows*Cols-1; i>=0; i--) {
			if (Cells[i]!=m1.Cells[i]){
				return true;
			}
		}
		return false;
	}
	inline tMatrix<T>&  operator+=(tMatrix<T>& m1){
		for (long i=Rows*Cols-1; i>=0; i--) {
			Cells[i]+=m1.Cells[i];
		}
		return *this;
	}
	inline tMatrix<T>& operator+=(T m1){
		for (long i=Rows*Cols-1; i>=0; i--) Cells[i]+=m1; 
		return *this;
	}
	inline tMatrix<T>&  operator-=(tMatrix<T>& m1){
		for (long i=Rows*Cols-1; i>=0; i--) {
			Cells[i]-=m1.Cells[i];
		}
		return *this;
	}
	inline tMatrix<T>& operator-=(T m1){
		for (long i=Rows*Cols-1; i>=0; i--) Cells[i]-=m1; 
		return *this;
	}
	inline tMatrix<T>&  operator*=(tMatrix<T>& m1){
		for (long i=Rows*Cols-1; i>=0; i--) {
			Cells[i]*=m1.Cells[i];
		}
		return *this;
	}
	inline tMatrix<T>& operator*=(T m1){
		for (long i=Rows*Cols-1; i>=0; i--) Cells[i]*=m1; 
		return *this;
	}
	inline T& max(){
		T* m=Cells; 
		long total=Rows*Cols-1; 
		for (long i=1; i<total; i++) {
			if (Cells[i]>*m) m=Cells+i;
		}
		return *m;
	}
	inline T& min(){
		T* m=Cells; 
		long total=Rows*Cols-1; 
		for (long i=1; i<total; i++) {
			if (Cells[i]<*m) m=Cells+i;
		}
		return *m;
	}
	inline T sum(){
		T m=0; 
		long total=Rows*Cols-1; 
		for (long i=Rows*Cols-1; i>=0; i--) {
			m+=Cells[i];
		}
		return m;
	}
	inline void print(const char* filename, const char* fmt="%5.2lf "){
		FILE* f=fopen(filename, "w+"); 
		if (f){
			for (long i=0; i<Rows; i++){
				for (long j=0; j<Cols; j++){
					fprintf(f, fmt, Cells[i*Cols+j]); //assume vectors arranged in a row
				}
				fprintf(f, "\n"); 
			}
			fclose(f);
		}
		return;
	}
public:
	tMatrix(){
		Cells=0; 
		Cols=Rows=0; 
	}
	~tMatrix(){
		reset(); 
	}
	void reset(){
		if (Cells) {
			delete[] Cells; 
			Cells=0; 
		}
		Cols=Rows=0; 
	}
}; 

#endif
