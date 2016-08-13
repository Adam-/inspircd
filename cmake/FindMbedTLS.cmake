############################################################################
# FindMdebTLS.txt
# Copyright (C) 2015  Belledonne Communications, Grenoble France
#
############################################################################
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
#
############################################################################
#
# - Find the mbedTLS include file and library
#
#  MBEDTLS_FOUND - system has mbedTLS
#  MBEDTLS_INCLUDE_DIRS - the mbedTLS include directory
#  MBEDTLS_LIBRARIES - The libraries needed to use mbedTLS

include(CMakePushCheckState)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckSymbolExists)


find_path(MBEDTLS_INCLUDE_DIRS
	NAMES mbedtls/ssl.h
	PATH_SUFFIXES include
)

find_library(MBEDTLS_LIBRARY
	NAMES mbedtls
)

set (MBEDTLS_LIBRARIES
	${MBEDTLS_LIBRARY}
)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(MbedTLS
	DEFAULT_MSG
	MBEDTLS_LIBRARIES
	MBEDTLS_INCLUDE_DIRS
)

mark_as_advanced(MBEDTLS_INCLUDE_DIRS MBEDTLS_LIBRARIES HAVE_SSL_GET_DTLS_SRTP_PROTECTION_PROFILE)
