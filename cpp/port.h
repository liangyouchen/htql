#ifndef PORT_H
#define PORT_H		CLY20000430

#include "log.h"
#include "sockheader.h"
#include "referlink2.h"


#ifdef UseExternalDLL
	#define PortSSL
#endif


extern const char* PortTypeList[];

#ifdef PortSSL
	#include "openssl/ssl.h"
#else
	typedef int SSL;
	typedef int SSL_CTX;
#endif

class SSLContext{
public:
	int Socket;
	SSL_CTX* Context;
	SSL* Session;
	int newSession(int socket, int port_type);
	int closeSession(); 
	
	SSLContext();
	~SSLContext(); 
	void reset(); 
}; 

class tPort{
public:
	enum {STAT_NONE=0, STAT_RECV=1, STAT_SEND=2, STAT_BOTH=3};
	int Flag;
	int SockReadyState;
	int MaxLine; //Max length for operator>>
	int Port;       //=portOK,portERR,....
	char Host[HOSTLENGTH];
	long Count;
	char *Result;

	int SessionType; 
	enum {ssNone, ssSSL=1, ssTLS=1, ssSSL_V3, ssSSL_V2, ssSSL_V23};
	ReferData AutoSentData; 

protected:
	int StopFlag;
	char *LinePos;
	int  State;  	//bit: 0x1-set port, 0x2-set host, 0x4-set time, 
			//     0x8-connect, 
	int  Socket;
	char Buf[MAIL_BUF_LEN];
	struct sockaddr_in skServerAddr;
	long TimeOutSent;
	long TimeOutRecv;
	long TimeOutConnect;
	int closeSocket();
	int sentPortData(const char* str, long len);
	int recvPortData(char* buf, long size);

	void* SessionContext; 
	int createSSLSession(int port_type);
	int closeSessionContext(); 

	int HaveUnreceivedResult;
	int receiveData2Result();
public:
	tPort();
	virtual ~tPort();

	int getRemoteAddr(char* Addr);
	int getRemotePort();
	int reset();
	int setPort(int CommPort=0, int port_type=ssNone);       //set server port; return value=Flag;
	int getPort();
	int setHost(char *ServerHost=NULL);//set server name; return value=Flag;
	virtual int setTimeout(long sendwait_second, long recvwait_second, long connect_second=-1);
	virtual int waitSendRcv(int WaitSent, int WaitRecv);
	virtual int onWaitSendRcv(int WaitSent, int WaitRecv); 
	virtual int checkRcvEnd(char* results, long len);
	virtual int stopPort();
	int setClient(int ClientSocket, sockaddr_in * Addr, int AddrLen);

	int put(char *PutString); //send a string ended with '\0'; 
				  //return value=Flag;
	int get();                //get a string sent by server; 
				  //return value=Flag;
	char* getResultMem(); // result memory is to be freed by caller.

	ReferLink2 SessionLog;
	int ToLogSession; 
	int logSession(int to_log=1); //0:no logging; 1:restart logging; 2:keep logging
	int addSessionLog(const char* result, const char*info, long data);
	int getSessionLog(ReferData* msg); 

	int operator >>(char *PutString); //get a line ended with '\n';
				          //return value=Flag;
	int operator <<(char *PutString); //same as put();
				          //return value=Flag;

	int Connect();
protected:
	int errlog(int ErrType, char*str=NULL);
};

#endif
