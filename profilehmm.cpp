#include "profilehmm.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define MAX_3(a,b,c)	(((a)>=(b) && (a)>=(c))?(a):(((b)>=(c))?(b):(c)))
#define MAX_2(a,b)	(((a)>=(b))?(a):(b))
#define MIN_3(a,b,c)	(((a)<=(b) && (a)<=(c))?(a):(((b)<=(c))?(b):(c)))
#define MIN_2(a,b)	(((a)<=(b))?(a):(b))

#define MAX_SEQ_LENGTH 10000 //ZQ
#define MAX_SEQ_NUM 1000 //ZQ


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif



HmmTrainingParameters::HmmTrainingParameters(){
	HmmLen=0;
	HmmStateNum=0;

	AlignedSeqNum=0;
	AlignedSeqs=0;
	AlignedSeqsPorb=0;

	RunMaxAlignment=1;
	RunSumAlignment=1;
	CheckEpoches=5;
	MinDiffPassMax=30;
	RunEpochMax = 0;

	MaxAlignmentEpoch=0;
	SumAlignmentEpoch=0;
	CurrentRunMaxSum=0;

	CurrentRunEpoch=0;
	TotalEpoch=0;

	AlignmentDiff=0;
	AdjustModelDiff=0;
	IsModelAdjusted=0;
	IsTrainingCompleted=0;
	IsCurrentRunCompleted=0;

	CurrentMinDiffPass=0;
	DiffCountMin=1000000;
	CurrentDiffCount=1000000;

	T_F=1.0;
	T_B=1.0;
	T_A=1.0;
	T_E=1.0;
	for (int i=0; i<3; i++){
		for (int j=0; j<3; j++)
			AffinedTransition[i][j]=0.0;
	}
}
HmmTrainingParameters::~HmmTrainingParameters(){
	reset();
}
void HmmTrainingParameters::reset(){
	HmmLen=0;
	HmmStateNum=0;
	F.reset();
	B.reset();
	A1.reset();
	E1.reset();

	resetAlignedSeqs();

	MaxAlignmentEpoch=0;
	SumAlignmentEpoch=0;
	CurrentRunMaxSum=0;
	RunEpochMax=0;

	CurrentRunEpoch=0;
	TotalEpoch=0;

	AlignmentDiff=0;
	AdjustModelDiff=0;
	IsModelAdjusted=0;
	IsTrainingCompleted=0;
	IsCurrentRunCompleted=0;

	CurrentMinDiffPass=0;
	DiffCountMin=1000000;
	CurrentDiffCount=1000000;

	T_F=1.0;
	T_B=1.0;
	T_A=1.0;
	T_E=1.0;

	for (int i=0; i<3; i++){
		for (int j=0; j<3; j++)
			AffinedTransition[i][j]=0.0;
	}

}

int HmmTrainingParameters::setHmmModelLength(long length){
	HmmLen = length;
	if (HmmStateNum>0){
		return createModel();
	}
	return 1;
}

int HmmTrainingParameters::setHmmModelStates(int statenum){
	HmmStateNum = statenum;
	if (HmmLen>0){
		return createModel();
	}
	return 1;
}

int HmmTrainingParameters::createModel(){
	if (HmmLen<=0 || HmmStateNum<=0) return 1;
	F.setCellSize(sizeof(double));
	F.setMatrixSize((HmmLen+3)*3, HmmLen+2);
	B.setCellSize(sizeof(double));
	B.setMatrixSize((HmmLen+3)*3, HmmLen+2);
	A1.setCellSize(sizeof(double));
	A1.setMatrixSize((HmmLen+2)*3, 3);
	E1.setCellSize(sizeof(double));
	E1.setMatrixSize((HmmLen+2)*3, HmmStateNum+2);
	return 0;
}

int HmmTrainingParameters::increaseModelLength(long pos){
	HmmLen++;
	//printf("Hmm Length increase by 1 = %d\n", HmmLen);
	F.insertCol(pos); F.insertRow((pos)*3);
	F.insertRow((pos)*3); F.insertRow((pos)*3);
	B.insertCol(pos); B.insertRow((pos)*3);
	B.insertRow((pos)*3); B.insertRow((pos)*3);
	A1.insertRow((pos)*3); E1.insertRow((pos)*3);
	A1.insertRow((pos)*3); E1.insertRow((pos)*3);
	A1.insertRow((pos)*3); E1.insertRow((pos)*3);
	return 0;
}
int HmmTrainingParameters::decreaseModelLength(long pos){
	HmmLen--;
	//printf("Hmm Length decrease by 1 = %d\n", HmmLen);
	F.dropCol(pos); F.dropRow(pos*3);
	F.dropRow(pos*3); F.dropRow(pos*3);
	B.dropCol(pos); B.dropRow(pos*3);
	B.dropRow(pos*3); B.dropRow(pos*3);
	A1.dropRow(pos*3); E1.dropRow(pos*3);
	A1.dropRow(pos*3); E1.dropRow(pos*3);
	A1.dropRow(pos*3); E1.dropRow(pos*3);
	return 0;
}

void HmmTrainingParameters::resetAlignedSeqs(){
	if (AlignedSeqs){
		for (int i=0; i<AlignedSeqNum; i++){
			if (AlignedSeqs[i]){
				free(AlignedSeqs[i]);
				AlignedSeqs[i]=0;
			}
		}
		free(AlignedSeqs);
		AlignedSeqs=0;
	}
	if (AlignedSeqsPorb){
		free(AlignedSeqsPorb);
		AlignedSeqsPorb=0;
	}
	AlignedSeqNum=0;
}

int HmmTrainingParameters::createAlignedSeqsSpace(int seqnum){
	resetAlignedSeqs();

	AlignedSeqNum = seqnum;
	AlignedSeqs = (char**) malloc(sizeof(char*)*(AlignedSeqNum));
	memset(AlignedSeqs, 0, sizeof(char*)*(AlignedSeqNum));
	AlignedSeqsPorb = (double*) malloc(sizeof(double)*(AlignedSeqNum));
	memset(AlignedSeqsPorb, 0, sizeof(double)*(AlignedSeqNum));
	return 0;
}


ProfileHmm::ProfileHmm():
	A(sizeof(double)), 
	E(sizeof(double)) 
{
	Seqs=0;
	SeqLens=0;
	SeqNum=0;
	States=0;
	StateNum=0;
	HmmLen=0;
	HmmStateNum=0;
	IsFixedSize=0;
	MatchMode = (char*) malloc(sizeof(char)*4);
	sprintf(MatchMode, "MDI");
	TrainingPara.AffinedTransition[M][M]=0.5;
	TrainingPara.AffinedTransition[M][D]=-0.25;
	TrainingPara.AffinedTransition[M][I]=-0.25;
	TrainingPara.AffinedTransition[D][M]=1.0;
	TrainingPara.AffinedTransition[D][D]=-0.75;
	TrainingPara.AffinedTransition[D][I]=-0.25;
}
ProfileHmm::~ProfileHmm(){
	reset();
	free(MatchMode);
}
void ProfileHmm::reset(){
	if (Seqs){
		for (int i=0; i<SeqNum; i++){
			if (Seqs[i]) free(Seqs[i]);
			Seqs[i]=0;
		}
	}
	if (SeqLens){
		free(SeqLens);
		SeqLens=0;
	}
	if (States){
		free(States);
		States=0;
	}
	StateNum=0;
	SeqNum=0;
	HmmLen=0;
	HmmStateNum=0;
	IsFixedSize=0;
	A.reset();
	E.reset();
	TrainingPara.reset();
}

