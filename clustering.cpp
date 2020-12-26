#include "clustering.h"
#include "referlink2.h"
#include "stdlib.h"
#include "math.h"
#include <sys/timeb.h>


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

int BTreeItem::cmp(char* p1, char* p2){
	if (*(long*)p1 > *(long*)p2) return 1; 
	if (*(long*)p1 < *(long*)p2) return -1; 
	return 0;
}
ClustersList::ClustersList(){
	Clusteri=0; 
	Coefi=0; 
	Clusteri_len=Coefi_len=0; 
	TP=FP=0;
	Next=0; 

	Center_distance=Min_energy=Match_energy_ratio=0;
}
ClustersList::~ClustersList(){
	reset(); 
}
void ClustersList::reset(){
	if (Clusteri) {
		delete [] Clusteri; 
		Clusteri=0; 
	}
	if (Coefi){
		delete [] Coefi; 
		Coefi=0; 
	}
	Clusteri_len=0; 
	Coefi_len=0; 
	if (Next){
		delete Next; 
		Next=0; 
	}
	TP=FP=0;

	Center_coef.reset(); 
	Center_distance=Min_energy=Match_energy_ratio=0;
}

Clustering::Clustering(){
	DistanceFun=euclideanDistance; 
	Merge2=merge2WeightedAverages; 
}

Clustering::~Clustering(){
	reset(); 
}

void Clustering::reset(){
	DistanceFun=euclideanDistance; 
	Merge2=merge2WeightedAverages; 
}
int Clustering::merge2WeightedAverages(tMatrix<double>* w_coef1, long i, tMatrix<double>* w_coef2, long j, tMatrix<double>* w_coef3, long k){
	long M=w_coef1->Cols; 
	long m; 
	double w1=(*w_coef1)(i,0);
	double w2=(*w_coef2)(j,0); 
	double w=w1+w2; 
	for (m=1; m<M; m++){
		(*w_coef3)(k,m)=w1*(*w_coef1)(i,m)+w2*(*w_coef2)(j,m);
		(*w_coef3)(k,m)/=w;
	}
	(*w_coef3)(k,0)=w;
	return 0;
}
double Clustering::euclideanDistance(tMatrix<double>* w_coef1, long i, tMatrix<double>* w_coef2, long j){
	long M=w_coef1->Cols; 
	long m; 
	double dist=0, v=0; 
	if (j>=0){ //regular distance
		for (m=1; m<M; m++){
			v=(*w_coef1)(i, m)-(*w_coef2)(j, m);
			dist+=v*v;
		}
	}else{ //energy
		for (m=1; m<M; m++){
			v=(*w_coef1)(i, m);
			dist+=v*v;
		}
	}
	if (M>1) dist/=(M-1);
	dist=sqrt(dist); 
	return dist; 
}
double Clustering::absdbl(double x){
	return (x>=0)?x:-x; 
}
double Clustering::sqr(double x){
	return x*x;
}

