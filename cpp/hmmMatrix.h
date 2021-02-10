#ifndef HMM_MATRIX_H
#define HMM_MATRIX_H
#include <stdio.h>

class HmmMatrix{
public:
	int setCellSize(int cellsize);
	inline int getCellSize();

	int setMatrixSize(long row, long col);
	int setCol(long col);
	int setRow(long row);
	long getCol();
	long getRow();

	char* getCell(long row, long col);
	char* operator() (long row, long col);

	int insertCol(long col);
	int insertRow(long row);
	int dropCol(long col);
	int dropRow(long row);
	void clearCells();

	enum {UnknownMatrix, IntMatrix, LongMatrix, DoubleMatrix};
	int dumpMatrix(FILE* f, char* description,int matrix_type, const char*row_str=0, const char* col_str=0);
	int dumpMatrix(char* filename, char* description, int matrix_type, const char*row_str=0, const char* col_str=0);

	HmmMatrix(int cellsize=sizeof(long) );
	~HmmMatrix();
	void reset();

protected:
	int CellSize;
	long Col;
	long Row;
	char* Cells;

	int createCells();
};



#endif