int ProfileHmm::readSequenceFile(char* filename){
	long len=0;
	char buf[1024];
	char* p=0;
	FILE* f=fopen(filename, "r");
	long i=0;
	while (i=fread(buf, 1, 1024, f)){
		char* p1=(char*) malloc(sizeof(char)*(i+len+1));
		if (p) {
			memcpy(p1, p, len);
			free(p);
		}
		memcpy(p1+len, buf, i);
		p1[len+i]=0;
		p=p1;
		len+=i;
	}
	if (p) {
		readSequenceString(p);
		free(p);
	}
/*	SeqLens = (long*)malloc(sizeof(long)*MAX_SEQ_LENGTH);
    Seqs = (char**)malloc(sizeof(char*)*MAX_SEQ_NUM);
    States = (char*)malloc(sizeof(char)*6);
    
    char* temp = (char*)malloc(sizeof(char)*50);
    
	sprintf(States,"-ACGT");
	StateNum = 5;
	int i=0;
	int j=0;
	char ch;
	size_t len;

	FILE* f;
	f=fopen(filename, "r");
	ch = fgetc(f);
	if(ch!='>') { //simple file
		while(!feof(f)) { 
			Seqs[i]= (char*)malloc(sizeof(char)*MAX_SEQ_LENGTH);
			fgets(Seqs[i],MAX_SEQ_LENGTH,f);
			len = strlen(Seqs[i]);
			if (len && Seqs[i][len-1]=='\n' || Seqs[i][len-1]=='\r') {
				Seqs[i][--len]=0;
			}
			SeqLens[i]=len;
			i++;
		}
		SeqNum = i;
   	} // if simple file

   else { // other file
   	     while(!feof(f)) {
   		        char c=' '; 
   		  		fgets(temp,100, f);
				len = strlen(temp);
				if (len && temp[len-1]=='\n' || temp[len-1]=='\r') {
					temp[--len]=0;
				}
				i = 0;
   				Seqs[j]= (char*)malloc(sizeof(char)*MAX_SEQ_LENGTH);
   				do{
					c = fgetc(f); 
   					if(c!='\n' ) {
						Seqs[j][i] = c;
						i++;
					}
				} while(c!='>' && !feof(f));
 				SeqLens[j]=i-1;
				Seqs[j][i-1]=0;
 				j++;
   			}
   			SeqNum = j;
   	}

	free(temp);
	*/
	/*SeqLens = (long*)malloc(sizeof(long)*MAX_SEQ_LENGTH);
    Seqs = (char**)malloc(sizeof(char*)*MAX_SEQ_NUM);
    States = (char*)malloc(sizeof(char)*6);
	sprintf(States,"-ACGT");
	StateNum = 5;
	int i=0;
	
	ifstream f(filename);

	while(!f.eof()) { 
		Seqs[i]= (char*)malloc(sizeof(char)*MAX_SEQ_LENGTH);
		f.getline(Seqs[i],MAX_SEQ_LENGTH,'\n');
	
		SeqLens[i]=strlen(Seqs[i]);
		i++;
	}
	
	SeqNum = i;*/ // ZQ Mar-25-2003
	
	return 0;
}

int ProfileHmm::readSequenceString(char* sequences){
	StateNum=0;
	int AllocStates=128;
    States = (char*)malloc(sizeof(char)*AllocStates);
	memset(States, 0, sizeof(char)*AllocStates);
	States[StateNum++] = '-';

	SeqNum=0;

	size_t seq_len = strlen(sequences);
	size_t pos=0;
	int format=0;
	char c=sequences[pos];
	if (c=='>'){
		int AllocNum=128;
		Seqs = (char**) malloc(sizeof(char*)*AllocNum);
		memset(Seqs, 0, sizeof(char*)*AllocNum);
		SeqLens = (long*) malloc(sizeof(long)*AllocNum);
		memset(SeqLens, 0, sizeof(long)*AllocNum);

		long AllocSeqSize=0;
		long CurrSeqPos=0;

		int comment=false;
		while (1){
			if (c==0) break;
			if (c == '>') comment = true;
			else if (comment){
				if (c=='\r' || c=='\n' ){
					comment=false;
					AllocSeqSize=0;
					CurrSeqPos=0;
					SeqNum++;
					if (AllocNum < SeqNum){
						char** tmp =(char**) malloc(sizeof(char*)*(AllocNum+128));
						memset(tmp, 0, sizeof(char*)*(AllocNum+128));
						memcpy(tmp, Seqs, sizeof(char*)*AllocNum);
						free( Seqs);
						Seqs = tmp;

						long* tmp1 =(long*) malloc(sizeof(long)*(AllocNum+128));
						memset(tmp1, 0, sizeof(long)*(AllocNum+128));
						memcpy(tmp1, SeqLens, sizeof(long)*AllocNum);
						free( SeqLens);
						SeqLens = tmp1;

						AllocNum+=128;
					}
				}else{
					//this->m_NameAllSeq[SeqNum] += c;
				}
			}else if (!comment && isalpha(c) ) {
				if (CurrSeqPos >= AllocSeqSize-1){
					AllocSeqSize+=512;
					char* temp = (char*) malloc(sizeof(char)*AllocSeqSize);
					if (Seqs[SeqNum-1]){
						memcpy(temp, Seqs[SeqNum-1], CurrSeqPos);
						free(Seqs[SeqNum-1]);
					}
					Seqs[SeqNum-1] = temp;
				}
				Seqs[SeqNum-1][CurrSeqPos++]=c;
				Seqs[SeqNum-1][CurrSeqPos]=0;
				SeqLens[SeqNum-1]=CurrSeqPos;

				if (!strchr(States, c)){
					if (StateNum >= AllocStates){
						char* tmp =(char*) malloc(sizeof(char)*(AllocStates+128));
						memset(tmp, 0, sizeof(char)*(AllocStates+128));
						memcpy(tmp, States, sizeof(char)*AllocStates);
						free( States);
						States = tmp;

						AllocStates += 128;
					}

					States[StateNum++] = c;
				}
			}
			c = sequences[++pos];
			if (pos>=seq_len) break;
		}

	}
	return 0;
}

