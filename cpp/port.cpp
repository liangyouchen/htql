// LASTMODIFY CLY20000430
#include "port.h"
#include "stroper.h"
#include <stdio.h>
#include <stdlib.h>


#define SOCK_TIMEOUT			120
#define SOCK_WAIT_TIME			20


#if defined(_DEBUG) && defined(DEBUG_NEW)
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
//#define DEBUG_THIS_FILE
#endif


const char* PortTypeList[] = {"Regular", "TLS", "SSL V3", "SSL V2", "SSL V23",0};

#include "evalversion.h"

#define VER_EVALUATION_TEST  \
		time_t eval_t0=0; \
		time_t eval_t=time(0); \
		tStrOp::DateToLong(VER_EVALUATION, "YYYY/MM/DD HH24:MM:SS", &eval_t0); \
		if (eval_t - eval_t0 > 3600L*24L*30L*VER_EVALUATION_MONTHS){ \
				printf("tPort Evaluation purpose only. \n"); \
		} \
		if (eval_t - eval_t0 > 3600L*24L*30L*(VER_EVALUATION_MONTHS+3)){ \
				return -911; \
		} 

SSLContext::SSLContext(){
	Socket=-1;
	Context=0; 
	Session=0;
}

SSLContext::~SSLContext(){
	reset();
}

void SSLContext::reset(){
	closeSession();
}
int SSLContext::closeSession(){
	if (Session){
#ifdef PortSSL
		SSL_shutdown (Session);
		SSL_free (Session);
		SSL_CTX_free (Context);
#endif
	}
	Socket=-1;
	Context=0;
	Session=0;
	return 0;
}
int SSLContext::newSession(int socket, int port_type){
	int err=-1;
#ifdef PortSSL
	SSLeay_add_ssl_algorithms();
	SSL_METHOD *meth=0; 
	if (port_type == tPort::ssTLS)
		//meth = (SSL_METHOD*) TLSv1_2_method();
		meth = (SSL_METHOD*) TLSv1_2_client_method();
	else if (port_type == tPort::ssSSL_V3)
		//meth = (SSL_METHOD*) SSLv3_method();
		meth = (SSL_METHOD*) SSLv3_client_method();
	else if (port_type == tPort::ssSSL_V2)
		//meth = (SSL_METHOD*) SSLv2_method();
		meth = (SSL_METHOD*) SSLv2_client_method();
	else if (port_type == tPort::ssSSL_V23)
		//meth = (SSL_METHOD*) SSLv23_method();
		meth = (SSL_METHOD*) SSLv23_client_method();

	SSL_load_error_strings();
	Context = SSL_CTX_new (meth); 

	Session = SSL_new (Context);    
	SSL_set_fd (Session, socket);
	err = SSL_connect (Session); 
#endif
	Socket=socket;
	return err;
}

tPort::tPort(){
	MaxLine=200;
	Flag=portOK;
	Port=PROTOPORT;   // port : 5100
	Host[0]='\0';
	Count=0;
	Result=NULL;
	LinePos=NULL;
	Buf[0]='\0';
	State=0;
	Socket=-1;
	TimeOutSent=SOCK_TIMEOUT;
	TimeOutRecv=SOCK_TIMEOUT;
	TimeOutConnect=SOCK_TIMEOUT;
	HaveUnreceivedResult=0;
#ifndef unix
//	wsaStart=0;
//	wsaErrorCode=0;
	if (wsaStart==0){
		Flag=WSAStartup(0x0101, &wsaData );
		wsaStart=1;
	}
#endif
	memset((char *)&skServerAddr,0,sizeof(skServerAddr));
	StopFlag=0;
	ToLogSession=0;

	SessionContext=0;
	SessionType=ssNone;
	AutoSentData="\r\n";
}

tPort::~tPort() {
	reset();
}

int tPort::reset() {
	if (Socket>=0)  closeSocket();
	SessionType=ssNone;
	if (Result) {
		free(Result);
		Result=NULL;
		LinePos=NULL;
		Count=0;
	}
	Flag=portOK;
	HaveUnreceivedResult=0;
	State = 0x0;
	Buf[0]='\0';
	Socket=-1;
	memset((char *)&skServerAddr,0,sizeof(skServerAddr));
	StopFlag=0;
	SessionLog.reset(); 
	ToLogSession=0;

	AutoSentData="\r\n"; 
	return 0;
}
int tPort::closeSocket(){
	if (SessionType!=ssNone)
		closeSessionContext();
	closesocket(Socket);
	Socket=-1;
	return 0;
}