int Clustering::cluster_NN_inc(tMatrix<double>* w_coef, tMatrix<long>* clusters, long* coef_order){ //w_coef: weight+coef; clusters: output clusters of 2 column, and w_coef.Row-1 rows
	//incremental clustering algorithm
	long N=w_coef->Rows; 

	tMatrix<double> coef_clusters; //centers
	coef_clusters.newDim(N*2, w_coef->Cols);
	memcpy(coef_clusters.Cells, w_coef->Cells,sizeof(double)*N*w_coef->Cols); 
	long n=N;

	ReferLink2 link2; //clusters; Name.P(long[]): 0:id1; 1:center1; 2:id2; 3:center2; 4:center3 
	
	ReferLink2* link=link2.insert();
	link->Name.Malloc(sizeof(long)*5);
	long* ids=(long*)link->Name.P;
	ids[0]=ids[1]=coef_order?coef_order[0]:0; ids[2]=ids[3]=coef_order?coef_order[1]:1; ids[4]=n++; 
	(*Merge2)(&coef_clusters,ids[1],&coef_clusters,ids[3],&coef_clusters,ids[4]);

	ReferLink2* last, *last1;
	long i, j;
	for (j=2; j<N; j++){
		i=coef_order?coef_order[j]:j;

		last=link2.Prev;
		//compare the distance of the top two nodes; 
		while (last!=&link2){
			long *ids1=(long*) last->Name.P;
			
			double score12=(*DistanceFun)(&coef_clusters,ids1[1],&coef_clusters,ids1[3]);
			double score1=(*DistanceFun)(&coef_clusters,ids1[1],&coef_clusters,i);
			double score2=(*DistanceFun)(&coef_clusters,ids1[3],&coef_clusters,i);

			if (score1>score12 && score2>score12){ //different from existing clusters, add to the next as a new cluster
				link=((ReferLink2*)last->Next)->insert();
				link->Name.Malloc(sizeof(long)*5);
				ids=(long*)link->Name.P;
				ids[0]=ids1[0]; ids[1]=ids1[4]; ids[2]=i; ids[3]=i; ids[4]=n++; 
				(*Merge2)(&coef_clusters,ids[1],&coef_clusters,ids[3],&coef_clusters,ids[4]);
				break;
			}else if (score1>score2){ //belongs to id2 cluster
				(*Merge2)(&coef_clusters,ids1[4],&coef_clusters,i,&coef_clusters,ids1[4]);
				for (last1=last->Prev; last1!=&link2; last1=last1->Prev){
					long* ids2=(long*) last1->Name.P; 
					if (ids2[0]==ids1[2] || ids2[2]==ids1[2]) break;
				}
				if (last1!=&link2) last=last1; 
				else{
					link=last->insert();
					link->Name.Malloc(sizeof(long)*5);
					ids=(long*)link->Name.P;
					ids[0]=ids1[2]; ids[1]=ids1[3]; ids[2]=i; ids[3]=i; ids[4]=n++; 
					ids1[3]=ids[4];
					(*Merge2)(&coef_clusters,ids[1],&coef_clusters,ids[3],&coef_clusters,ids[4]);
					break;
				}
			}else{ //belongs to id1 cluster
				(*Merge2)(&coef_clusters,ids1[4],&coef_clusters,i,&coef_clusters,ids1[4]);
				for (last1=last->Prev; last1!=&link2; last1=last1->Prev){
					long* ids2=(long*) last1->Name.P; 
					if (ids2[0]==ids1[0] || ids2[2]==ids1[0]) break;
				}
				if (last1!=&link2) last=last1; 
				else{
					link=last->insert();
					link->Name.Malloc(sizeof(long)*5);
					ids=(long*)link->Name.P;
					ids[0]=ids1[0]; ids[1]=ids1[1]; ids[2]=i; ids[3]=i; ids[4]=n++; 
					ids1[1]=ids[4];
					(*Merge2)(&coef_clusters,ids[1],&coef_clusters,ids[3],&coef_clusters,ids[4]);
					break;
				}
			}
		}
	}

	clusters->newDim(n-N, 2); 
	i=0;
	for (link=(ReferLink2*) link2.Next; link && link!=&link2; link=(ReferLink2*) link->Next){
		ids=(long*)link->Name.P;
		(*clusters)(i, 0)=ids[0]; 
		(*clusters)(i, 1)=ids[2];
		i++;
	}

	return 0;
}