int ProfileHmm::calculateProfile(){
	int i,j,k;	
	int MATCH[3],DEL[3],INSERT[3];

	HmmLen = SeqLens[0];
	for (i=1; i<SeqNum; i++){
		if (SeqLens[i] > HmmLen) HmmLen = SeqLens[i];
	}

	HmmStateNum = StateNum;

	A.setMatrixSize((HmmLen+2)*3, 3);
	E.setMatrixSize((HmmStateNum+2)*3, HmmLen+2);
	
	int count=3;
	int s;
	for (k=0; k<=HmmLen+1; k++){
		for(s=0;s<3;s++) { 
			MATCH[s] = 1;
			DEL[s] = 1;
			INSERT[s] = 1;
		}
		for (j=0; j<SeqNum; j++){
			if (k>0 && (k>SeqLens[j]  || Seqs[j][k-1]=='-' ) ){
				if (k>=HmmLen){
					if (HmmLen>=SeqLens[j]){
						DEL[M]++;
					}else {
						DEL[I]++;
					}
				}else if (k>SeqLens[j] || Seqs[j][k]=='-') {
					DEL[D]++;
				}else{
					DEL[M]++;
				}
			}else{
				if (k>=HmmLen){
					if (HmmLen>=SeqLens[j]){
						MATCH[M]++;
					}else{
						MATCH[I]++;
					}
				}else if (k>=SeqLens[j] || Seqs[j][k]=='-') {
					MATCH[D]++;
				}else{
					MATCH[M]++;
				}
			}
		}
		count = MATCH[M] + MATCH[D] + MATCH[I];
		for(i=0;i<3;i++) {
			*(double*)A(k*3+M,i) = ((double)MATCH[i])/count;
		}
		count = DEL[M] + DEL[D] + DEL[I];
		for(i=0;i<3;i++) {
			*(double*)A(k*3+D,i) = ((double)DEL[i])/count;
		}
		count = INSERT[M] + INSERT[D] + INSERT[I];
		for(i=0;i<3;i++) {
			*(double*)A(k*3+I,i) = ((double)INSERT[i])/count;
		}
	}
	dumpA(stdout);
	tologA();

	double* SC = (double*)malloc(sizeof(double)*(StateNum+1));
	for(k=0;k<=HmmLen;k++) {
		for (i=0; i<StateNum; i++){
			SC[i]=1;
		}
		count=StateNum;

		if (k<HmmLen){
			for(i=0;i<SeqNum;i++){
				SC[X(Seqs[i][k])] +=1;
				count++;
			}
		}

		for (i=0; i<StateNum; i++){
			*(double*)E(i*3+M,k+1) = ((double)SC[i])/count;
			*(double*)E(i*3+D,k+1) = ((double)1)/StateNum;
			*(double*)E(i*3+I,k+1) = ((double)1)/StateNum;
		}
	}
	free(SC);

	for (i=0; i<StateNum; i++){
		*(double*)E(i*3+M,0) = ((double)1)/StateNum;
		*(double*)E(i*3+D,0) = ((double)1)/StateNum;
		*(double*)E(i*3+I,0) = ((double)1)/StateNum;
	}
	dumpE(stdout);
	tologE();

	return 0;

}

int ProfileHmm::initializeTrainingModel(int hmm_size){
	//calculateProfile();
	if (hmm_size){
		HmmLen=hmm_size;
	}else{
		HmmLen = SeqLens[0];
		for (int i=1; i<SeqNum; i++){
			if (SeqLens[i] > HmmLen) HmmLen = SeqLens[i];
		}
	}
	HmmStateNum = StateNum;
	//HmmLen+=1;
	printf("Hmm Length=%d, States=%d\n", HmmLen, HmmStateNum);
	A.setMatrixSize((HmmLen+2)*3, 3);
	E.setMatrixSize((HmmStateNum+2)*3, HmmLen+2);

	TrainingPara.reset();
	TrainingPara.AffinedTransition[M][M]=0.5;
	TrainingPara.AffinedTransition[M][D]=-0.25;
	TrainingPara.AffinedTransition[M][I]=-0.25;
	TrainingPara.AffinedTransition[D][M]=1.0;
	TrainingPara.AffinedTransition[D][D]=-0.75;
	TrainingPara.AffinedTransition[D][I]=-0.25;
	TrainingPara.setHmmModelLength(HmmLen);
	TrainingPara.setHmmModelStates(HmmStateNum);
	TrainingPara.createAlignedSeqsSpace(SeqNum);
	TrainingPara.RunMaxAlignment = true;
	TrainingPara.RunSumAlignment = true;
	TrainingPara.CurrentRunMaxSum = -1;
	TrainingPara.TotalEpoch=0;
	TrainingPara.IsTrainingCompleted = false;
	return HmmLen;
}

int ProfileHmm::setFixedSizeTrainingModel(int is_fixed_size){
	IsFixedSize = is_fixed_size;
	return 0;
}

int ProfileHmm::trainModelOneEpoch(){
	char* result_seq=0;
	long result_len=0;

	TrainingPara.IsTrainingCompleted = false;
	TrainingPara.IsModelAdjusted = false;

	TrainingPara.A1.clearCells();
	TrainingPara.E1.clearCells();
	int j;
	for (j=0; j<SeqNum; j++){
		result_seq = Seqs[j];
		result_len = SeqLens[j];
		//fprintf(f, "%s\n", result_seq);
		this->forwardAlgorithm(&TrainingPara.F, result_seq, result_len, TrainingPara.T_F,TrainingPara.CurrentRunMaxSum);
		//FILE* f =fopen("dump.txt", "w+");
		//dumpDoubleMatrix(f, &F, "F:");
		this->backwardAlgorithm(&TrainingPara.B, result_seq, result_len, TrainingPara.T_B,TrainingPara.CurrentRunMaxSum);
		//dumpDoubleMatrix(f, &B, "B:");
		//this->traceBackPath(&F, &B, result_seq, result_len, &result_seq);
		this->addContributionA(&TrainingPara.F, &TrainingPara.B, &TrainingPara.A1, result_seq, result_len, TrainingPara.T_A);
		//dumpDoubleMatrix(f, &A1, "A1:");
		this->addContributionE(&TrainingPara.F, &TrainingPara.B, &TrainingPara.E1, result_seq, result_len, TrainingPara.T_E);
		//dumpDoubleMatrix(f, &E1, "E1:");
		//fclose(f);
	}

	TrainingPara.CurrentDiffCount = recalculateModelPara(&TrainingPara.A1, &TrainingPara.E1);

	if (TrainingPara.CurrentDiffCount < TrainingPara.DiffCountMin){
		TrainingPara.DiffCountMin=TrainingPara.CurrentDiffCount;
		TrainingPara.CurrentMinDiffPass=0;
	}else{
		TrainingPara.CurrentMinDiffPass++;
	}

	TrainingPara.CurrentRunEpoch++;
	TrainingPara.TotalEpoch++;

	if (TrainingPara.CurrentRunEpoch % TrainingPara.CheckEpoches ==0 
		|| TrainingPara.CurrentDiffCount==0 
		|| TrainingPara.CurrentMinDiffPass >=TrainingPara.MinDiffPassMax  ){

		TrainingPara.AlignmentDiff=0;
		for (j=0; j<SeqNum; j++){
			double prob=profileAlign(Seqs[j], SeqLens[j], &result_seq, &result_len);
			//printf("%s\n", result_seq);
			if (TrainingPara.AlignedSeqs[j] && strcmp(TrainingPara.AlignedSeqs[j], result_seq)==0){
				free(result_seq);
			}else{
				TrainingPara.AlignmentDiff++;
				if (TrainingPara.AlignedSeqs[j]) free(TrainingPara.AlignedSeqs[j]);
				TrainingPara.AlignedSeqs[j] = result_seq;
				TrainingPara.AlignedSeqsPorb[j] = prob;
			}
		}

		TrainingPara.AdjustModelDiff = adjustHmmModel(&TrainingPara);
		TrainingPara.IsModelAdjusted = true;
		
		if (!TrainingPara.AdjustModelDiff && !TrainingPara.AlignmentDiff ){
			TrainingPara.IsCurrentRunCompleted = true;
		}
		if (TrainingPara.CurrentMinDiffPass >=TrainingPara.MinDiffPassMax) {
			TrainingPara.IsCurrentRunCompleted = true;
		}
		if (!TrainingPara.CurrentDiffCount || (TrainingPara.RunEpochMax &&
			TrainingPara.CurrentRunEpoch>=TrainingPara.RunEpochMax) ){
			TrainingPara.IsCurrentRunCompleted = true;
		}
		if (TrainingPara.IsCurrentRunCompleted){
			if (TrainingPara.CurrentRunMaxSum==0){
				TrainingPara.MaxAlignmentEpoch = TrainingPara.CurrentRunEpoch;
				if (TrainingPara.RunSumAlignment){
					TrainingPara.CurrentRunMaxSum = 1;
				}else{
					TrainingPara.IsTrainingCompleted = true;
				}
			}else if (TrainingPara.CurrentRunMaxSum==1){
				TrainingPara.SumAlignmentEpoch = TrainingPara.CurrentRunEpoch;
				TrainingPara.IsTrainingCompleted = true;
			}else{
				TrainingPara.IsTrainingCompleted = true;
			}
		}

	}

	return TrainingPara.IsTrainingCompleted;
}