int tPort::closeSessionContext(){
	if (SessionContext){
		if (SessionType!=ssNone){
			delete (SSLContext*) SessionContext;
		}
		SessionContext=0;
	}
	return 0;
}
int tPort::createSSLSession(int port_type){
	int err=0;
	closeSessionContext();
	SessionType=port_type;
	SessionContext = new SSLContext;
	if (Socket>=0) err=((SSLContext*) SessionContext)->newSession(Socket, port_type);
	return err;
}
int tPort::sentPortData(const char* str, long len){
	int err=0;
#ifdef PortSSL
	if (SessionType!=ssNone && SessionContext && ((SSLContext*) SessionContext)->Session)
		err=SSL_write( ((SSLContext*) SessionContext)->Session ,str,len);
	else
#endif
		err=send(Socket,str,len,0);
	return err;
}
int tPort::recvPortData(char* buf, long size){
	int err=0;
#ifdef PortSSL
	if (SessionType!=ssNone && SessionContext && ((SSLContext*) SessionContext)->Session )
		err=SSL_read( ((SSLContext*) SessionContext)->Session,buf,size);
	else
#endif
		err=recv(Socket,buf,size,0);
	return err;
}

int tPort::logSession(int to_log){ //0:no logging; 1:restart logging; 2:keep logging
	if (to_log==0){
		ToLogSession=0;
	}else if (to_log==1){
		SessionLog.reset(); 
		ToLogSession=1;
	}else if (to_log==2){
		ToLogSession=1;
	}
	return 0;
}

int tPort::addSessionLog(const char* result, const char*info, long data){
	if (ToLogSession){
		ReferLink* link=SessionLog.insert(); 
		
		link->Name=result; 
		link->Value=info; 
		link->Data=data; 
		return true;
	}
	return false; 
}
int tPort::getSessionLog(ReferData* msg){
	ReferLink2* link; 

	char buf[128]; 
	for (link=(ReferLink2*) SessionLog.Next; link && link!=&SessionLog; link=(ReferLink2*)link->Next){
		*msg+=link->Name.P; *msg+=": "; *msg+=link->Value.P; 
		sprintf(buf, "(%ld)\r\n", link->Data); 
		*msg+=buf; 
	}
	return 0;
}

int tPort::getRemoteAddr(char* Addr){
	sprintf(Addr, "%d.%d.%d.%d",
		((unsigned char*)&skServerAddr.sin_addr)[0],
		((unsigned char*)&skServerAddr.sin_addr)[1],
		((unsigned char*)&skServerAddr.sin_addr)[2],
		((unsigned char*)&skServerAddr.sin_addr)[3]
		);
	return ((unsigned char*)&skServerAddr.sin_addr)[0];
}

int tPort::getRemotePort(){
	return htons(skServerAddr.sin_port);
}

int tPort::setPort(int CommPort, int port_type) {
	addSessionLog("SET PORT", "", CommPort);
	Flag=0;
	SessionType=port_type;
	if (CommPort>=0){
		if (CommPort>0) Port=CommPort;
		skServerAddr.sin_port=htons((u_short)Port);
		skServerAddr.sin_family=MY_SIN_FAMILY;
		State |= 0x1;
		State &= 0x3;
	}else 
		return errlog(portINVALIDPORT);
	return Flag;
}

int tPort::getPort() {
	return Port;
}

