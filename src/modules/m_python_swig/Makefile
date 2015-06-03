BASE=/home/adam/ircd/inspircd
SWIG=swig

all:
	#${SWIG} -python -c++ -external-runtime swig.h
	${SWIG} -python -c++  -cpperraswarn  -I${BASE}/include inspircd.i
	g++ -g -DINSPIRCD_SWIG -DSWIG_TYPE_TABLE=inspircd -I${BASE}/include -I /usr/include/python2.7/ -fPIC -shared inspircd_wrap.cxx -o _inspircd.so
	touch wrapper.cpp
	#g++ -g -DSWIG_TYPE_TABLE=inspircd -o ${BASE}/build/obj/m_python/m_python.o -pipe -fPIC -DPIC -pedantic -Woverloaded-virtual -Wformat=2 -Wmissing-format-attribute -Wall -O2 -I${BASE}/include -I/usr/include/python2.7  -DMODNAME=m_python.so -c m_python.cpp
	#g++ -g -o ${BASE}/build/modules/m_python.so -fPIC -shared -rdynamic ${BASE}/build/obj/m_python/m_python.o -lpython2.7