int Clustering::cluster_NN(tMatrix<double>* w_coef, tMatrix<long>* clusters, tMatrix<double>*sel_distance){ //w_coef: weight+coef; clusters: output clusters of 2 column, and w_coef.Row-1 rows
	//compute distance
	long N=w_coef->Rows; 

	tMatrix<long> distance; 
	distance.newDim(N,N); //distance, i, j
	BTreeItem searchmin;
	searchmin.NoDuplicate=false; 
	BTreeRecord searchrecord;
	searchrecord.tree=&searchmin;

	//tMatrix<double> distance;
	//distance.newDim(N, N); 
	double multiply=1.0e6;
	long i, j, step; 
	for (i=0; i<N; i++){
		for (j=0; j<i; j++){ //only lower-left triangle
			if (sel_distance){
				distance(i,j)=(long) ((*sel_distance)(i,j)*multiply);
			}else{
				distance(i,j)=(long) ((*DistanceFun)(w_coef, i,w_coef, j)*multiply); 
			}
			searchmin.insert((char*) &distance(i,j)); 
		}
	}
	
	clusters->newDim(N-1, 2); 
	
	//find the nearest neighbor
	int* select=(int*) malloc(sizeof(int)*N);  
	for (i=0; i<N; i++) select[i]=1; 
	
	//stepwise clustering
	for (step=0; step<N-1; step++){ 
		//find the closest matching points
		long* minnode=0; 
		long offset=0, min_i=0, min_j=0;
		while (1){ //find minimum from btree, unless found an unselected node
			minnode=(long*) searchrecord.moveFirst();
			if (!minnode) break; //ASSERT(minnode)
			offset=minnode-distance.Cells;
			min_i=offset/N; min_j=offset%N; 
			if (!select[min_i] || !select[min_j]){ //remove dropped distances
				searchmin.remove((char*) minnode, true);
			}else{
				break;
			}
		}
		long min_dist=*minnode; 
		/*for (i=0; i<N; i++){
			if (!select[i]) continue; 
			for (j=0; j<N; j++){
				if (!select[j] || i==j) continue; 
				
				if (min_dist<0 || min_dist>distance(i,j)){
					min_dist=distance(i,j); 
					min_i=i; min_j=j; 
				}
			}
		}*/
		//ASSERT(min_i>=0 && min_j>=0); 
		(*clusters)(step, 0) = min_i; 
		(*clusters)(step, 1) = min_j; 
		
		//merge the matching points, using weights
		(*Merge2)(w_coef,min_i, w_coef,min_j, w_coef,min_i); 
		
		//update the distance matrix
		select[min_j]=0; 
		for (j=0; j<N; j++){
			if (!select[j] || j==min_i) continue; 
			if (min_i>j){
				searchmin.remove((char*) &distance(min_i,j), true); 
				distance(min_i,j)=(long)((*DistanceFun)(w_coef,min_i,w_coef,j)*multiply); 
				searchmin.insert((char*) &distance(min_i,j)); 
			}else{
				searchmin.remove((char*) &distance(j, min_i), true); 
				distance(j, min_i)=(long)((*DistanceFun)(w_coef,j,w_coef,min_i)*multiply); 
				searchmin.insert((char*) &distance(j, min_i)); 
			}
		}
	}
	
	if (select) free(select); 
	
	return 0;
}
int Clustering::cluster_NN1(tMatrix<double>* w_coef, tMatrix<long>* clusters){ //w_coef: weight+coef; clusters: output clusters of 2 column, and w_coef.Row-1 rows
	//compute distance
	long N=w_coef->Rows; 
	tMatrix<double> distance;
	distance.newDim(N, N); 
	long i, j, step; 
	for (i=0; i<N; i++){
		for (j=0; j<N; j++){
			distance(i,j)=(*DistanceFun)(w_coef,i,w_coef,j); 
		}
	}
	
	clusters->newDim(N-1, 2); 
	
	//find the nearest neighbor
	int* select=(int*) malloc(sizeof(int)*N);  
	for (i=0; i<N; i++) select[i]=1; 
	
	//stepwise clustering
	for (step=0; step<N-1; step++){ 
		//find the closest matching points
		long min_i=-1, min_j=-1; 
		double min_dist=-1; 
		for (i=0; i<N; i++){
			if (!select[i]) continue; 
			for (j=0; j<N; j++){
				if (!select[j] || i==j) continue; 
				
				if (min_dist<0 || min_dist>distance(i,j)){
					min_dist=distance(i,j); 
					min_i=i; min_j=j; 
				}
			}
		}
		//ASSERT(min_i>=0 && min_j>=0); 
		(*clusters)(step, 0) = min_i; 
		(*clusters)(step, 1) = min_j; 
		
		//merge the matching points, using weights
		(*Merge2)(w_coef,min_i, w_coef,min_j, w_coef,min_i); 
		
		//update the distance matrix
		select[min_j]=0; 
		for (j=0; j<N; j++){
			if (!select[j] || j==min_i) continue; 
			distance(min_i,j)=(*DistanceFun)(w_coef,min_i,w_coef,j); 
			distance(j, min_i)=(*DistanceFun)(w_coef,j, w_coef,min_i); 
		}
	}
	
	if (select) free(select); 
	
	return 0;
}
int Clustering::findConnected(long id, tMatrix<long>* clusters, long max_cls_n, long**clsi, long* clsi_len, long** coefi, long* coefi_len){
	return findConnected(&id, 1, clusters, max_cls_n, clsi, clsi_len, coefi, coefi_len);
}
int Clustering::findConnected(long*ids, long len, tMatrix<long>* clusters, long max_cls_n, long**clsi, long* clsi_len, long** coefi, long* coefi_len){
	//find all ids that are connected to each other

	long N=0; 
	
	long i, j, k;
	//get the max max_cls_n
	if (max_cls_n<0){
		for (j=0; j<len; j++){
			for (i=0; i<clusters->Rows; i++){
				if ((*clusters)(i,0)==ids[j] || (*clusters)(i,1)==ids[j]){
					if (i>=max_cls_n) max_cls_n=i+1; 
					break;
				}
			}
		}
	}
	//get the max N
	for (i=0; i<max_cls_n; i++){
		if ((*clusters)(i,0)>=N) N=(*clusters)(i,0)+1; 
		if ((*clusters)(i,1)>=N) N=(*clusters)(i,1)+1; 
	}

	long* new_ids=(long*) malloc(sizeof(long)*N);  
	for (i=0; i<N; i++) new_ids[i]=0; 
	for (k=0; k<len; k++) new_ids[ids[k]]=1; 

	int* new_cls=(int*) malloc(sizeof(int)*max_cls_n); 
	for (i=0; i<max_cls_n; i++) new_cls[i]=0; 
	
	int has_new=true; 
	long step=1; 
	while (has_new){
		has_new=false;
		for (j=0; j<N; j++){
			if (new_ids[j]!=step) continue; 
			for (i=0; i<max_cls_n; i++){ //expand the cluster from each branch_idxs
				if ((*clusters)(i, 0)==j){
					k=(*clusters)(i, 1); 
					if (!new_ids[k]){
						new_ids[k]=step+1;
						has_new=true;
						new_cls[i]=1;
					}
				}
				if ((*clusters)(i, 1)==j){
					k=(*clusters)(i, 0); 
					if (!new_ids[k]){
						new_ids[k]=step+1;
						has_new=true;
						new_cls[i]=1;
					}
				}
			}
		}
		step++;
	}
	
	//copy to coefi
	if (coefi){
		k=0; 
		for (i=0; i<N; i++){
			if (new_ids[i]){
				k++; 
			}
		}
		*coefi_len=k; 
		*coefi = (long*) malloc(sizeof(long)*((*coefi_len)+1));
		k=0;
		for (i=0; i<N; i++){
			if (new_ids[i]){
				(*coefi)[k++]=i; 
			}
		}
		(*coefi)[k++]=-1; 
	}
	if (new_ids) free(new_ids); 

	//copy to clsi
	if (clsi){
		k=0; 
		for (i=0; i<max_cls_n; i++){
			if (new_cls[i]){
				k++; 
			}
		}
		*clsi_len=k; 
		*clsi = (long*) malloc(sizeof(long)*((*clsi_len)+1));
		k=0; 
		for (i=0; i<max_cls_n; i++){
			if (new_cls[i]){
				(*clsi)[k++]=i; 
			}
		}
		(*clsi)[k++]=-1; 
	}
	if (new_cls) free(new_cls); 
	
	return 0;
}
int Clustering::getPlotPositions(tMatrix<long>* clusters, long*clsi, long clsi_len, tMatrix<double>* positions, long* outcomes){
	long* ci=clsi; 
	long i, j, k; 
	if (!ci) {
		ci=(long*) malloc(sizeof(long)*(clusters->Rows));
		for (i=0; i<clusters->Rows; i++) ci[i]=i;
		clsi_len=clusters->Rows; 
	}
	
	long N=0; 
	for (i=0; i<clsi_len; i++){
		if ((*clusters)(ci[i],0)>=N) N=(*clusters)(ci[i],0)+1; 
		if ((*clusters)(ci[i],1)>=N) N=(*clusters)(ci[i],1)+1; 
	}

	tMatrix<double> pos_table; //[x, y, Nclass1, Nclass2]: book-keeping the latest plotting position of data, 
	pos_table.newDim(N, 4);	//it is updated every iternation

	tMatrix<double> fill_lines; //a retancle for filling lines; [x(:), y(:)]
	fill_lines.newDim(clsi_len+1, 2*clsi_len);
	long filled_n=0; 

	long curr_y=1; 
	positions->newDim(clsi_len, 13); //hold all the matching points and positions, (:,[0:4 5:9 10:12]),
							// [match1, x1, y1, n_x1class0, n_x1class1, match2, x2, y2, n_x2class0, n_x2class1, x_next, Nclass0, Nclass1]
	for (k=0; k<clsi_len; k++){
		long match1=(*clusters)(ci[k],0); 
		long match2=(*clusters)(ci[k],1); //match1 matches match2
		long m=0; 
		if (match1>match2) {
			m=match1; match1=match2; match2=m; 
		}

		if (pos_table(match1,0)<0.01){ //initial position
			pos_table(match1,0)=1.0; 
			pos_table(match1,1)=curr_y++; 
			pos_table(match1,2)=(outcomes)?outcomes[match1]==0:0; 
			pos_table(match1,3)=(outcomes)?outcomes[match1]!=0:0; 
		}
		if (pos_table(match2,0)<0.01){ //initial position
			pos_table(match2,0)=1.0; 
			pos_table(match2,1)=curr_y++; 
			pos_table(match2,2)=(outcomes)?outcomes[match1]==0:0; 
			pos_table(match2,3)=(outcomes)?outcomes[match1]!=0:0; 
		}
		//set positions of connecting points: [match1, x1, y1, match2, x2, y2]
		double x_next=(pos_table(match1,0)>pos_table(match2,0))?pos_table(match1,0)+1:pos_table(match2,0)+1; // line that connect the two points [x_next, y_range(:)]
		long y_min=(pos_table(match1,1)<pos_table(match2,1))? (long)pos_table(match1,1):(long)pos_table(match2,1); 
		long y_max=(pos_table(match1,1)>pos_table(match2,1))? (long)(pos_table(match1,1)+0.99):(long)(pos_table(match2,1)+0.99); 
		//find overlapping points
		int overlapping=1; 
		while (overlapping){
			overlapping=0;
			for (i=0; i<filled_n; i++){
				for (j=y_min; j<=y_max; j++) {
					if (absdbl(fill_lines(i,j)-x_next)<0.01) {
						x_next-=0.05; overlapping=1; 
					}
				}
			}
		}
        double Nclass0=pos_table(match1,2)+pos_table(match2,2);
        double Nclass1=pos_table(match1,3)+pos_table(match2,3);
		(*positions)(k, 0)=match1; 
		for (i=0;i<4;i++) (*positions)(k, 1+i)=pos_table(match1, i); 
		(*positions)(k, 5)=match2; 
		for (i=0;i<4;i++) (*positions)(k, 6+i)=pos_table(match2, i); 
		(*positions)(k, 10)=x_next; 
		(*positions)(k, 11)=Nclass0; 
		(*positions)(k, 12)=Nclass1; 

		//change the pos_table
        pos_table(match1,0)=pos_table(match2,0)=x_next;  //set x 1 to the right;
        pos_table(match1,1)=pos_table(match2,1)=(pos_table(match1,1)+pos_table(match2,1))/2; //set y at the middle of the two points
        pos_table(match1,2)=pos_table(match2,2)=Nclass0;
        pos_table(match1,3)=pos_table(match2,3)=Nclass1;

        //fill lines
		for (j=y_min; j<=y_max; j++){ 
			for (i=0; i<filled_n; i++){
				if (fill_lines(i, j)<0.01) 
					break; 
			}
			fill_lines(i, j)=x_next; 
			if (i==filled_n) filled_n++; 
		}
		
	}
	return 0;
}