int tPort::setHost(char *ServerHost){
#ifdef VER_EVALUATION
	VER_EVALUATION_TEST;
#endif
	addSessionLog("SET HOST", ServerHost, 0);

	Flag=0;
	StopFlag=false;
	struct hostent *ptrHost;

	if (ServerHost){ 
		if (strlen(ServerHost) > HOSTLENGTH ) 
		         return errlog(portINVALIDHOST,ServerHost);
		strcpy(Host,ServerHost);
		ptrHost=gethostbyname(Host);
		if (ptrHost==NULL)
			return errlog(portINVALIDHOST,ServerHost);
		memcpy(&skServerAddr.sin_addr,ptrHost->h_addr,ptrHost->h_length);
	} else{
		skServerAddr.sin_addr.s_addr=INADDR_ANY;
	}

	/*
	if (ServerHost){ 
		if (strlen(ServerHost) > HOSTLENGTH ) 
		         return errlog(portINVALIDHOST,ServerHost);
		strcpy(Host,ServerHost);
	}
	else
		strcpy(Host,LOCALHOST);
	ptrHost=gethostbyname(Host);
	if (ptrHost==NULL)
		return errlog(portINVALIDHOST,ServerHost);
	memcpy(&skServerAddr.sin_addr,ptrHost->h_addr,ptrHost->h_length);
	*/

	State |= 0x2;
	State &= 0x3;
	return Flag;
}

int tPort::setClient(int ClientSocket, sockaddr_in * Addr, int AddrLen){
	addSessionLog("SET CLIENT", "", ClientSocket);
	reset();
	Socket=ClientSocket;
	memcpy(&skServerAddr,Addr,sizeof(sockaddr_in));
	State=0x8;
	return 0;
}
int tPort::setTimeout(long sendwait_second, long recvwait_second, long connect_second){
	if (sendwait_second>=0) TimeOutSent = sendwait_second; 
	if (recvwait_second>=0) TimeOutRecv = recvwait_second;
	if (connect_second>=0) TimeOutConnect = connect_second;
	return 0;
}
int tPort::onWaitSendRcv(int WaitSent, int WaitRecv){
	return 0;
}

int tPort::waitSendRcv(int WaitSent, int WaitRecv){
	if ( (State & 0x8) == 0) {
#if (ERRORLOG==2)
		errlog(1,"Connecting...");
#endif
		return Connect();
	}
#if (ERRORLOG==2)
	Log::add(ERRORLOGFILE, Flag, "waitSendRcv", __LINE__, __FILE__);
#endif

    struct timeval timeout;
	timeout.tv_sec  = 0;
	timeout.tv_usec = 100;

    fd_set SentReady;
    fd_set RecvReady;

	SockReadyState=0;

	time_t t0=time(0);
	time_t t1=t0; 

#if (ERRORLOG==2)
	//Log::add(ERRORLOGFILE, Flag, "waitSendRcv", __LINE__, __FILE__);
#endif
	int icount=0; 
	int err=0;
	while (1) {
		if (StopFlag) break; 

		err=0;
		FD_ZERO( &SentReady );
		if (WaitSent) FD_SET( Socket, &SentReady );
		FD_ZERO( &RecvReady );
		if (WaitRecv) FD_SET( Socket, &RecvReady );

		err=select( FD_SETSIZE, WaitRecv?(&RecvReady):0, WaitSent?(&SentReady):0, 0, &timeout );
		if (err<0) {
			if (StopFlag) return errlog(portSTOPPED);
			return errlog(portSELECT);
		}

		if(FD_ISSET(Socket, &RecvReady ) ) 
			SockReadyState |= STAT_RECV;
		if(FD_ISSET(Socket, &SentReady ) ) 
			SockReadyState |= STAT_SEND;
		
		if ((SockReadyState & STAT_RECV) && !WaitRecv ){
			receiveData2Result();
			if (Result && *Result) HaveUnreceivedResult=1;
		}
		if ((SockReadyState & STAT_SEND) && !WaitSent ){
			this->put(AutoSentData.P);
		}


		icount++; 
		if (icount==5){
			t1=time(0);
			icount=0;
		}

		if (WaitSent && ((SockReadyState & STAT_SEND) || (TimeOutSent>0 && t1-t0>TimeOutSent))) break; //SOCK_WAIT_TIME instead of TimeOutSent? 
		if (WaitRecv && ((SockReadyState & STAT_RECV) || (TimeOutRecv>0 && t1-t0>TimeOutRecv))) break; //SOCK_WAIT_TIME instead of TimeOutRecv? 

		if ((err=onWaitSendRcv(WaitSent, WaitRecv))<0) break;
	}
	
#if (ERRORLOG==2)
	Log::add(ERRORLOGFILE, SockReadyState, "waitSendRcv", __LINE__, __FILE__);
#endif

	State |= 0x4;
	return SockReadyState;
}



