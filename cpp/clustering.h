#ifndef TS_CLUSTERING_H_CLY_2009_06_17
#define TS_CLUSTERING_H_CLY_2009_06_17

#include "tMatrix.h"
#include "btree.h" 

class BTreeItem: public BTree{
public:
	virtual int cmp(char* p1, char* p2); 
}; 

class ClustersList{
public: 
	long* Clusteri; 
	long* Coefi; 
	long Clusteri_len; 
	long Coefi_len; 
	double TP;
	double FP;

	tMatrix<double> Center_coef; 
	double Center_distance; 
	double Min_energy;
	double Match_energy_ratio;

	ClustersList* Next; 
public:
	ClustersList(); 
	~ClustersList(); 
	void reset(); 
};
class Clustering{
public:
	double (*DistanceFun)(tMatrix<double>* w_coef1, long i, tMatrix<double>* w_coef2, long j); //j<0: energy; j>=0: distance
	int (*Merge2)(tMatrix<double>* w_coef1, long i, tMatrix<double>* w_coef2, long j, tMatrix<double>* w_coef3, long k); //merge 2 to 1, including weights, results in i
	int cluster_NN_inc(tMatrix<double>* w_coef, tMatrix<long>* clusters, long* coef_order=0); //w_coef: weight+coef; clusters: output clusters of 2 column, and w_coef.Row-1 rows
	int cluster_NN(tMatrix<double>* w_coef, tMatrix<long>* clusters, tMatrix<double>*sel_distance=0); //w_coef: weight+coef; clusters: output clusters of 2 column, and w_coef.Row-1 rows
	int cluster_NN1(tMatrix<double>* w_coef, tMatrix<long>* clusters); //Old code; w_coef: weight+coef; clusters: output clusters of 2 column, and w_coef.Row-1 rows
	int findConnected(long*ids, long len, tMatrix<long>* clusters, long max_cls_n=-1, long**clsi=0, long* clsi_len=0, long** coefi=0, long* coefi_len=0); 
	int findConnected(long id, tMatrix<long>* clusters, long max_cls_n=-1, long**clsi=0, long* clsi_len=0, long** coefi=0, long* coefi_len=0); 
	int getPlotPositions(tMatrix<long>* clusters, long*clsi, long clsi_len, tMatrix<double>* positions, long* outcomes); 
	int mapIDCountUniquePPV(tMatrix<long>* clusters, long*coefmapid, int*id_class, tMatrix<double>* TP_FP_PPV); 
	int findPPVClusters(tMatrix<long>* clusters, tMatrix<double>* TP_FP_PPV, double min_ppv, ClustersList** subsets);
	int computePPVClustersCenter(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv_low, ClustersList* subsets, ClustersList** centers, int to_refine_di, long*selected_coef_index);
	int searchCoefCluster2Phases(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, ClustersList** centers); 
	int searchCoefClusterInc(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, ClustersList** centers, tMatrix<long>* clusters); 
	int searchCoefClusterSingle(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, ClustersList** centers); 
	int searchCoefClusterSelected(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, tMatrix<double>* distance, int* select, long select_count, ClustersList** centers); 

	static double euclideanDistance(tMatrix<double>* w_coef1, long i, tMatrix<double>* w_coef2, long j); 
	static int merge2WeightedAverages(tMatrix<double>* w_coef1, long i, tMatrix<double>* w_coef2, long j, tMatrix<double>* w_coef3, long k);
	static double absdbl(double x); 
	static double sqr(double x); 
	
	double ticTime(); 
	double tocTime(const char* msg);
	double TicTime;
public:
	Clustering(); 
	~Clustering(); 
	void reset(); 
};

#endif

