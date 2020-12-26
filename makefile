
CC=gcc 
CXX=g++
LIB=-lm  -lc -lnsl -ldl 

FILE_C=pybrowser.cpp pyhtql.cpp pyhtqlmodule.cpp pyhtqltools.cpp txalign.cpp txdict.cpp txnaivebayes.cpp txregex.cpp docbase.cpp htbrowser.cpp HtNaiveBayes.cpp HtPageModel.cpp HtqlAdapter.cpp htscript.cpp htscriptgen.cpp HtTextAlign.cpp htwrapper.cpp HtWrapperModels.cpp htmlbuf.cpp htmlql.cpp htql.cpp htqlexpr.cpp htqlsyntax.cpp htqlupdate.cpp qhtmlql.cpp qhtql.cpp filedb.cpp fileoper.cpp perlfdb.cpp recfile.cpp txtset.cpp alignment.cpp bindata.cpp btree.cpp clustering.cpp dirfunc.cpp expr.cpp fibheap.cpp freemem.cpp hmmMatrix.cpp htwordlist.cpp links.cpp log.cpp port.cpp profilehmm.cpp qlsyntax.cpp referdata.cpp referdatabase.cpp referlink.cpp referlink2.cpp referlock.cpp referscheduler.cpp referset.cpp refersqlparser.cpp RegExParser.cpp stack.cpp stroper.cpp strstack.cpp tsoper.cpp iwsqlsyntax.cpp IrbNavigationBrowser.cpp HTMLCacheFile.cpp PathNavigationBrowser.cpp PathNavigationBrowserControl.cpp
FILE_H=py2_4.h pybrowser.h pyhtql.h pyhtqlmodule.h pyhtqltools.h StdAfx.h txalign.h txdict.h txnaivebayes.h txregex.h docbase.h htbrowser.h HtNaiveBayes.h HtPageModel.h HtqlAdapter.h htscript.h htscriptgen.h HtTextAlign.h htwrapper.h HtWrapperModels.h htmlbuf.h htmlql.h htql.h htqlexpr.h htqlsyntax.h htqlupdate.h qhtmlql.h qhtql.h filedb.h fileoper.h perlfdb.h recfile.h txtset.h alignment.h bindata.h btree.h clustering.h dirfunc.h evalversion.h expr.h fibheap.h freemem.h heapsortorder.h hmmMatrix.h htwordlist.h links.h log.h platform.h port.h profilehmm.h qlsyntax.h referdata.h referdatabase.h referlink.h referlink2.h referlock.h referscheduler.h referset.h refersqlparser.h RegExParser.h sockheader.h stack.h stroper.h strstack.h tMatrix.h tsoper.h ethoerror.h iwsqlformat.h iwsqlsyntax.h IrbNavigationBrowser.h LaWebDownloadMan.h LaWebLocks.h HTMLCacheFile.h PathNavigationBrowser.h PathNavigationBrowserControl.h
FILE_O=pybrowser.o pyhtql.o pyhtqlmodule.o pyhtqltools.o txalign.o txdict.o txnaivebayes.o txregex.o docbase.o htbrowser.o HtNaiveBayes.o HtPageModel.o HtqlAdapter.o htscript.o htscriptgen.o HtTextAlign.o htwrapper.o HtWrapperModels.o htmlbuf.o htmlql.o htql.o htqlexpr.o htqlsyntax.o htqlupdate.o qhtmlql.o qhtql.o filedb.o fileoper.o perlfdb.o recfile.o txtset.o alignment.o bindata.o btree.o clustering.o dirfunc.o expr.o fibheap.o freemem.o hmmMatrix.o htwordlist.o links.o log.o port.o profilehmm.o qlsyntax.o referdata.o referdatabase.o referlink.o referlink2.o referlock.o referscheduler.o referset.o refersqlparser.o RegExParser.o stack.o stroper.o strstack.o tsoper.o iwsqlsyntax.o IrbNavigationBrowser.o HTMLCacheFile.o PathNavigationBrowser.o PathNavigationBrowserControl.o

.SUFFIXES: .h .cpp .o .a

.cpp.o: 
    $(CC) -c $*.cpp -DPython27 -DNO_WINDOWS -DNOIWEB -DNOIWIRB -DNO_PathTree -DVC6 -I/usr/include/python3.6/

all: $(FILE_O)
    ar -r libPyHTQL26.a $(FILE_O)
    tar cvf libPyHTQL26.tar libPyHTQL26.a $(FILE_H) 
    rm -f libPyHTQL26.tar.gz
    gzip libPyHTQL26.tar

irbrun: 
    gcc -c irbrun.cpp 
    g++ -o irbrun irbrun.o libPyHTQL26.a -lpthread
    # ./irbrun demo.irb /run:path1
    
pyhtql: 
    python setup.py install
    
clean: 
    /bin/rm -f *.o
    /bin/rm -rf build/temp.*
    