int Clustering::mapIDCountUniquePPV(tMatrix<long>* clusters, long*coefmapid, int*id_class, tMatrix<double>* TP_FP_PPV){
	long clsi_len=clusters->Rows; 
	long i;

	long N=0; 
	for (i=0; i<clsi_len; i++){
		if ((*clusters)(i,0)>=N) N=(*clusters)(i,0)+1; 
		if ((*clusters)(i,1)>=N) N=(*clusters)(i,1)+1; 
	}

	if (TP_FP_PPV->Cols==0) 
		TP_FP_PPV->newDim(clsi_len, 3);

	BTreeItem* indexer=new BTreeItem[N];
	long match1, match2;
	char* p; 
	for (i=0; i<clsi_len; i++){
		//merge ids from cluster(i,[0 1])
		match1=(*clusters)(i,0); 
		match2=(*clusters)(i,1);
		//merge match2 to match1
		indexer[match1].NoDuplicate = true;
		indexer[match2].NoDuplicate = true;
		if (indexer[match1].total==0) indexer[match1].insert((char*) (coefmapid+match1));
		if (indexer[match2].total==0) indexer[match2].insert((char*) (coefmapid+match2));
		BTreeRecord rec(&indexer[match2]); 
		for (p=rec.moveFirst(); !rec.isEOF(); p=rec.moveNext()){
			indexer[match1].insert(p); 
		}

		//count to TP_FP_PPV
		rec.reset(); 
		rec.tree=&indexer[match1]; 
		long count1=0, count2=0; 
		for (p=rec.moveFirst(); !rec.isEOF(); p=rec.moveNext()){
			if (id_class[*(long*)p] > 0) count2++; 
			else count1++; 
		}
		(*TP_FP_PPV)(i, 0)=count2; 
		(*TP_FP_PPV)(i, 1)=count1; 
		(*TP_FP_PPV)(i, 2)=((double) count2)/(count1+count2); 
	}

	if (indexer) delete[] indexer; 
	return 0;
}

