#include "tsoper.h"

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <math.h>

#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


double tTsOp::getMean(double* data, long len){
	double sum=0;
	long i;
	for (i=0; i<len; i++) sum+=data[i];
	return sum/len;
}
double tTsOp::getStd(double* data1, double* data2, long len){
	double std=0;
	long i;
	for (i=0; i<len; i++) std+=(data1[i]-data2[i])*(data1[i]-data2[i]);
	return std/len;
}
double tTsOp::getStd1(double* data1, long len, double mean){
	double std=0;
	long i;
	for (i=0; i<len; i++) std+=(data1[i]-mean)*(data1[i]-mean);
	return std/len;
}
double tTsOp::getAbs(double data){
	return data>0?data:-data;
}
double tTsOp::getMin(double* data, long len, long* min_index){
	double min=1e7;
	long i;
	for (i=0; i<len; i++) {
		if (data[i]<min) {
			min=data[i];
			if (min_index) *min_index=i;
		}
	}
	return min;
}
double tTsOp::getMax(double* data, long len, long* max_index){
	double max=-1e7;
	long i;
	for (i=0; i<len; i++) {
		if (data[i]>max) {
			max=data[i];
			if (max_index) *max_index=i;
		}
	}
	return max;
}
int tTsOp::diff1(double* data, double* result, long len){
	long i;
	result[0]=0;
	for (i=1; i<len; i++){
		result[i]=data[i]-data[i-1];
	}
	return 0;
}
int tTsOp::diff2(double* data1, double* data2, double* result, long len){
	long i;
	for (i=0; i<len; i++){
		result[i]=data2[i]-data1[i];
	}
	return 0;
}
int tTsOp::singleSmooth(double* data, long len, double alpha){
	long i;
	for (i=1; i<len; i++){
		data[i]=alpha*data[i-1] + (1-alpha)*data[i];
	}
	return 0;
}
int tTsOp::median(double* source, double* dest, long len, long lag){
	if (lag>len) lag=len; 
	long i=0,j=0 ;
	double* sort;
	long pos=0, end=0;
	long mid=lag>>1;
	int is_odd=lag%2;
	long heading=lag/2; //rounded;
	sort=(double*) malloc(sizeof(double)*(lag+1));
	memset(sort, 0, sizeof(double)*(lag+1));
	for (i=0; i<len+lag; i++){
		pos=end;
		//remove last point
		if (i>=len){ 
			for (j=0; j<end; j++){
				if (sort[j]==source[i-lag]){
					pos=j;
					break;
				}
			}
			end--;
			while (pos<end) {
				sort[pos]=sort[pos+1];
				pos++;
			}
		}else if (i<lag){
			end++;
		}else{
			for (j=0; j<end; j++){
				if (sort[j]==source[i-lag]){
					pos=j;
					break;
				}
			}
		}

		if (i<len){
			sort[pos]=source[i];
		
			//compare preceeding values (positions before pos);
			for (j=pos-1; j>=0; j--){
				if (sort[j]>source[i]){
					sort[j+1]=sort[j];
				}else{
					break;
				}
			}
			if (j<pos-1) sort[j+1]=source[i];

			if (j>=pos-1){
				//compare the proceeding values (positions after pos);
				for (j=pos+1; j<end; j++){
					if (sort[j]<source[i]){
						sort[j-1]=sort[j];
					}else{
						break;
					}
				}
				if (j>pos+1) sort[j-1]=source[i];
			}
		}

		if (i<lag-1){
			if (i%2==0)
				dest[i>>1]=sort[i>>1];
		}else if (i>=len){
			if ((len+lag-i)%2==0)
				dest[len-(len+lag-i)/2]=sort[(len+lag-i)/2-1];
		}else{
			if (is_odd)
				dest[i-heading]=sort[mid];
			else
				dest[i-heading]=(sort[mid]+sort[mid-1])/2;
		}
		//printf("i=%ld ", i); for (j=0; j<end; j++){printf("%lf ", sort[j]);} printf("\n");
	}
	free(sort);
	return 0;
}
int tTsOp::movingAvg(double* source, double*dest, long len, long lag){
	if (lag>len) lag=len; 
	double sum=0.0;
	long i=0;
	long pos1=0, pos2=0;
	long heading=lag/2; //rounded;
	for (pos1=0; pos1<len+lag; pos1++){
		if (pos1<len)
			sum+=source[pos1];
		if (pos1>=lag)
			sum-=source[pos1-lag];

		if (pos1<lag-1){
			if (pos1%2)
				dest[pos2++]=sum/(pos1+1);
		}else if (pos1<len) {
			dest[pos2++]=sum/lag;
		}
		if (pos1>=len-1){
			if ((lag+len-pos1)%2)
				dest[pos2++]=sum/(lag+len-pos1-1);
		}
		if (pos2>=len) break;
	}
	return 0;
}
int tTsOp::bubbleSort(double* data, long* index, long len, int increasing){
	long i, j;
	double val;
	long k;
	if (increasing){
		for (i=0; i<len; i++){
			for (j=len-1; j>i; j--){
				if (data[j]<data[j-1]){
					val=data[j];
					data[j]=data[j-1];
					data[j-1]=val;

					k=index[j];
					index[j]=index[j-1];
					index[j-1]=k;
				}
			}
		}
	}else{
		for (i=0; i<len; i++){
			for (j=len-1; j>i; j--){
				if (data[j]>data[j-1]){
					val=data[j];
					data[j]=data[j-1];
					data[j-1]=val;

					k=index[j];
					index[j]=index[j-1];
					index[j-1]=k;
				}
			}
		}
	}
	return 0;
}
int tTsOp::sampleAvg(double* source, long len, double sample_rate, double* dest, long* dest_len){
	double freq=1/sample_rate;
	long t=0;
	long pos=0, pos1=0, pos2=0;
	double tsum=0;

	for (pos=0; pos<len; pos++){
		tsum+=source[pos];
		if (pos==pos2){
			dest[t++]=tsum/(pos2-pos1+1);
			tsum=0;
			pos1=pos2+1;
			pos2=(long)(freq*t);
		}
	}
	if (dest_len) *dest_len=t;
	return 0;
}
int tTsOp::alignSeries(double* peak_times1, long len1, double* peak_times2, long len2, double* cost, long* result_len, double* result_times1, double* result_times2){
/*
	typedef struct {
			double diff;
			double cost;
			long prev_i;
			long prev_j;
		} SeriesAlignmentNode;

	long cols=len1+1; 
	SeriesAlignmentNode* matrix=(SeriesAlignmentNode*) malloc(sizeof(SeriesAlignmentNode)*(len1+1)*(len2+1));
	memset(matrix, 0, sizeof(SeriesAlignmentNode)*(len1+1)*(len2+1));
	long i,j,k;
	for (i=1; i<=len1; i++) {
		matrix[i].cost=getAbs(peak_times1[i-1]-peak_times1[0]);
		matrix[i].prev_i=0; //up
		matrix[i].prev_j=0;
	}
	for (j=1; j<=len2; j++) {
		matrix[j*cols].cost=getAbs(peak_times2[j-1]-peak_times1[0]);
		matrix[j*cols].prev_i=0; 
		matrix[j*cols].prev_j=0; //left
	}
	double cost=0;
	for (j=1; j<=len2; j++){
		for (i=1; i<=len1; i++){
			matrix[j*cols+i].diff=peak_times2[j-1]-peak_times1[i-1];
			matrix[j*cols+i].cost=matrix[(j-1)*cols].cost+getAbs(matrix[j*cols+i].diff-matrix[j*cols+i].diff);
			matrix[j*cols+i].prev_i=0; 
			matrix[j*cols+i].prev_j=j-1;
			for (k=0; k<i; k++){
				cost=matrix[(j-1)*cols].cost;
			}
			for (k=0; k<j; k++){
			}			
			if ((str1[i-1] == str2[j-1])) {
				matrix[j*cols+i].cost = matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnMATCH];
				matrix[j*cols+i].prev = 3;
			} else {
				cost_replace=matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnREPLACE];
				if (matrix[(j-1)*cols+(i-1)].prev==3) cost_replace+=MatchMismatchPenalty;

				cost_insert=matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT];
				if (matrix[(j-1)*cols+i].prev==3) cost_insert+=MatchMismatchPenalty;

				cost_delete=matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE];
				if (matrix[j*cols+(i-1)].prev==3) cost_delete+=MatchMismatchPenalty;

				if (cost_replace<=cost_insert && cost_replace<=cost_delete){
					matrix[j*cols+i].cost=cost_replace; 
					matrix[j*cols+i].prev=3;
				}else if (cost_insert<=cost_delete){
					matrix[j*cols+i].cost=cost_insert; 
					matrix[j*cols+i].prev=1;
				}else{
					matrix[j*cols+i].cost=cost_delete; 
					matrix[j*cols+i].prev=2;
				}
				/*g.cost = matrix[(j-1)*cols+(i-1)].cost+IndelCost[algnREPLACE];
				g.prev=3;
				if (matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT] <g.cost){
					g.cost = matrix[(j-1)*cols+i].cost + IndelCost[algnINSERT];
					g.prev=1;
				}
				if (matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE] <g.cost){
					g.cost = matrix[j*cols+(i-1)].cost + IndelCost[algnDELETE];
					g.prev=2;
				}
				matrix[j*cols+i].cost = g.cost;
				matrix[j*cols+i].prev = g.prev;* /
			}
		}
	}
	*/
	return 0;
}


