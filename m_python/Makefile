BASE=/home/adam/ircd/inspircd
SWIG=/home/adam/ircd/inspircd/swig/bin/swig

all:
	${SWIG} -python -c++ -external-runtime swig.h
	${SWIG} -importall -ignoremissing -python -c++ -I${BASE}/include inspircd.i
	g++ -g -DSWIG -DSWIG_TYPE_TABLE=inspircd -I${BASE}/include -I /usr/include/python2.6/ -fPIC -shared inspircd_wrap.cxx -o _inspircd.so
	
	g++ -g -DSWIG_TYPE_TABLE=inspircd -o ${BASE}/build/obj/m_python/m_python.o -pipe -fPIC -DPIC -pedantic -Woverloaded-virtual -Wformat=2 -Wmissing-format-attribute -Wall -O2 -I${BASE}/include -I/usr/include/python2.6  -DMODNAME=m_python.so -c m_python.cpp
	g++ -g -DSWIG_TYPE_TABLE=inspircd -o ${BASE}/build/modules/m_python.so -fPIC -shared -rdynamic ${BASE}/build/obj/m_python/m_python.o -lpython2.6