int Clustering::findPPVClusters(tMatrix<long>* clusters, tMatrix<double>* TP_FP_PPV, double min_ppv, ClustersList** subsets){
	long i, k; 

	//find satisfying positions in reversed order
	int* taken_clsi=new int[clusters->Rows]; 
	for (i=0; i<clusters->Rows; i++) taken_clsi[i]=false; 

	long *clsi=0, clsi_len=0, *coefi=0, coefi_len=0; 
	for (k=TP_FP_PPV->Rows-1; k>=0; k--){
		if (!taken_clsi[k] && (*TP_FP_PPV)(k, 2)>=min_ppv){
			findConnected((*clusters)(k, 0), clusters, k+1, &clsi, &clsi_len, &coefi, &coefi_len); 
			if (!clsi) continue; 

			for (i=0; i<clsi_len; i++){
				taken_clsi[clsi[i]]=true; 
			}

			//prune empty sub-branches
			long maxi, left, right, left_i, right_i; 
			maxi=clsi[clsi_len-1]; 
			while (1){
				//clusters->print("clusters_vector.txt", "%ld ");
				//TP_FP_PPV->print("TP_FP_PPV_vector.txt", "%lf ");
				left=(*clusters)(maxi, 0); 
				right=(*clusters)(maxi, 1); 
				for (left_i=maxi-1; left_i>=0; left_i--) {
					if ((*clusters)(left_i, 0)==left || (*clusters)(left_i, 1)==left)
						break;
				}
				for (right_i=maxi-1; right_i>=0; right_i--) {
					if ((*clusters)(right_i, 0)==right || (*clusters)(right_i, 1)==right)
						break;
				}
				if (left_i>=0 ){
					if ((*TP_FP_PPV)(left_i, 0)==0 && right_i>=0){
						maxi=right_i; 
						continue; 
					}else if ((*TP_FP_PPV)(left_i, 0)==(*TP_FP_PPV)(maxi, 0)){
						//right_i may be a leaf node
						maxi=left_i; 
						continue; 
					}
				}
				if (right_i>=0){
					if ((*TP_FP_PPV)(right_i, 0)==0 && left_i>=0){
						maxi=left_i; 
						continue; 
					}else if ((*TP_FP_PPV)(right_i, 0)==(*TP_FP_PPV)(maxi, 0)){
						//left_i may be a leaf node
						maxi=right_i; 
						continue; 
					}
				}
				break;
			}
			if (maxi!=clsi[clsi_len-1]){
				if (clsi) free(clsi); clsi=0; 
				if (coefi) free(coefi); coefi=0; 
				findConnected((*clusters)(maxi, 0), clusters, maxi+1, &clsi, &clsi_len, &coefi, &coefi_len); 
			}

			//return the clsi and coefi
			ClustersList* center=new ClustersList; 
			center->Clusteri=clsi; 
			center->Clusteri_len=clsi_len; 
			center->Coefi=coefi; 
			center->Coefi_len=coefi_len; 
			center->TP=(*TP_FP_PPV)(maxi,0); 
			center->FP=(*TP_FP_PPV)(maxi,1); 
			center->Next=*subsets; 
			*subsets=center; 
			//if (clsi) free(clsi); clsi=0; 
			//if (coefi) free(coefi); coefi=0; 
		}
	}
	if (taken_clsi) delete [] taken_clsi; 
	return 0;
}
double Clustering::ticTime(){
	timeb t;
	ftime(&t); 
	TicTime=t.time+(double) t.millitm/1000; 
	return TicTime;
}