int ProfileHmm::trainInitRun(){
	if (TrainingPara.CurrentRunMaxSum == -1){
		if (TrainingPara.RunMaxAlignment)
			TrainingPara.CurrentRunMaxSum = 0;
		else if (TrainingPara.RunSumAlignment){
			TrainingPara.CurrentRunMaxSum = 1;
		}
	}

	TrainingPara.CurrentDiffCount = 1000000;
	TrainingPara.DiffCountMin = 1000000;
	TrainingPara.CurrentMinDiffPass = 0;
	TrainingPara.CurrentRunEpoch = 0;
	TrainingPara.IsCurrentRunCompleted = false;
	return 0;
}

void ProfileHmm::onAfterEachEpoch(){
	printf(" => epoch %d ... %d \n", TrainingPara.CurrentRunEpoch, TrainingPara.CurrentDiffCount);
}
void ProfileHmm::onAfterCheckEpoches(){
	printSeqs(stdout, TrainingPara.AlignedSeqs, TrainingPara.AlignedSeqNum);
}
void ProfileHmm::onAfterInitializeModel(){
	printf("Initial Hmm Length=%d, States=%d\n", HmmLen, HmmStateNum);
}
void ProfileHmm::onAfterTrainingComplete(){
	printf("Final Hmm Length=%d, States=%d\n", HmmLen, HmmStateNum);
}

int ProfileHmm::trainModelRun(int max_epoch){

	this->trainInitRun();
	TrainingPara.RunEpochMax = max_epoch;

	while (!TrainingPara.IsCurrentRunCompleted){

		this->trainModelOneEpoch();

		onAfterEachEpoch();
		if (TrainingPara.IsModelAdjusted){
			onAfterCheckEpoches();
		}
	}
	return TrainingPara.CurrentRunEpoch;
}


int ProfileHmm::trainProfile(long passes, int initialize_model){
	if (initialize_model || HmmLen==0) 
		this->initializeTrainingModel();
	onAfterInitializeModel();

	TrainingPara.IsTrainingCompleted=false;
	TrainingPara.CurrentRunMaxSum = -1;
	while (!TrainingPara.IsTrainingCompleted){//first max, then sum;
		this->trainModelRun(passes);
	}
	onAfterTrainingComplete();

	return 0;
}

int ProfileHmm::adjustHmmModel(HmmTrainingParameters* para){
	int all_delete=0;
	int all_insert=0;
	long HmmLenBak = HmmLen;
	for (long d=0; d<=HmmLenBak; d++){
		int all_space=true;
		for (int j=0; j<SeqNum; j++){
			if (para->AlignedSeqs[j] && para->AlignedSeqs[j][d]!='-') {
				all_space = false;
				break;
			}
		}
		long pos=d-all_delete+all_insert;
		if (all_space){ //(*(double*)A(d*3+M, D)) <=(*(double*)A(d*3+M, M)) && (*(double*)E(M, d+1)) < 0.1 ){
			all_delete++;
			if (pos<HmmLen-1){
				for (long e=pos; e<HmmLen; e++){
					int s1, s2;
					for (s1=0; s1<3; s1++){
						for (s2=0; s2<3; s2++){
							(*(double*)A(e*3+s1, s2)) = (*(double*)A((e+1)*3+s1, s2));
						}
					}
					for (s1=0; s1<3; s1++){
						for (s2=0; s2<HmmStateNum; s2++){
								(*(double*)E(s2*3+s1, e+1)) = (*(double*)E(s2*3+s1, e+2));
						}
					}
				}
			}else{
				HmmLen--;
				printf("Hmm Length decrease by 1 = %d\n", HmmLen);
				A.dropRow(pos*3);
				A.dropRow(pos*3);
				A.dropRow(pos*3);
				E.dropCol(pos+1);
				para->decreaseModelLength(pos);
			}
		}
	//	if ((*(double*)A(pos*3+M, I)) < (*(double*)A(pos*3+M, M)) && (*(double*)A(pos*3+M, I)) <0.5){
		if ((*(double*)A(pos*3+M, M))-(*(double*)A(pos*3+M, I)) >0.3){
			if (++all_insert>all_delete){
				HmmLen++;
				printf("Hmm Length increase by 1 = %d\n", HmmLen);
				A.insertRow((pos+1)*3);
				A.insertRow((pos+1)*3);
				A.insertRow((pos+1)*3);
				E.insertCol(pos+2);
				para->increaseModelLength(pos+1);
				d++;
			}else{
				for (long e=HmmLen; e>pos; e--){
					int s1, s2;
					for (s1=0; s1<3; s1++){
						for (s2=0; s2<3; s2++){
							(*(double*)A(e*3+s1, s2)) = (*(double*)A((e-1)*3+s1, s2));
						}
					}
					for (s1=0; s1<3; s1++){
						for (s2=0; s2<HmmStateNum; s2++){
								(*(double*)E(s2*3+s1, e+1)) = (*(double*)E(s2*3+s1, e));
						}
					}
				}
			}
		}
	}
	return (all_delete || all_insert);
}

void ProfileHmm::printSeqs(FILE* f, char** seqs, int seq_num, int print_prob){
	int* is_end;
	long index;
	is_end = (int*) malloc(sizeof(int)*seq_num);
	memset(is_end,0, sizeof(int)*seq_num);
	for (long pos=0; pos<HmmLen; pos+=50){
		for (int i=0; i<seq_num; i++){
			fprintf(f, "%-10d", i+1);
			for (int j=0; !is_end[i] && j<5; j++){
				for (int k=0; k<10; k++){
					index  = pos+j*10+k;
					if (!is_end[i]){
						if (seqs[i][index]==0){
							is_end[i]=true;
						}
						if (!is_end[i])	fprintf(f, "%c", seqs[i][index]);
					}
				}
				fprintf(f, " ");
			}
			if (print_prob && pos+50>=HmmLen && TrainingPara.AlignedSeqsPorb)	
				fprintf(f, "\t%lf\n", TrainingPara.AlignedSeqsPorb[i]);
			else 
				fprintf(f, "\n");
		}
		fprintf(f, "\n");
	}
	free(is_end);
	return;
}