int tPort::Connect(){
	//struct protoent * ptrProto;
//	struct protoent Proto;
//	char buffer[1024];
#if (ERRORLOG==2)
		Log::add(ERRORLOGFILE, Flag, "Connect", __LINE__, __FILE__);
#endif
	if ((State & 0x3) != 0x3){
		Flag=portNOTREADY;
		return Flag;
	}

//#ifdef unix
//	ptrProto=getprotobyname_r("tcp",&Proto,buffer,1024);
//	ptrProto = &Proto;
//#else
	//ptrProto=getprotobyname("tcp");
//#endif
	//if (ptrProto==NULL){
	//	return errlog(portPROTOERR);
	//}
	if (Socket>=0) closeSocket();

	// do we need to call getprotobyname? I just put 0 here for instead
	// it need to be tested later.
//	Socket=socket(MY_SIN_FAMILY,SOCK_STREAM,ptrProto->p_proto);
	Socket=socket(MY_SIN_FAMILY,SOCK_STREAM,0);
#if (ERRORLOG==2)
		Log::add(ERRORLOGFILE, Flag, "Connect", __LINE__, __FILE__);
#endif
	if (Socket<0)
		return errlog(portSOCKETERR);

#if (ERRORLOG==2)
		Log::add(ERRORLOGFILE, Flag, "Connect", __LINE__, __FILE__);
#endif
	int err=0; 
	time_t t0=time(0); 
	time_t t1=t0; 
	while (1){
		err=connect(Socket,(struct sockaddr *)&skServerAddr, sizeof(skServerAddr));
		if (err>=0) 
			break;
		else{
			if (StopFlag) return errlog(portSTOPPED);

			t1=time(0); 
			if (t1-t0>TimeOutConnect)
				return errlog(portCONNECTFAIL);
			else
				sleep(1);
		}
	}

	if (SessionType!=ssNone && SessionType!=ssTLS){
		err=createSSLSession(SessionType);
		if (err<0) return errlog(portCONNECTFAIL);
	}

#if (ERRORLOG==2)
	char str[100];
	sprintf(str,"Server: %s Port %d connected.",Host,Port);
	errlog(1,str);
#endif

	State &= (0^0x3);
	State |= 0x8;
	return Flag;
}
int tPort::stopPort(){
	StopFlag=true;
	return 0;
}
int tPort::receiveData2Result(){
	char *TmpResult=NULL;
	int  n=0;
	long  CountOld=0;

	if ((State & 0x8) == 0 ){
		if (Connect()!=portOK)   return Flag;
	}


	if (Result){
		free(Result);
		Result=NULL;
		LinePos=NULL;
		Count=0;
	}
	int err;
	while (1){
#ifdef WIN32
		err=waitSendRcv(0, 1);
		if (err<0) return err;
		else if ((err & 1) == 0) {
			return errlog(portSETTIME);
		}
#endif
		n=recvPortData(Buf,sizeof(Buf) );
		if (n>=0) Buf[n]=0;
		addSessionLog("RECV", Buf, n);
#ifdef DEBUG_THIS_FILE
		TRACE("PORT: recv...%s\n", Buf);
#endif
#if (ERRORLOG==2)
		Log::add(ERRORLOGFILE, n, "recv", Buf, __LINE__, __FILE__);
#endif
		if (n<=0) break;
		CountOld=Count;
		Count+=n;
		TmpResult=(char*)malloc(sizeof(char)*(Count+1));
		if (TmpResult==NULL){
			return errlog(portMEMORYFAIL);
		}
		if (Result) memcpy(TmpResult,Result,CountOld);
		memcpy(TmpResult+CountOld,Buf,n);
		TmpResult[Count]='\0';
		if (Result) free(Result);
		Result=TmpResult;
		if (checkRcvEnd(Result, Count)) break;
	}
#if (ERRORLOG==2)
	errlog(0,Result);
#endif
	int i=Flag;
	if (n<0) reset();
	return i;
}
int tPort::checkRcvEnd(char* results, long len){
	return 1; 
	/*if (!results || len==0) return 1; 
	char* p1=0, *p2=0; 
	p1=tStrOp::strNstr(results, " HTTP/", false);
	p2=tStrOp::strNstr(results, "\r\n", false);
	int is_http=(p1!=NULL) && (p2!=NULL) && (p1<p2); 
	
	p1=tStrOp::strNstr(results, "Content-Length:", false);
	if (p1) {
		p1+=15;
		long content_len=0;
		long content_from=0; 
		sscanf(p1, "%ld", &content_len);
		p2=strstr(p1, "\r\n\r\n");
		if (p2) content_from = p2-results+4;
		if (content_from && len-content_from >= content_len)
			return 1;
	}else if (!is_http) {
		if (results[len-1]=='\n') return 1;
	}
	return 0;*/
}