double Clustering::tocTime(const char* msg){
	timeb t;
	ftime(&t); 
	double t1=t.time+(double) t.millitm/1000; 
	double lapse=t1-TicTime;

	TicTime=t1;

	printf("%s: %.3lf seconds\n", msg?msg:"", lapse); 
	return lapse;
}
int Clustering::computePPVClustersCenter(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv_low, ClustersList* subsets, ClustersList** centers, int to_refine_di, long*selected_coef_index){
	//this function will delete subsets
	//find only valid subsets
	ClustersList* list; 
	double maxTP=0; 
	for (list=subsets; list; list=list->Next){
		if (maxTP<list->TP) maxTP=list->TP; 
	}
	long i, j;
	while (subsets){
		//each center
		if (subsets->TP <maxTP/2){ //invalid center, delete it
			list=subsets;
			subsets=list->Next;
			list->Next=0;
			delete list;
		}else{ //valid center
			//translate coef index, copy to a local center_coef;
			tMatrix<double> center_coef; 
			center_coef.newDim(subsets->Coefi_len+1, w_coef->Cols); 
			for (i=0; i<subsets->Coefi_len; i++){
				//translate coef index
				if (selected_coef_index){
					subsets->Coefi[i]=selected_coef_index[subsets->Coefi[i]];
				}

				//copy coef to a local center_coef;
				for (j=0; j<w_coef->Cols; j++){
					center_coef(i+1, j)=(*w_coef)(subsets->Coefi[i], j);
				}
			}

			//computer average center
			subsets->Center_coef.newDim(1, w_coef->Cols); 
			for (j=0; j<w_coef->Cols; j++){
				subsets->Center_coef(0, j)=0;
				for (i=0; i<subsets->Coefi_len; i++){
					subsets->Center_coef(0, j)+=(*w_coef)(subsets->Coefi[i], j);
				}
				subsets->Center_coef(0, j)/=subsets->Coefi_len; 

				//also copy to the local center_coef;
				center_coef(0, j)=subsets->Center_coef(0, j); 
			}

			//compute distance, the maximum cluster distance is too large, need to get the maximum distance based on all coef
			int is_ok=true;
			double max_di=0, di1=0; 
			for (i=0; i<subsets->Coefi_len; i++){
				di1=(*DistanceFun)(&center_coef,i+1, &center_coef,0); 
				if (di1>max_di) max_di=di1; 
			}
			subsets->Center_distance=max_di; 

			if (to_refine_di){
				BTreeItem sort_di, TP, FP;
				sort_di.NoDuplicate=false;
				TP.NoDuplicate=true;
				FP.NoDuplicate=true;
				tMatrix<long> di; 
				di.newDim(w_coef->Rows, 1); 
				for (i=0; i<w_coef->Rows; i++){
					di(i,0)=(long) ( (*DistanceFun)(w_coef,i, &center_coef,0)*1e5 ); 
					sort_di.insert((char*) &di(i,0)); 
				}
				BTreeRecord search_di; 
				search_di.tree=&sort_di;
				long* last_di=0, *curr_di=0; 
				double max_ppv=0, ppv=0;

				max_di*=1e5; 
				for (curr_di=(long*) search_di.moveFirst(); curr_di; curr_di=(long*) search_di.moveNext()){
					i=curr_di-di.Cells; 
					if (id_class[coefmapid[i]]) TP.insert((char*) &coefmapid[i]);
					else FP.insert((char*) &coefmapid[i]);

					ppv=TP.total/((double)(TP.total+FP.total));
					if (ppv>=min_ppv_low) {
						max_ppv=ppv; 
						last_di=curr_di;
					}
					if (*curr_di>max_di && ppv<min_ppv_low) break; 
				}

				if (last_di){
					subsets->Center_distance=(*last_di)/1.0e5; 
				}else{
					is_ok=false;
				}
				if (TP.total<2) is_ok=false;
			}


			//set other parameters, no use now?
			//subsets->Match_energy_ratio=match_energy_ratio; 
			//subsets->Min_energy=min_energy; 
			subsets->Match_energy_ratio=0; 
			subsets->Min_energy=0; 

			//save to output centers
			//sequentially search if there is a duplicated center in the list
			if (is_ok){
				for (list=*centers; list; list=list->Next){
					if (list->TP<1.1) {//too few TP cases
						is_ok=false;
						break;
					}
					if (list->Coefi_len!=subsets->Coefi_len) continue; 
					int same_coefi=true;
					for (i=0; i<subsets->Coefi_len; i++){
						if (subsets->Coefi[i]!=list->Coefi[i]){
							same_coefi=false; 
							break;
						}
					}
					if (same_coefi){
						is_ok=false;
						break;
					}
				}
			}

			list=subsets;
			subsets=list->Next;
			if (is_ok){ //save to output centers
				list->Next=*centers;
				*centers=list;
			}else{ //exist, delete the current one
				list->Next=0; 
				delete list;
			}
		}
	}
	return 0;
}