double ProfileHmm::max_double3(double a, double b, double c){
	return (a>=b && a>=c)?a:((b>=c)?b:c);
}
double ProfileHmm::log_add3(double a, double b, double c){
	double max = MAX_3(-a, -b, -c);
	double d = exp(-a-max) + exp(-b-max) + exp(-c-max);
	return -log(d) - max;
}
double ProfileHmm::max_double2(double a, double b){
	return (a>=b)?a:b;
}
double ProfileHmm::log_add2(double a, double b){
	double max = MAX_2(-a, -b);
	double d = exp(-a-max) + exp(-b-max);
	return -log(d) - max;
}

double ProfileHmm::log_minus(double a, double b){
	double max = MAX_2(-a, -b);
	double d = exp(-a-max) - exp(-b-max);
	return -log(d) - max;
}

int ProfileHmm::forwardAlgorithm(HmmMatrix* F, char* seq, long len, double T, int sum_or_max){
	long i, k;
	F->clearCells();
	for (i=0; i<=HmmLen+1; i++){
		*(double*)(*F)(i*3+M,0)=9000.0;
		*(double*)(*F)(i*3+D,0)=9000.0;
		*(double*)(*F)(i*3+I,0)=9000.0;
	}
	for (i=0; i<=len+1; i++){
		*(double*)(*F)(M,i)=9000.0;
		*(double*)(*F)(D,i)=9000.0;
		*(double*)(*F)(I,i)=9000.0;
	}
	double T1 = T;//(T>1)?(1/T):1;
	double a, b, c;
	*(double*)(*F)(M,0)=0;
	for (i=0; i<=len+1; i++){ //len
		for (k=0; k<=HmmLen+1; k++){
			if (i>0 && k>0){
				a=*(double*)(*F)((k-1)*3+M, i-1)+ (*(double*)A((k-1)*3+M, M));
				b=*(double*)(*F)((k-1)*3+I, i-1)+ (*(double*)A((k-1)*3+I, M));
				c=*(double*)(*F)((k-1)*3+D, i-1)+ (*(double*)A((k-1)*3+D, M));
				if (sum_or_max==0){//max
					*(double*)(*F)(k*3+M, i) = MIN_3(a, b, c);
				}else{//sum
					*(double*)(*F)(k*3+M, i) = log_add3(a, b, c);
				}
				*(double*)(*F)(k*3+M, i) += *(double*)E(X(seq[i-1])*3+M,k)*T1;
			}
			if (i>0){
				a=*(double*)(*F)((k)*3+M, i-1)+ (*(double*)A((k)*3+M, I));
				b=*(double*)(*F)((k)*3+I, i-1)+ (*(double*)A((k)*3+I, I));
				c=*(double*)(*F)((k)*3+D, i-1)+ (*(double*)A((k)*3+D, I));
				if (sum_or_max==0){//max
					*(double*)(*F)(k*3+I, i) = MIN_3(a, b, c);
				}else{//sum
					*(double*)(*F)(k*3+I, i) = log_add3(a,b,c);
				}
				*(double*)(*F)(k*3+I, i) += *(double*)E(X(seq[i-1])*3+I,k)*T1;
			}
			if (k>0){
				a=*(double*)(*F)((k-1)*3+M, i)+ (*(double*)A((k-1)*3+M, D));
				b=*(double*)(*F)((k-1)*3+I, i)+ (*(double*)A((k-1)*3+I, D));
				c=*(double*)(*F)((k-1)*3+D, i)+ (*(double*)A((k-1)*3+D, D));
				if (sum_or_max==0){//max
					*(double*)(*F)(k*3+D, i) = MIN_3(a, b, c);
				}else{//sum
					*(double*)(*F)(k*3+D, i) = log_add3(a,b,c);
				}
			}
		}
	}
//	*(double*)(*F)((HmmLen+1)*3+M, len+1) = log_add3(
//			*(double*)(*F)(HmmLen*3+M, len)+ (*(double*)A(HmmLen*3+M, M)),
//			*(double*)(*F)(HmmLen*3+I, len)+ (*(double*)A(HmmLen*3+I, M)),
//			*(double*)(*F)(HmmLen*3+D, len)+ (*(double*)A(HmmLen*3+D, M)));
	return 0;
}

int ProfileHmm::backwardAlgorithm(HmmMatrix* B, char* seq, long len, double T, int sum_or_max){
	long i, k;
	B->clearCells();
	for (i=0; i<=HmmLen+1; i++){
		*(double*)(*B)(i*3+M,len+1)=9000.0;
		*(double*)(*B)(i*3+D,len+1)=9000.0;
		*(double*)(*B)(i*3+I,len+1)=9000.0;
	}
	for (i=0; i<=len+1; i++){
		*(double*)(*B)((HmmLen+1)*3+M,i)=9000.0;
		*(double*)(*B)((HmmLen+1)*3+D,i)=9000.0;
		*(double*)(*B)((HmmLen+1)*3+I,i)=9000.0;
	}

	double T1 = T;//(T>1)?(1/T):1;
	double a,b,c;
	*(double*)(*B)((HmmLen+1)*3+M, len+1)=0.0; // 1.0
	for (i=len; i>=0; i--){ //len
		for (k=HmmLen; k>=0; k--){
			a=*(double*)(*B)((k+1)*3+M, i+1)+ (*(double*)A((k)*3+M, M)) + ((i<len)?(*(double*)E(X(seq[i])*3+M,k+1)*T1):0);
			b=*(double*)(*B)((k)*3+I, i+1)+ (*(double*)A((k)*3+M, I)) + ((i<len)?(*(double*)E(X(seq[i])*3+I,k)*T1):9000);
			c=*(double*)(*B)((k+1)*3+D, i)+ (*(double*)A((k)*3+M, D));
			if (sum_or_max==0){//max
				*(double*)(*B)(k*3+M, i) = MIN_3(a, b, c);
			}else{//sum
				*(double*)(*B)(k*3+M, i) = log_add3(a,b,c);
			}

			a=*(double*)(*B)((k+1)*3+M, i+1)+ (*(double*)A((k)*3+I, M)) + ((i<len)?(*(double*)E(X(seq[i])*3+M,k+1)*T1):0);
			b=*(double*)(*B)((k)*3+I, i+1)+ (*(double*)A((k)*3+I, I)) + ((i<len)?(*(double*)E(X(seq[i])*3+I,k)*T1):9000);
			c=*(double*)(*B)((k+1)*3+D, i)+ (*(double*)A((k)*3+I, D));
			if (sum_or_max==0){//max
				*(double*)(*B)(k*3+I, i) = MIN_3(a, b, c);
			}else{//sum
				*(double*)(*B)(k*3+I, i) = log_add3(a,b,c);
			}

			a=*(double*)(*B)((k+1)*3+M, i+1)+ (*(double*)A((k)*3+D, M)) + ((i<len)?(*(double*)E(X(seq[i])*3+M,k+1)*T1):0);
			b=*(double*)(*B)((k)*3+I, i+1)+ (*(double*)A((k)*3+D, I)) + ((i<len)?(*(double*)E(X(seq[i])*3+I,k)*T1):9000);
			c=*(double*)(*B)((k+1)*3+D, i)+ (*(double*)A((k)*3+D, D));
			if (sum_or_max==0){//max
				*(double*)(*B)(k*3+D, i) = MIN_3(a, b, c);
			}else{//sum
				*(double*)(*B)(k*3+D, i) = log_add3(a,b,c);
			}
		}
	}
	return 0;
}

