
/* $CompileFlags: exec("swig -python -c++  -cpperraswarn  -I../include ../src/modules/m_python_swig/inspircd.i") -w pkgconfincludes("python3","/Python.h","") */
/* $LinkerFlags: rpath("pkg-config --libs python3") pkgconflibs("python3","/libpython3.so","-lpython3") */

#define SWIG_TYPE_TABLE "inspircd"
#define SWIG_DIRECTOR_NORTTI

#include "inspircd_wrap.cxx"