int Clustering::searchCoefCluster2Phases(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, ClustersList** centers){
	//first, reorder matrix so that positive cases is added first
	long N=w_coef->Rows;

	tMatrix<long> coef_order; 
	coef_order.newDim(N, 1); 
	long i=0, j=0; 
	for (i=0; i<N; i++){
		if (id_class[coefmapid[i]]){
			coef_order(j++, 0)=i; 
		}
	}
	for (i=0; i<N; i++){
		if (!id_class[coefmapid[i]]){
			coef_order(j++, 0)=i; 
		}
	}

	//do clustering for selected w_coef;
	tMatrix<long> clusters;
	cluster_NN_inc(w_coef, &clusters, coef_order.Cells);

	tMatrix<double> TP_FP_PPV;
	mapIDCountUniquePPV(&clusters, coefmapid, id_class, &TP_FP_PPV); 

	ClustersList*subsets=0; 
	//selected_coef_id.print("selected_coef_id.txt", "%ld ");
	findPPVClusters(&clusters, &TP_FP_PPV, min_ppv, &subsets); 

	//find only valid subsets
	ClustersList*centers1=0; 
	computePPVClustersCenter(w_coef, coefmapid, id_class, min_ppv, subsets, &centers1, 0, 0); 
	
	//find patterns belonging to each center, and redo search
	ClustersList** center, *center1; 
	double distance;
	tMatrix<int> select; 
	select.newDim(N,1);
	long select_count=0; 
	for (center=&centers1; *center; ){
		select_count=0; 
		for (i=0; i<N; i++){
			distance=(*DistanceFun)(w_coef,i,&(*center)->Center_coef,0); 
			if (distance<=(*center)->Center_distance){
				select(i,0)=1;
				select_count++;
			}else{
				select(i,0)=0;
			}
		}

		if (select_count>=4000){
			center1=(*center)->Next; 
			subsets=*center;
			subsets->Next=0; 
			*center=center1;

			int to_refine_di=1; 
			computePPVClustersCenter(w_coef, coefmapid, id_class, min_ppv_low, subsets, centers, to_refine_di, 0); 
		}else if (select_count>0){
			searchCoefClusterSelected(w_coef, coefmapid, id_class, min_ppv, min_ppv_low, 0, select.Cells, select_count, centers); 
			center=&(*center)->Next;
		}else {
			center=&(*center)->Next;
		}
	}

	if (centers1) delete centers1; 

	return 0;
}

int Clustering::searchCoefClusterSelected(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, tMatrix<double>* distance, int* select, long select_count, ClustersList** centers){
	long N=w_coef->Rows;
	if (!select) select_count=N;

	//copy selected w_coef;
	tMatrix<double> selected_wcoef; 
	selected_wcoef.newDim(select_count, w_coef->Cols); 
	tMatrix<long> selected_coef_index;
	selected_coef_index.newDim(select_count, 1);
	tMatrix<long> selected_coef_id; 
	selected_coef_id.newDim(select_count, 1);
	tMatrix<double> selected_distance;
	selected_distance.newDim(select_count,select_count);
	long i,j, k=0; 
	for (i=0; i<N; i++){
		if (select && !select[i]) continue; 
		for (j=0; j<w_coef->Cols; j++) 
			selected_wcoef(k, j)=(*w_coef)(i, j); 
		selected_coef_id(k, 0)=coefmapid[i]; 
		selected_coef_index(k, 0)=i; 
		
		long m=0; 
		for (j=0; j<N; j++){
			if (select && !select[j]) continue; 
			if (distance) {
				selected_distance(k, m)=(*distance)(i,j);
			}else{
				selected_distance(k, m)=(*DistanceFun)(w_coef, i, w_coef, j);
			}
			m++;
		}
		k++;
	}
	//tocTime("copy");

	//do clustering for selected w_coef;
	tMatrix<long> clusters; 
	cluster_NN(&selected_wcoef, &clusters, &selected_distance);
	if (clusters.Rows==0) return 0; 

	//tocTime("cluster_NN");

	tMatrix<double> TP_FP_PPV;
	mapIDCountUniquePPV(&clusters, selected_coef_id.Cells, id_class, &TP_FP_PPV); 

	//tocTime("mapIDCountUniquePPV");

	ClustersList*subsets=0; 
	//selected_coef_id.print("selected_coef_id.txt", "%ld ");
	findPPVClusters(&clusters, &TP_FP_PPV, min_ppv, &subsets); 
	//tocTime("findPPVClusters");

	int to_refine_di=1; 
	computePPVClustersCenter(w_coef, coefmapid, id_class, min_ppv_low, subsets, centers, to_refine_di, selected_coef_index.Cells);

	return 0;
}

