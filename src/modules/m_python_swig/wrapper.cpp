
/* $CompileFlags: exec("swig -python -c++  -cpperraswarn  -I../include ../src/modules/m_python_swig/inspircd.i") -w -frtti -fvisibility=default pkgconfincludes("python","/Python.h","") */
/* $LinkerFlags: rpath("pkg-config --libs python") pkgconflibs("python","/libpython2.7.so","-lpython2.7") */

#define SWIG_TYPE_TABLE "inspircd"

#include "inspircd_wrap.cxx"
