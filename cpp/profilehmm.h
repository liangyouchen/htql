#ifndef PROFILE_HMM_2003_03_12
#define PROFILE_HMM_2003_03_12

#include "hmmMatrix.h"

class HmmTrainingParameters{
public:
	long HmmLen;
	int HmmStateNum;

	HmmMatrix F;
	HmmMatrix B;
	HmmMatrix A1;
	HmmMatrix E1;

	int setHmmModelLength(long length);
	int setHmmModelStates(int statenum);
	int increaseModelLength(long pos);
	int decreaseModelLength(long pos);

	char** AlignedSeqs;
	int AlignedSeqNum;
	double* AlignedSeqsPorb;
	void resetAlignedSeqs();
	int createAlignedSeqsSpace(int seqnum);

	int RunMaxAlignment;
	int RunSumAlignment;
	int CheckEpoches;
	int MinDiffPassMax;
	int RunEpochMax;

	int CurrentRunMaxSum;
	long MaxAlignmentEpoch;
	long SumAlignmentEpoch;

	int CurrentRunEpoch;
	int TotalEpoch;

	int AlignmentDiff;
	int IsModelAdjusted;
	int AdjustModelDiff;
	int IsCurrentRunCompleted;
	int IsTrainingCompleted;

	int CurrentMinDiffPass;
	long DiffCountMin;
	long CurrentDiffCount;

	double T_F;
	double T_B;
	double T_A;
	double T_E;

	double AffinedTransition[3][3]; // ProfileHmm::{M, D, I}

	HmmTrainingParameters();
	~HmmTrainingParameters();
	void reset();

protected:
	int createModel();
};

class ProfileHmm{
public:
	enum {M=0, D, I};
	char* MatchMode;

	// sequences information;
	char** Seqs;	// sequences
	long* SeqLens;	// sequence lengths
	int SeqNum;		// sequences number
	char* States;	// states;
	int StateNum;	// states number;

	//profile parameters;
	long HmmLen;	// hmm model's length -- 2*max(SeqLens)+1 ?
	int HmmStateNum;//hmm models's states number -- StateNum+2 ?
	int IsFixedSize;
	HmmMatrix A;	// *(double*)A(i,j) -- A.setMatrixSize(HmmLen*3, 3);
		// store the -log(Akl)
	HmmMatrix E;	// *(double*)E(i,j) -- E.setMatrixSize(HmmStateNum*3, HmmLen);

	HmmTrainingParameters TrainingPara;

public:
	int dumpA(FILE* f);
	int dumpE(FILE* f);
	int dumpVB(FILE* f, HmmMatrix* V, HmmMatrix* B, long len);
	int dumpDoubleMatrix(FILE* f, HmmMatrix* D, char* description);
	int tologA();
	int tologE();

	int readSequenceFile(char* filename);
		// read sequences from a file; 
		// set values for Seqs, SeqLens, SeqNum, States, StateNum;
		// '-' in file stands for a gap;
	int readSequenceString(char* sequences);
		//sequences in a string
	int calculateProfile();
		// calculate A and E;
	int trainProfile(long passes=0, int initialize_model=true);
		// train A and E by Baum-Welch algorithm;
	double profileAlign(char* seq, long len, char** result_seq, long* result_len, int print_insert=0);
		// profile alignment;
		// result sequence is set in *result_seq, result length in *result_len;
		// '-' in *result_seq represents a gap;
	virtual void onAfterEachEpoch();
	virtual void onAfterCheckEpoches();
	virtual void onAfterInitializeModel();
	virtual void onAfterTrainingComplete();

	int initializeTrainingModel(int hmm_size=0);
	int setFixedSizeTrainingModel(int is_fixed_size);
	int trainInitRun();
	int trainModelOneEpoch();
	int trainModelRun(int max_epoch);
public:
	long X(char x);  //translate a state char to an state index
	double max_double3(double a, double b, double c);
	double log_add3(double a, double b, double c);
	double max_double2(double a, double b);
	double log_add2(double a, double b);
	double log_minus(double a, double b);
	int viterbiCalc(HmmMatrix* V, HmmMatrix* B, long state_j, long x_i, char* seq);
	int forwardAlgorithm(HmmMatrix* F, char* seq, long len, double T=1.0, int sum_or_max=1);
	int backwardAlgorithm(HmmMatrix* B, char* seq, long len, double T=1.0, int sum_or_max=1);
	int traceBackPath(HmmMatrix* F, HmmMatrix* B, char* seq, long len, char** result_seq);
	int addContributionA(HmmMatrix* F, HmmMatrix*B, HmmMatrix* A1, char* seq, long len, double T);
	int addContributionE(HmmMatrix* F, HmmMatrix*B, HmmMatrix* E1, char* seq, long len, double T);
	long recalculateModelPara(HmmMatrix* A1, HmmMatrix* E1);
	int adjustHmmModel(HmmTrainingParameters* para);
	int assign_diff(double* var, double val);
	void printSeqs(FILE* f, char** seqs, int seq_num, int print_prob=0);
public:
	ProfileHmm();
	~ProfileHmm();
	void reset();
};

#endif