#cppfiles=['pybrowser.cpp', 'pyhtql.cpp', 'pyhtqlmodule.cpp', 'pyhtqltools.cpp', 'txalign.cpp', 'txdict.cpp', 'txnaivebayes.cpp', 'txregex.cpp', 'docbase.cpp', 'htbrowser.cpp', 'HtNaiveBayes.cpp', 'HtPageModel.cpp', 'HtqlAdapter.cpp', 'htscript.cpp', 'htscriptgen.cpp', 'HtTextAlign.cpp', 'htwrapper.cpp', 'HtWrapperModels.cpp', 'htmlbuf.cpp', 'htmlql.cpp', 'htql.cpp', 'htqlexpr.cpp', 'htqlsyntax.cpp', 'htqlupdate.cpp', 'qhtmlql.cpp', 'qhtql.cpp', 'filedb.cpp', 'fileoper.cpp', 'perlfdb.cpp', 'recfile.cpp', 'txtset.cpp', 'alignment.cpp', 'bindata.cpp', 'btree.cpp', 'clustering.cpp', 'dirfunc.cpp', 'expr.cpp', 'fibheap.cpp', 'freemem.cpp', 'hmmMatrix.cpp', 'htwordlist.cpp', 'links.cpp', 'log.cpp', 'port.cpp', 'profilehmm.cpp', 'qlsyntax.cpp', 'referdata.cpp', 'referdatabase.cpp', 'referlink.cpp', 'referlink2.cpp', 'referlock.cpp', 'referscheduler.cpp', 'referset.cpp', 'refersqlparser.cpp', 'RegExParser.cpp', 'stack.cpp', 'stroper.cpp', 'strstack.cpp', 'tsoper.cpp', 'iwsqlsyntax.cpp', 'IrbNavigationBrowser.cpp', 'HTMLCacheFile.cpp', 'PathNavigationBrowser.cpp', 'PathNavigationBrowserControl.cpp']
#hfiles=['py2_4.h', 'pybrowser.h', 'pyhtql.h', 'pyhtqlmodule.h', 'pyhtqltools.h', 'StdAfx.h', 'txalign.h', 'txdict.h', 'txnaivebayes.h', 'txregex.h', 'docbase.h', 'htbrowser.h', 'HtNaiveBayes.h', 'HtPageModel.h', 'HtqlAdapter.h', 'htscript.h', 'htscriptgen.h', 'HtTextAlign.h', 'htwrapper.h', 'HtWrapperModels.h', 'htmlbuf.h', 'htmlql.h', 'htql.h', 'htqlexpr.h', 'htqlsyntax.h', 'htqlupdate.h', 'qhtmlql.h', 'qhtql.h', 'filedb.h', 'fileoper.h', 'perlfdb.h', 'recfile.h', 'txtset.h', 'alignment.h', 'bindata.h', 'btree.h', 'clustering.h', 'dirfunc.h', 'evalversion.h', 'expr.h', 'fibheap.h', 'freemem.h', 'heapsortorder.h', 'hmmMatrix.h', 'htwordlist.h', 'links.h', 'log.h', 'platform.h', 'port.h', 'profilehmm.h', 'qlsyntax.h', 'referdata.h', 'referdatabase.h', 'referlink.h', 'referlink2.h', 'referlock.h', 'referscheduler.h', 'referset.h', 'refersqlparser.h', 'RegExParser.h', 'sockheader.h', 'stack.h', 'stroper.h', 'strstack.h', 'tMatrix.h', 'tsoper.h', 'ethoerror.h', 'iwsqlformat.h', 'iwsqlsyntax.h', 'IrbNavigationBrowser.h', 'LaWebDownloadMan.h', 'LaWebLocks.h', 'HTMLCacheFile.h', 'PathNavigationBrowser.h', 'PathNavigationBrowserControl.h']
#ofiles=['pybrowser.o', 'pyhtql.o', 'pyhtqlmodule.o', 'pyhtqltools.o', 'txalign.o', 'txdict.o', 'txnaivebayes.o', 'txregex.o', 'docbase.o', 'htbrowser.o', 'HtNaiveBayes.o', 'HtPageModel.o', 'HtqlAdapter.o', 'htscript.o', 'htscriptgen.o', 'HtTextAlign.o', 'htwrapper.o', 'HtWrapperModels.o', 'htmlbuf.o', 'htmlql.o', 'htql.o', 'htqlexpr.o', 'htqlsyntax.o', 'htqlupdate.o', 'qhtmlql.o', 'qhtql.o', 'filedb.o', 'fileoper.o', 'perlfdb.o', 'recfile.o', 'txtset.o', 'alignment.o', 'bindata.o', 'btree.o', 'clustering.o', 'dirfunc.o', 'expr.o', 'fibheap.o', 'freemem.o', 'hmmMatrix.o', 'htwordlist.o', 'links.o', 'log.o', 'port.o', 'profilehmm.o', 'qlsyntax.o', 'referdata.o', 'referdatabase.o', 'referlink.o', 'referlink2.o', 'referlock.o', 'referscheduler.o', 'referset.o', 'refersqlparser.o', 'RegExParser.o', 'stack.o', 'stroper.o', 'strstack.o', 'tsoper.o', 'iwsqlsyntax.o', 'IrbNavigationBrowser.o', 'HTMLCacheFile.o', 'PathNavigationBrowser.o', 'PathNavigationBrowserControl.o']