int ProfileHmm::traceBackPath(HmmMatrix* F, HmmMatrix* B, char* seq, long len, char** result_seq){
	HmmMatrix FB(sizeof(double));
	FB.setMatrixSize((HmmLen+2)*3, HmmLen+2);
	long i,j,s, s1;
	for (i=0; i<=(HmmLen+1)*3+2; i++){
		for (j=0; j<=HmmLen+1; j++)
			*(double*)FB(i, j)=*(double*)(*F)(i, j)+(*(double*)(*B)(i, j));
	}
	
	//*result_len = HmmLen;
	*result_seq = (char*) malloc(sizeof(char)*(HmmLen+1) );
	long count = HmmLen;
	(*result_seq)[count--]=0;
	i=HmmLen;
	j=len;
	s=0;
	long pos=len-1;
	double min;
	while (i >0 || j>0){
		min=10000.0;
		for (s1=0; s1<3; s1++){
			if (i>0 && j>0){
				if (*(double*)FB((i-1)*3+s1,j-1)<min){
					min=*(double*)FB((i-1)*3+s1,j-1);
					s=M;
				}
			}
			if (i>0){
				if (*(double*)FB((i-1)*3+s1,j)<min){
					min=*(double*)FB((i-1)*3+s1,j);
					s=D;
				}
			}
			if (j>0){
				if (*(double*)FB((i)*3+s1,j-1)<min){
					min=*(double*)FB((i)*3+s1,j-1);
					s=I;
				}
			}
		}
		if (s==M){
			(*result_seq)[count--]=seq[pos--];
			i--;
			j--;
		}else if (s==I){
			pos--;
			j--;
		}else if (s==D){
			(*result_seq)[count--]='-';
			i--;
		}
	}
	dumpDoubleMatrix(stdout, &FB, "FB:");
	printf("%s\n", *result_seq);
	return 0;
}


int ProfileHmm::addContributionA(HmmMatrix* F, HmmMatrix*B, HmmMatrix* A1, char* seq, long len, double T){
	long i, k, s; 
	double P = *(double*)(*F)((HmmLen+1)*3+M, len+1);
	double val;
	double a, max;
	//double T=5.0;
	for (k=0; k<=HmmLen; k++){
		for (s=0; s<3; s++){
			//---1
			max = -10000.0;
			for (i=0; i<len; i++){
				a = *(double*)(*F)(k*3+s, i) + (*(double*)A((k)*3+s, M)) + (*(double*)E(X(seq[i])*3+M,k+1)) + (*(double*)(*B)((k+1)*3+M, i+1));
				if (-a>max) max=-a;
			}
			a = *(double*)(*F)(k*3+s, len) + (*(double*)A((k)*3+s, M));
			if (-a>max) max=-a;
			if (max > -9999.0){
				val=0.0;
				for (i=0; i<len; i++){
					a = *(double*)(*F)(k*3+s, i) + (*(double*)A((k)*3+s, M)) + (*(double*)E(X(seq[i])*3+M,k+1)) + (*(double*)(*B)((k+1)*3+M, i+1));
					val += exp((-a - max)/T);
				}
				a = *(double*)(*F)(k*3+s, len) + (*(double*)A((k)*3+s, M));
				val += exp((-a - max)/T);
				val = -log(val) - max;
				val -= P;
				*(double*)(*A1)(k*3+s, M) = log_add2(*(double*)(*A1)(k*3+s, M), val);
			}

			//---2
			max = -10000.0;
			for (i=0; i<len; i++){
				a = *(double*)(*F)(k*3+s, i) + (*(double*)A((k)*3+s, I)) + (*(double*)E(X(seq[i])*3+I,k)) + (*(double*)(*B)((k)*3+I, i+1));
				if (-a>max) max=-a;
			}
			//a = *(double*)(*F)(k*3+s, len) + (*(double*)A((k)*3+s, I)) ;
			//if (-a>max) max=-a;
			if (max > -9999.0){
				val=0.0;
				for (i=0; i<len; i++){
					a = *(double*)(*F)(k*3+s, i) + (*(double*)A((k)*3+s, I)) + (*(double*)E(X(seq[i])*3+I,k)) + (*(double*)(*B)((k)*3+I, i+1));
					val += exp((-a - max)/T);
				}
				//a = *(double*)(*F)(k*3+s, len) + (*(double*)A((k)*3+s, I)) ;
				//val += exp((-a - max)/T);
				val = -log(val) - max;
				val -= P;
				*(double*)(*A1)(k*3+s, I) = log_add2(*(double*)(*A1)(k*3+s, I), val);
			}

			//---3
			max = -10000.0;
			for (i=0; i<len; i++){
				a = *(double*)(*F)(k*3+s, i) + (*(double*)A((k)*3+s, D)) + (*(double*)E(D,k+1)) + (*(double*)(*B)((k+1)*3+D, i));
				if (-a>max) max=-a;
			}
			a = *(double*)(*F)(k*3+s, len) + (*(double*)A((k)*3+s, D));
			if (-a>max) max=-a;
			if (max > -9999.0){
				val=0.0;
				for (i=0; i<len; i++){
					a = *(double*)(*F)(k*3+s, i) + (*(double*)A((k)*3+s, D)) + (*(double*)E(D,k+1)) + (*(double*)(*B)((k+1)*3+D, i));
					val += exp( (-a - max)/T);
				}
				a = *(double*)(*F)(k*3+s, len) + (*(double*)A((k)*3+s, D));
				val += exp( (-a - max)/T);
				val = -log(val) - max;
				val -= P;
				*(double*)(*A1)(k*3+s, D) = log_add2(*(double*)(*A1)(k*3+s, D), val);
			}

/*			*(double*)(*A1)(k*3+M, M) = log_add2(*(double*)(*A1)(k*3+M, M), - log(4*SeqNum/2));
			*(double*)(*A1)(k*3+M, D) = log_add2(*(double*)(*A1)(k*3+M, D), +log(4*SeqNum));
			*(double*)(*A1)(k*3+M, I) = log_add2(*(double*)(*A1)(k*3+M, I), +log(4*SeqNum));

			*(double*)(*A1)(k*3+D, M) = log_add2(*(double*)(*A1)(k*3+D, M), - log(4*SeqNum/5));
			*(double*)(*A1)(k*3+D, D) = log_add2(*(double*)(*A1)(k*3+D, D), + log(SeqNum));
			*(double*)(*A1)(k*3+D, I) = log_add2(*(double*)(*A1)(k*3+D, I), + log(4*SeqNum));
*/		
		}
	}
	return 0;
}
int ProfileHmm::addContributionE(HmmMatrix* F, HmmMatrix*B, HmmMatrix* E1, char* seq, long len, double T){
	long i, k, s, state_j; 
	double P = *(double*)(*F)((HmmLen+1)*3+M, len+1);
	double val;
	double a, max;
	//double T = 5.0;
	for (k=0; k<=HmmLen; k++){
		for (state_j=0; state_j<HmmStateNum; state_j++){
			for (s=0; s<3; s++){
				max = -10000.0;
				for (i=0; i<len; i++){
					if (seq[i] == States[state_j]){
						a = *(double*)(*F)(k*3+s, i+1) + (*(double*)(*B)((k)*3+s, i+1));
						if (-a>max) max = -a;
					}
				}
				//a = *(double*)(*F)(k*3+M, len+1);
				//if (-a>max) max = -a;
				if (max > -9999.0){
					val=0.0;
					for (i=0; i<len; i++){
						if (seq[i] == States[state_j]){
							a = *(double*)(*F)(k*3+s, i+1) + (*(double*)(*B)((k)*3+s, i+1));
							val += exp((-a - max)*T);
						}
					}
					//a = *(double*)(*F)(k*3+M, len+1);
					//val += exp(-a - max);
					val = -log(val) - max;
					//if (s==D) val = 0;
					val -= P;
					*(double*)(*E1)(k*3+s,state_j) = log_add2(*(double*)(*E1)(k*3+s,state_j), val);
				}
			}
		}
	}
	return 0;
}


