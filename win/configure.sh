#!/bin/bash

LIBS_TAG=2014-7-24
BASE=`pwd`
INSPIRCD_BASE=$BASE/../
LIBS_BASE=$BASE/windows-libs

# Unpack extra libraries
if [ ! -f windows-libs.tar.gz ] ; then
	wegt https://github.com/Adam-/windows-scripts/releases/download/$LIBS_TAG/windows-libs.tar.gz
fi
rm -rf windows-libs
mkdir windows-libs
tar zxf windows-libs.tar.gz -C windows-libs

# Enable extra modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_geoip.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_ldapauth.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_ldapoper.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_mysql.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_regex_pcre.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_sqlite3.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_ssl_gnutls.cpp $INSPIRCD_BASE/src/modules
ln -s $INSPIRCD_BASE/src/modules/extra/m_ssl_openssl.cpp $INSPIRCD_BASE/src/modules

# Copy required DLLs
cp $LIBS_BASE/gmp/lib/libgmp-10.dll $BASE
cp $LIBS_BASE/nettle/bin/libhogweed-2-5.dll $BASE
cp $LIBS_BASE/nettle/bin/libnettle-4-7.dll $BASE
cp $LIBS_BASE/mysql/lib/libmysql.dll $BASE
cp $LIBS_BASE/pcre/bin/libpcre-1.dll $BASE
cp $LIBS_BASE/sqlite/bin/libsqlite3-0.dll $BASE
cp $LIBS_BASE/openldap/bin/liblber.dll $BASE
cp $LIBS_BASE/openldap/bin/libldap.dll $BASE
cp $LIBS_BASE/openssl/bin/libeay32.dll $BASE
cp $LIBS_BASE/openssl/bin/ssleay32.dll $BASE
cp $LIBS_BASE/gnutls/bin/libgnutls-28.dll $BASE

cd build
rm -rf *
cmake .. -DTOOLCHAIN_PREFIX=i686-w64-mingw32  -DCMAKE_TOOLCHAIN_FILE=../Toolchan-mingw.cmake -DEXTRA_INCLUDES="$LIBS_BASE/geoip/include/;$LIBS_BASE/gnutls/include/;$LIBS_BASE/mysql/include/;$LIBS_BASE/openldap/include/;$LIBS_BASE/openssl/include/;$LIBS_BASE/pcre/include/;$LIBS_BASE/sqlite/include/;$LIBS_BASE/gnutls/include" -DEXTRA_LIBS="$LIBS_BASE/geoip/lib/;$LIBS_BASE/gnutls/lib/;$LIBS_BASE/mysql/lib/;$LIBS_BASE/openldap/lib;$LIBS_BASE/openssl/lib/;$LIBS_BASE/pcre/lib;$LIBS_BASE/sqlite/lib/;$LIBS_BASE/gnutls/lib"