int Clustering::searchCoefClusterInc(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, ClustersList** centers, tMatrix<long>* clusters){
	//do clustering for selected w_coef;
	cluster_NN_inc(w_coef, clusters);

	tMatrix<double> TP_FP_PPV;
	mapIDCountUniquePPV(clusters, coefmapid, id_class, &TP_FP_PPV); 

	ClustersList*subsets=0; 
	//selected_coef_id.print("selected_coef_id.txt", "%ld ");
	findPPVClusters(clusters, &TP_FP_PPV, min_ppv, &subsets); 

	//find only valid subsets
	int to_refine_di=1; 
	computePPVClustersCenter(w_coef, coefmapid, id_class, min_ppv_low, subsets, centers, to_refine_di, 0); 

	return 0;
}

int Clustering::searchCoefClusterSingle(tMatrix<double>* w_coef, long*coefmapid, int*id_class, double min_ppv, double min_ppv_low, ClustersList** centers){
	//ticTime();
	long N=w_coef->Rows; 
	tMatrix<double> energy; 
	energy.newDim(N,1); 
	
	//compute energy
	long i, j; 
	for (i=0; i<N; i++){
		energy(i, 0)=(*DistanceFun)(w_coef, i, w_coef, -1); 
	}

	//compute distance
	tMatrix<double> distance; 
	distance.newDim(N, N); 
	tMatrix<double> min_distance; 
	min_distance.newDim(N,1);
	for (i=0; i<N; i++){
		double mindist=0; 
		for (j=0; j<N; j++){
			distance(i, j)=(*DistanceFun)(w_coef,i, w_coef,j); 
			if (i!=j && distance(i,j)<mindist) 
				mindist=distance(i,j); 
		}
		min_distance(i, 0)=mindist;
	}

	//set the energy and matching ratio for search
	double e_mean=0; //mean energy
	double e_std=0; //standard deviation of energy
	for (i=0; i<N; i++){
		//e_mean+=log(energy(i,0));
		//e_std+=sqr(log(energy(i,0))); 
		e_mean+=energy(i,0);
		e_std+=sqr(energy(i,0)); 
	}
	e_mean/=N; 
	if (N>1) e_std/=(N-1); 
	e_std=sqrt(e_std);

	//set the energy and matching ratio for search
	double match_energy_ratio=0.5; 
	double e_min=e_mean-e_std; 
	double e_max=e_mean+e_std; 
	long bins=15;

	//tocTime("prepare");

	tMatrix<int> select; 
	select.newDim(N,1);
	long select_count=0; 
	long maxzero=(w_coef->Cols-1)/3; 
	long countzero=0;
	//grid search for valid subsets
	for (double min_log_energy=e_min; min_log_energy<e_max; min_log_energy+=e_std/bins){
		//double min_energy=exp(min_log_energy); 
		double min_energy=min_log_energy; 
		if (min_energy<=0) continue; 

		//count selected 
		select_count=0; 
		for (i=0; i<N; i++) {
			countzero=0; 
			for (j=1; j<w_coef->Cols; j++) {
				if ((*w_coef)(i, j)<1e-10 && (*w_coef)(i, j)>-1e-10) countzero++; 
			}
			if (energy(i,0)>=min_energy && countzero<=maxzero 
				&& min_distance(i,0)<min_energy*match_energy_ratio){
				select(i,0)=1;
				select_count++;
			}else{
				select(i,0)=0;
			}
		}
		if (select_count==0) continue; 

		searchCoefClusterSelected(w_coef, coefmapid, id_class, min_ppv, min_ppv_low, &distance, select.Cells, select_count, centers); 

		ClustersList* list; 
		for (list=*centers; list; list=list->Next){
			list->Match_energy_ratio=match_energy_ratio; 
			list->Min_energy=min_energy; 
		}
	}
	return 0;
}