int ProfileHmm::assign_diff(double* var, double val){
	double esp=0.0001;
	double d = val - (*var);
	if (d>esp || d<-esp){
		*var = val;
		return 1;
	}
	return 0;
}

long ProfileHmm::recalculateModelPara(HmmMatrix* A1, HmmMatrix* E1){
	long k, s0, t;
	double val;
	double a,b,c, max;
	long diff_count=0;
	for (k=0; k<=HmmLen+1; k++){
		if (k<HmmLen){
			for (int i=0; i<3; i++){
				for (int j=0; j<3; j++){
					if (TrainingPara.AffinedTransition[i][j] >0.0001){
						(*(double*)(*A1)(k*3+i, j)) = log_add2((*(double*)(*A1)(k*3+i, j)), log(TrainingPara.AffinedTransition[i][j]));
					}else if (TrainingPara.AffinedTransition[i][j] <-0.0001){
						(*(double*)(*A1)(k*3+i, j)) = log_minus((*(double*)(*A1)(k*3+i, j)), -log(-TrainingPara.AffinedTransition[i][j]));
					}
				}
			}/*
			(*(double*)(*A1)(k*3+M, M)) = log_add2((*(double*)(*A1)(k*3+M, M)), +log(2.0/4));
			(*(double*)(*A1)(k*3+M, D)) = log_add2((*(double*)(*A1)(k*3+M, D)), -log(1.0/4));
			//(*(double*)(*A1)(k*3+M, I)) = log_add2((*(double*)(*A1)(k*3+M, I)), -log(1.0/4));
			(*(double*)(*A1)(k*3+D, M)) = log_add2((*(double*)(*A1)(k*3+D, M)), +log(4.0/4));
			(*(double*)(*A1)(k*3+D, D)) = log_add2((*(double*)(*A1)(k*3+D, D)), -log(3.0/4));
			//(*(double*)(*A1)(k*3+D, I)) = log_add2((*(double*)(*A1)(k*3+D, I)), -log(1.0/4));
			*/
		}else{
			(*(double*)(*A1)(k*3+M, D)) = log_add2((*(double*)(*A1)(k*3+M, D)), -log(double(1.0/4)));
			(*(double*)(*A1)(k*3+M, I)) = log_add2((*(double*)(*A1)(k*3+M, I)), +log(double(2)));
			(*(double*)(*A1)(k*3+D, D)) = log_add2((*(double*)(*A1)(k*3+D, D)), -log(double(3.0/4)));
			(*(double*)(*A1)(k*3+D, I)) = log_add2((*(double*)(*A1)(k*3+D, I)), +log(double(2)));
		}
		for (s0=0; s0<3; s0++){
			a = (*(double*)(*A1)(k*3+s0, M));
			b = (*(double*)(*A1)(k*3+s0, D));
			c = (*(double*)(*A1)(k*3+s0, I));
			max = MAX_3(-a, -b, -c);
			a = exp(-a-max);
			b = exp(-b-max);
			c = exp(-c-max);

			//if (b>a && b>c) b=(b+MAX_2(a,c))/2;
			//else if (b<a && b<c) b-=(b+MAX_2(-a, -c))/2;

			diff_count += assign_diff( (double*)A(k*3+s0, M), -log(a/(a+b+c)) );
			diff_count += assign_diff( (double*)A(k*3+s0, D), -log(b/(a+b+c)) );
			diff_count += assign_diff( (double*)A(k*3+s0, I), -log(c/(a+b+c)) );
		}
	}

	double nu = log(double(HmmStateNum));
	double* ttt=(double*) malloc(sizeof(double)*(HmmStateNum+1));
	for (k=0; k<=HmmLen+1; k++){
		for (s0=0; s0<3; s0++){
			max = -10000.0;
			for (t=1; t<HmmStateNum; t++){
				a = *(double*)(*E1)(k*3+s0,t);
				if (-a>max) max = -a;
				ttt[t]=a;
			}
			val=0;
			for (t=0; t<HmmStateNum; t++){
				val += exp( - *(double*)(*E1)(k*3+s0,t) - max);
			}
			for (t=0; t<HmmStateNum; t++){
				a = exp( - (*(double*)(*E1)(k*3+s0,t)) - max);
				b = -log(a/val);
				diff_count += assign_diff( (double*)E(t*3+s0,k), (-log(a/val) -nu) );
			}
		}
	}
	free(ttt);
	return diff_count;
}

int ProfileHmm::tologA(){
	long i, s,v;
	for (i=0; i<=HmmLen+1; i++){
		for (s=0; s<3; s++){
			for (v=0; v<3; v++){
				*(double*)A(i*3+s, v) = -log(*(double*)A(i*3+s, v));
			}
		}
	}
	return 0;
}
int ProfileHmm::tologE(){
	long i, s,v;
	double nu = log(double(StateNum));
	//nu=0;
	for (i=0; i<=HmmLen+1; i++){
		for (s=0; s<3; s++){
			for (v=0; v<StateNum; v++){
				*(double*)E(v*3+s,i) = -log(*(double*)E(v*3+s,i)) - nu;
			}
		}
	}
	return 0;
}

int ProfileHmm::dumpA(FILE* f){
	long i, s,v;
	for (i=0; i<=HmmLen+1; i++){
		fprintf(f, "i=%d \n", i);
		for (v=0; v<3; v++){
			fprintf(f, "%6c  ", MatchMode[v]);
		}
		fprintf(f, "\n");
		for (s=0; s<3; s++){
			fprintf(f, "%c: ", MatchMode[s]);
			for (v=0; v<3; v++){
				fprintf(f, "%6.3lf  ", *(double*)A(i*3+s, v));
			}
			fprintf(f, "\n");
		}
	}
	return 0;
}

