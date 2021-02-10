from distutils.core import setup, Extension

from distutils.sysconfig import get_python_inc

import sys
pyver_id = 'Python32'
if sys.version_info.major < 3: 
    if sys.version_info.major == 2 and sys.version_info.minor >=6: 
        # for python 2.6 and 2.7
        pyver_id = 'Python24'
    else:
        # for python 2.4 and 2.5
        pyver_id = 'Python24'

module1 = Extension('htql',
            define_macros = [('MAJOR_VERSION', '1'),
                             ('MINOR_VERSION', '0'),
                             (pyver_id, 1),
                             ('NO_WINDOWS', 1),
                             ('NOIWEB', 1),
                             ('NOIWIRB', 1),
                             ('NO_PathTree', 1)
                             ],
            include_dirs = [get_python_inc() ],
            libraries = [],
            library_dirs = [],
            sources = ["cpp/"+f for f in ['txnaivebayes.cpp', 'pybrowser.cpp', 'pyhtql.cpp', 'pyhtqlmodule.cpp', 'pyhtqltools.cpp', 'txalign.cpp', 'txdict.cpp', 'txregex.cpp', 'docbase.cpp', 'htbrowser.cpp', 'HtPageModel.cpp', 'HtqlAdapter.cpp', 'htscript.cpp', 'htscriptgen.cpp', 'HtTextAlign.cpp', 'htwrapper.cpp', 'HtWrapperModels.cpp', 'htmlbuf.cpp', 'htmlql.cpp', 'htql.cpp', 'htqlexpr.cpp', 'htqlsyntax.cpp', 'htqlupdate.cpp', 'qhtmlql.cpp', 'qhtql.cpp', 'filedb.cpp', 'fileoper.cpp', 'perlfdb.cpp', 'recfile.cpp', 'txtset.cpp', 'alignment.cpp', 'bindata.cpp', 'btree.cpp', 'clustering.cpp', 'dirfunc.cpp', 'expr.cpp', 'fibheap.cpp', 'freemem.cpp', 'hmmMatrix.cpp', 'htwordlist.cpp', 'links.cpp', 'log.cpp', 'port.cpp', 'profilehmm.cpp', 'qlsyntax.cpp', 'referdata.cpp', 'referdatabase.cpp', 'referlink.cpp', 'referlink2.cpp', 'referlock.cpp', 'referscheduler.cpp', 'referset.cpp', 'refersqlparser.cpp', 'RegExParser.cpp', 'stack.cpp', 'stroper.cpp', 'strstack.cpp', 'tsoper.cpp', 'iwsqlsyntax.cpp', 'IrbNavigationBrowser.cpp', 'HTMLCacheFile.cpp', 'PathNavigationBrowser.cpp', 'PathNavigationBrowserControl.cpp', 'HtNaiveBayes.cpp']]);

setup (name = 'htql',
       version = '1.0',
       description = 'This is a htql package',
       ext_modules = [module1])


