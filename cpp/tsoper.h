#ifndef TS_OPER_H_CLY_2007_01_15
#define TS_OPER_H_CLY_2007_01_15

class tTsOp{
public:
	static double getMean(double* data, long len);
	static double getAbs(double data);
	static double getStd(double* data1, double* data2, long len);
	static double getStd1(double* data1, long len, double mean);
	static double getMin(double* data, long len, long* min_index=0);
	static double getMax(double* data, long len, long* max_index=0);
	static int diff1(double* data, double* result, long len);
	static int diff2(double* data1, double* data2, double* result, long len);
	static int singleSmooth(double* data, long len, double alpha);
	static int median(double* source, double* dest, long len, long lag);
	static int movingAvg(double* source, double*dest, long len, long lag);
	static int bubbleSort(double* data, long* index, long len, int increasing);
	static int sampleAvg(double* source, long len, double sample_rate, double* dest, long* dest_len);
	static int alignSeries(double* peak_times1, long len1, double* peak_times2, long len2, double* cost, long* result_len, double* result_times1, double* result_times2);
};

#endif