int ProfileHmm::dumpE(FILE* f){
	long i,v;
	for (i=0; i<=HmmLen+1; i++){
			fprintf(f, "i=%d\n", i);
			for (v=0; v<StateNum; v++){
				fprintf(f, "%5c  ", States[v]);
			}
			fprintf(f, "\n");
			fprintf(f, "M: ");
			for (v=0; v<StateNum; v++){
				fprintf(f, "%5.3lf  ", *(double*)E(v*3+M,i));
			}
			fprintf(f, "\n");
			fprintf(f, "D: ");
			for (v=0; v<StateNum; v++){
				fprintf(f, "%5.3lf  ", *(double*)E(v*3+D,i));
			}
			fprintf(f, "\n");
			fprintf(f, "I: ");
			for (v=0; v<StateNum; v++){
				fprintf(f, "%5.3lf  ", *(double*)E(v*3+I,i));
			}
			fprintf(f, "\n");
	}
	return 0;
}

int ProfileHmm::dumpVB(FILE* f, HmmMatrix* V, HmmMatrix* B, long len){
	long i, s,v;
	for (i=0; i<=HmmLen+1; i++){
		fprintf(f, "i=%d\n", i);
		for (v=0; v<=len+1; v++){
			fprintf(f, "%9d   ", v);
		}
		fprintf(f, "\n");
		for (s=0; s<3; s++){
			fprintf(f, "%c: ", MatchMode[s]);
			for (v=0; v<=len+1; v++){
				fprintf(f, "%8.3lf:%c  ", (*(double*)(*V)(i*3+s, v)), MatchMode[*(long*)(*B)(i*3+s,v)] );
			}
			fprintf(f, "\n");
		}
	}
	return 0;
}

int ProfileHmm::dumpDoubleMatrix(FILE* f, HmmMatrix* D, char* description){
	long i, v;
	long row=D->getRow();
	long col=D->getCol();
	if (description){
		fprintf(f, "%s\n", description);
	}
	for (i=0; i<row; i++){
		if (i%3==0){
			fprintf(f, "i=%d\n", i);
			for (v=0; v<col; v++){
				fprintf(f, "%8d  ", v);
			}
			fprintf(f, "\n");
		}
		for (v=0; v<col; v++){
			fprintf(f, "%8.3lf  ", *(double*)(*D)(i,v));
		}
		fprintf(f, "\n");
	}
	return 0;
}

double ProfileHmm::profileAlign(char* seq, long len, char** result_seq, long* result_len, int print_insert){
	long i, j;
	HmmMatrix V(sizeof(double));
	HmmMatrix S(sizeof(long));
	V.setMatrixSize((HmmLen+2)*3, len+2);
	S.setMatrixSize((HmmLen+2)*3, len+2);
	for (i=0; i<=HmmLen+1; i++){
		*(double*)V(i*3+M,0)=9000.0;
		*(double*)V(i*3+D,0)=9000.0;
		*(double*)V(i*3+I,0)=9000.0;
	}
	for (i=0; i<=len+1; i++){
		*(double*)V(M,i)=9000.0;
		*(double*)V(D,i)=9000.0;
		*(double*)V(I,i)=9000.0;
		double a = *(double*)V(I,i);
	}
	*(double*)V(M,0)=0;
	for (i=0; i<=len+1; i++){
		for (j=0; j<=HmmLen+1; j++){
			viterbiCalc(&V, &S, j, i, seq);
		}
	}

	FILE* outputVB = fopen("dumpVB.txt", "w+");
	dumpVB(outputVB, &V, &S, len);
	fclose(outputVB);
	
	i=len;
	j=HmmLen;
	double prop=*(double*)V((HmmLen+1)*3+M,len+1);
	int s=((long*)S((HmmLen+1)*3+M,len+1))[0];
	if (*(double*)V((HmmLen+1)*3+I,len+1) < *(double*)V((HmmLen+1)*3+M,len+1) ){
		s=((long*)S((HmmLen+1)*3+I,len+1))[0];
		prop=*(double*)V((HmmLen+1)*3+I,len+1);
	}
	int s1;
	long count = 0;
	while (j>0 && i>=0){
		s1 = ((long*)S(j*3+s,i))[0];
		if (s==M){
			if (j<=HmmLen)
				count++;
			i--;
			j--;
		}else if (s==I){
			if (print_insert)
				count++;
			i--;
		}else if (s==D){
			if (j<=HmmLen)
				count ++;
			j--;
		}
		s=s1;
	}
	*result_len = count;
	*result_seq = (char*) malloc(sizeof(char)*(count+1) );
	(*result_seq)[count--]=0;
	i=len;
	j=HmmLen;
	s=((long*)S((HmmLen+1)*3+M,len+1))[0];
	if (*(double*)V((HmmLen+1)*3+I,len+1) < *(double*)V((HmmLen+1)*3+M,len+1) ){
		s=((long*)S((HmmLen+1)*3+I,len+1))[0];
	}

	long pos=len-1;
	while (j>0 && i>=0){
		s1 = ((long*)S(j*3+s,i))[0];
		if (s==M){
			if (j<=HmmLen)
				(*result_seq)[count--]=seq[pos--];
			i--;
			j--;
		}else if (s==I){
			if (print_insert)
				(*result_seq)[count--]='.';
			pos--;
			i--;
		}else if (s==D){
			if (j<=HmmLen)
				(*result_seq)[count--]='-';
			j--;
		}
		s=s1;
	}

	outputVB = fopen("dumpVB.txt", "a");
	fprintf(outputVB, "Alignment: %s\n", *result_seq);
	fclose(outputVB);
	

	return prop; //*(double*)V((HmmLen+1)*3+M,len+1);
}

int ProfileHmm::viterbiCalc(HmmMatrix* V, HmmMatrix* B, long state_j, long x_i, char* seq){
	double min=1000000.0;
	double val=0;
	long s;
	if (state_j>0 && x_i>0){
		*(double*)(*V)(state_j*3+M,x_i) = (state_j<=HmmLen)?(*(double*)E(X(seq[x_i-1])*3+M,state_j)):0;
		for (s=0; s<3; s++){
			val = (*(double*)(*V)((state_j-1)*3+s, x_i-1)) + (*(double*)A((state_j-1)*3+s, M));
			if (val < min){
				((long*)(*B)(state_j*3+M,x_i))[0]=s;
				min = val;
			}
		}
		double d=*(double*)(*V)(state_j*3+M,x_i);
		*(double*)(*V)(state_j*3+M,x_i) += min;
	}

	if (x_i >0){
		*(double*)(*V)(state_j*3+I,x_i) = (*(double*)E(X(seq[x_i-1])*3+I,state_j));
		min=1000000.0;
		for (s=0; s<3; s++){
			val = (*(double*)(*V)(state_j*3+s, x_i-1)) + (*(double*)A(state_j*3+s, I));
			if (val < min){
				((long*)(*B)(state_j*3+I,x_i))[0]=s;
				min = val;
			}
		}
		*(double*)(*V)(state_j*3+I,x_i) += min;
	}

	if (state_j>0){
		*(double*)(*V)(state_j*3+D,x_i) = 0.0;
		min=1000000.0;
		for (s=0; s<3; s++){
			val = (*(double*)(*V)((state_j-1)*3+s, x_i)) + (*(double*)A((state_j-1)*3+s, D));
			if (val < min){
				((long*)(*B)(state_j*3+D,x_i))[0]=s;
				min = val;
			}
		}
		*(double*)(*V)(state_j*3+D,x_i) += min;
	}

	return 0;
}
long ProfileHmm::X(char x){
	char*p = strchr(States, x);
	return p?(p-States):0;
}