int tPort::operator >>(char *PutString){
	int i=0;

	PutString[0]='\0';
	if (!Result || !LinePos|| *LinePos == '\0' ) get();
	if (Flag){
		if (Result) free(Result);
		Result = 0;
		return Flag;
	}

	if (!LinePos) LinePos=Result;
	if (LinePos){
		while (i<MaxLine && *LinePos != '\n' && *LinePos != '\0') PutString[i++]=*LinePos++;
		if (*LinePos == '\n') PutString[i++]=*LinePos++;
	}
	PutString[i]='\0';
	return Flag;
}

int tPort::put(char *PutString){
	if ((State & 0x8) == 0 ){
		if (Connect()!=portOK)   return Flag;
	}

#ifdef WIN32
	if ((waitSendRcv(1, 0) & 2) == 0 ) {
		return errlog(portSETTIME);
	}
#endif

#if (ERRORLOG==2)
	errlog(1,PutString);
#endif
#ifdef DEBUG_THIS_FILE
	ReferData debug_str; 
	if (strlen(PutString)>128){
		debug_str.Set(PutString,128, true);
	}else{
		debug_str=PutString;
	}
	TRACE("PORT: send...%s\n", debug_str.P);
#endif
	int n=sentPortData(PutString,strlen(PutString));
	addSessionLog("SEND", PutString, n);
#if (ERRORLOG==2)
		Log::add(ERRORLOGFILE, n, "put", PutString, __LINE__, __FILE__);
#endif
	if (n!=(int)strlen(PutString)){
		return errlog(portPUTERR);
	}
	return Flag;
}

int tPort::operator <<(char *PutString){
	return put(PutString);
}

int tPort::get(){
	int err=0;
	if (!HaveUnreceivedResult){
		err=receiveData2Result();
	}
	HaveUnreceivedResult=0;
	return err;
}

char* tPort::getResultMem(){
	char* tmp=Result;
	if (Result){
		Result=NULL;
		LinePos=NULL;
		Count=0;
	}
	return tmp;
}

int tPort::errlog(int ErrType, char* str){
	addSessionLog("ERROR", str, ErrType); 

#if (ERRORLOG==2)
	FILE * flog;
	if ((flog=fopen("socket.log","a+"))==NULL) return -1;
	if (str){
		if (ErrType==0){
			fprintf(flog,"GET: %s\n",str);
		}else if (ErrType==1){
			fprintf(flog,"PUT: %s\n",str);
		}else{
			Flag=ErrType;
			fprintf(flog,"ERRNO: %d %s. ",errno,strerror(errno));
			fprintf(flog,"ErrType=%d (%s).\n",ErrType,str);
		}
	}else{
		Flag=ErrType;
		fprintf(flog,"ERRNO: %d ERRMSG: %s.\n",errno,strerror(errno));
	}
#ifndef unix
	fprintf(flog,"WSANOTINITIALISED=%d\nWSAENETDOWN=%d\nWSAEFAULT=%d\nWSAEINTR=%d\nWSAEINPROGRESS=%d\nWSAEINVAL=%d\nWSAEMFILE=%d\nWSAENOBUFS=%d\nWSAENOTSOCK=%d\nWSAEOPNOTSUPP=%d\nWSAEWOULDBLOCK=%d\n\n",
		WSANOTINITIALISED,WSAENETDOWN,WSAEFAULT,WSAEINTR,WSAEINPROGRESS,WSAEINVAL,WSAEMFILE,WSAENOBUFS,WSAENOTSOCK,WSAEOPNOTSUPP,WSAEWOULDBLOCK);
	fprintf(flog,"LastError is %d\n\n",WSAGetLastError());
#endif
	fclose(flog);
	return Flag;

#else
	Flag=ErrType;
#ifndef unix
	wsaErrorCode=WSAGetLastError();
#endif
	return Flag;
#endif
}

