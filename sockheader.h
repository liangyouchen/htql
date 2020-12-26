#ifndef PORT_DEF_H
#define PORT_DEF_H CLY19991214

#include "log.h"

#ifndef  unix
#include <afx.h>
#include <windows.h>

#include <winsock2.h>

#else
#define  closesocket  	close
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <netdb.h>
#include <errno.h>
//#ifndef IN_ADDR
//#define IN_ADDR
/*
struct in_addr {
        union {
                struct { u_char s_b1,s_b2,s_b3,s_b4; } S_un_b;
                struct { u_short s_w1,s_w2; } S_un_w;
                u_long S_addr;
        } S_un;
}*/
//#endif //IN_ADDR
#endif

#ifndef MAIL_BUF_LEN
#define MAIL_BUF_LEN			256
#endif
#define HOSTLENGTH		100
#define PROTOPORT		5100
#define LOCALHOST		"localhost"

#if (!defined(ERRORLOG) || (ERRORLOG == 0))
#define portOK			0
#define portERR			-1
#define portBAD			-2
#define portINVALIDPORT		-3
#define portINVALIDHOST		-4
#define portPROTOERR		-5
#define portSOCKETERR		-6
#define portCONNECTFAIL		-7
#define portMEMORYFAIL		-8
#define portNOTREADY		-9
#define portPUTERR		-10
#define portBINDFAIL		-12
#define portLISTENFAIL		-13
#define portACCEPTFAIL		-14
#define portFORKFAIL		-15
#define portSELECT			-16

#else
#define portOK			0
#define portERR			(Log::add(ERRORLOGFILE,-1,"port.cpp:Port error",__LINE__))
#define portBAD			(Log::add(ERRORLOGFILE,-2,"port.cpp:Port bad",__LINE__))
#define portINVALIDPORT		(Log::add(ERRORLOGFILE,-3,"port.cpp:Port invalid",__LINE__))
#define portINVALIDHOST		(Log::add(ERRORLOGFILE,-4,"port.cpp:Host error",__LINE__))
#define portPROTOERR		(Log::add(ERRORLOGFILE,-5,"port.cpp:Protocol error",__LINE__))
#define portSOCKETERR		(Log::add(ERRORLOGFILE,-6,"port.cpp:Socket error",__LINE__))
#define portCONNECTFAIL		(Log::add(ERRORLOGFILE,-7,"port.cpp:Connect fail",__LINE__))
#define portMEMORYFAIL		(Log::add(ERRORLOGFILE,-8,"port.cpp:Memory error",__LINE__))
#define portNOTREADY		(Log::add(ERRORLOGFILE,-9,"port.cpp:Not ready",__LINE__))
#define portPUTERR		(Log::add(ERRORLOGFILE,-10,"port.cpp:Put error",__LINE__))
#define portBINDFAIL		(Log::add(ERRORLOGFILE,-12,"port.cpp:Bind fail",__LINE__))
#define portLISTENFAIL		(Log::add(ERRORLOGFILE,-13,"port.cpp:Listen fail",__LINE__))
#define portACCEPTFAIL		(Log::add(ERRORLOGFILE,-14,"port.cpp:Accept fail",__LINE__))
#define portFORKFAIL		(Log::add(ERRORLOGFILE,-15,"port.cpp:fork fail",__LINE__))
#define portSELECT		(Log::add(ERRORLOGFILE,-16,"port.cpp:select fail",__LINE__))

#endif
#define portSETTIME		-11
#define portSTOPPED		-17


#define MY_SIN_FAMILY		AF_INET
//#define MY_SIN_FAMILY		PF_INET

#ifndef unix
	static int wsaErrorCode=0;
	static int wsaStart=0;
	static WSADATA wsaData;
	static WORD wVersionRequested;
#endif


#endif 

