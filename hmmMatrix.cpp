#include "hmmMatrix.h"

#include <stdlib.h>
#include <memory.h>
#include <math.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



HmmMatrix::HmmMatrix(int cellsize){
	CellSize=cellsize;
	if (CellSize<=0) CellSize = sizeof(char);

	Cells = 0;
	Col=Row=0;
}

HmmMatrix::~HmmMatrix(){
	reset();
}
void HmmMatrix::reset(){
	if (Cells){
		free(Cells);
		Cells=0;
	}
	Col=Row=0;
}

int HmmMatrix::setCellSize(int cellsize){
	reset();
	CellSize=cellsize;
	if (CellSize<=0) CellSize = sizeof(char);
	return 0;
}

int HmmMatrix::setMatrixSize(long row, long col){
	setRow(row);
	setCol(col);
	return 1;
}

int HmmMatrix::setCol(long col){
	if (Cells) reset();
	Col=col;

	if (Row) return createCells();
	return 1;
}
int HmmMatrix::setRow(long row){
	if (Cells) reset();
	Row=row;

	if (Col) return createCells();
	return 0;
}

long HmmMatrix::getCol(){
	return Col;
}
long HmmMatrix::getRow(){
	return Row;
}

int HmmMatrix::getCellSize(){
	return CellSize;
}

char* HmmMatrix::getCell(long row, long col){
	return Cells+(Col*row+col)*CellSize;
}

char* HmmMatrix::operator() (long row, long col){
	return Cells+(Col*row+col)*CellSize;
}

int HmmMatrix::createCells(){
	if (!Row || !Col) return 1;
	if (Cells){
		free(Cells);
		Cells=0;
	}
	Cells = (char*) malloc(CellSize*Col*Row);
	if (!Cells) return -1;
	memset(Cells, 0, CellSize*Col*Row);
	return 0;
}

void HmmMatrix::clearCells(){
	if (!Row || !Col) return ;
	memset(Cells, 0, CellSize*Col*Row);
}

int HmmMatrix::insertCol(long col){
	if (col>Col) return -1;
	if (Col==0 && Row==0){
		Row=1;
	}
	char* tmpcell = (char*) malloc(CellSize*Row*(Col+1));
	for (long i=0; i<Row; i++){
		if (col>0)
			memcpy(tmpcell+i*(Col+1)*CellSize, Cells+i*Col*CellSize, CellSize*col);
		if (col<Col)
			memcpy(tmpcell+(i*(Col+1)+col+1)*CellSize, Cells+(i*Col+col)*CellSize, CellSize*(Col-col));
		memset(tmpcell+(i*(Col+1)+col)*CellSize, 0, CellSize);
	}
	free(Cells);
	Cells = tmpcell;

	Col++;
	return 0;
}
int HmmMatrix::insertRow(long row){
	if (row>Row) return -1;
	if (Col==0 && Row==0){
		Col=1;
	}
	char* tmpcell = (char*) malloc(CellSize*(Row+1)*(Col));
	if (row>0)
		memcpy(tmpcell, Cells, CellSize*Col*row);
	if (row < Row)
		memcpy(tmpcell+Col*(row+1)*CellSize, Cells+Col*row*CellSize, CellSize*Col*(Row-row));
	memset(tmpcell+Col*row*CellSize, 0, CellSize*Col);
	free(Cells);
	Cells = tmpcell;

	Row++;
	return 0;
}

int HmmMatrix::dropCol(long col){
	if (Col<=col) return -1;
	char* tmpcell = (char*) malloc(CellSize*Row*(Col-1));
	for (long i=0; i<Row; i++){
		if (col>0)
			memcpy(tmpcell+i*(Col-1)*CellSize, Cells+i*Col*CellSize, CellSize*(col));
		if (col<Col-1)
			memcpy(tmpcell+(i*(Col-1)+col)*CellSize, Cells+(i*Col+col+1)*CellSize, CellSize*(Col-col-1));
	}
	if (Cells) free(Cells);
	Cells = tmpcell;

	Col--;
	return 0;
}
int HmmMatrix::dropRow(long row){
	if (Row<=row) return -1;
	char* tmpcell = (char*) malloc(CellSize*(Row-1)*(Col));
	if (row>0)
		memcpy(tmpcell, Cells, CellSize*Col*(row));
	if (row < Row-1)
		memcpy(tmpcell+Col*(row)*CellSize, Cells+Col*(row+1)*CellSize, CellSize*Col*(Row-row-1));
	if (Cells) free(Cells);
	Cells = tmpcell;

	Row--;
	return 0;
}

int HmmMatrix::dumpMatrix(char* filename, char* description,int matrix_type, const char*row_str, const char* col_str){
	FILE* f=fopen(filename, "w+");
	int i=dumpMatrix(f, description, matrix_type,row_str,col_str);
	fclose(f);
	return i;
}

int HmmMatrix::dumpMatrix(FILE* f, char* description,int matrix_type, const char*row_str, const char* col_str){
	long i, v;
	long row=getRow();
	long col=getCol();
	if (description){
		fprintf(f, "%s\n", description);
	}
	for (i=0; i<row; i++){
		if (i%3==0){
			fprintf(f, "row (i) =%d\n----", i);
			for (v=0; v<col; v++){
				switch (matrix_type){
				case IntMatrix: 
					fprintf(f, "-%4d%c", v, col_str?col_str[v]:'-');
					break;
				case DoubleMatrix: 
					fprintf(f, "-%6d%c", v, col_str?col_str[v]:'-');
					break;
				case LongMatrix:
					fprintf(f, "-%4d%c", v, col_str?col_str[v]:'-');
					break;
				default:
					fprintf(f, "-%d%c", v%10, col_str?col_str[v]:'-');
					break;
				}
			}
			fprintf(f, "\n");
		}
		fprintf(f, "(%2d)%c", i, row_str?row_str[i]:' ');
		for (v=0; v<col; v++){
			switch (matrix_type){
			case IntMatrix: 
				fprintf(f, "%5d ", *(int*)(*this)(i,v));
				break;
			case DoubleMatrix: 
				fprintf(f, "%8.3lf  ", *(double*)(*this)(i,v));
				break;
			case LongMatrix:
				fprintf(f, "%5ld ", *(long*)(*this)(i,v));
				break;
			default:
				fprintf(f, "%c ", *(*this)(i,v));
				break;
			}
		}
		fprintf(f, "\n");
	}
	return 0;
}
