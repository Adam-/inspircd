cmake_minimum_required(VERSION 2.8)

project(InspIRCd CXX)

set(BIN_PATH "bin" CACHE PATH "Binary path")
set(CONF_PATH "conf" CACHE PATH "Configuration file path")
set(MODULE_PATH "modules" CACHE PATH "Module path")
set(DATA_PATH "data" CACHE PATH "Data path")
set(LOG_PATH "logs" CACHE PATH "Log file path")

set(EXTRA_INCLUDES "" CACHE PATH "Extra include paths")
set(EXTRA_LIBS "" CACHE PATH "Extra library paths")

set(INSPIRCD_BASE ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_MODULE_PATH "${INSPIRCD_BASE}/cmake")

# Grab version info from version.sh
file(STRINGS "${INSPIRCD_BASE}/src/version.sh" VERSIONSH)
string(REGEX REPLACE ".*InspIRCd-([0-9]*).*" "\\1" MAJOR_VERSION "${VERSIONSH}")
string(REGEX REPLACE ".*InspIRCd-[0-9]*\\.([0-9]*).*" "\\1" MINOR_VERSION "${VERSIONSH}")
string(REGEX REPLACE ".*InspIRCd-[0-9]*\\.[0-9]*\\.([0-9]*).*" "\\1" PATCH_VERSION "${VERSIONSH}")
set(FULL_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${PATCH_VERSION}")

include(inspircd)
include_directories(include)

include_directories(${EXTRA_INCLUDES})
link_directories(${EXTRA_LIBS})

include(CheckCXXSourceRuns)
CHECK_CXX_SOURCE_RUNS("eventfd" "${INSPIRCD_BASE}/cmake/test/eventfd.cpp" HAS_EVENTFD "")
CHECK_CXX_SOURCE_RUNS("clock_gettime" "${INSPIRCD_BASE}/cmake/test/clock_gettime.cpp" HAS_CLOCK_GETTIME "rt")
CHECK_CXX_SOURCE_RUNS("kqueue" "${INSPIRCD_BASE}/cmake/test/kqueue.cpp" HAS_KQUEUE "")

include(CheckIncludeFileCXX)
CHECK_INCLUDE_FILE_CXX("sys/epoll.h" HAS_EPOLL)
CHECK_INCLUDE_FILE_CXX("port.h" HAS_PORT)
CHECK_INCLUDE_FILE_CXX("poll.h" HAS_POLL)

set(SOCKETENGINE "select")
if(HAS_POLL)
	set(SOCKETENGINE "poll")
endif(HAS_POLL)
if(HAS_PORT)
	set(SOCKETENGINE "ports")
endif(HAS_PORT)
if(HAS_KQUEUE)
	set(SOCKETENGINE "kqueue")
endif(HAS_KQUEUE)
if(HAS_EPOLL)
	set(SOCKETENGINE "epoll")
endif(HAS_EPOLL)

configure_file(include/config.h.cmake include/config.h)

find_program(PERL perl)
if(PERL)
	set_property(GLOBAL PROPERTY RULE_MESSAGES OFF)
	set(CMAKE_CXX_COMPILE_OBJECT "${PERL} ${INSPIRCD_BASE}/cmake/run-cc.pl ${CMAKE_CXX_COMPILE_OBJECT}")
	set(CMAKE_CXX_LINK_EXECUTABLE "${PERL} ${INSPIRCD_BASE}/cmake/run-cc.pl ${CMAKE_CXX_LINK_EXECUTABLE}")
	set(CMAKE_CXX_CREATE_SHARED_MODULE "${PERL} ${INSPIRCD_BASE}/cmake/run-cc.pl ${CMAKE_CXX_CREATE_SHARED_MODULE}")
endif(PERL)

if(WIN32)
	add_subdirectory(win32)
endif(WIN32)
add_subdirectory(src)
add_subdirectory(docs/conf)

if(OSX)
	configure_file(cmake/org.inspircd.plist.template.cmake ${CMAKE_CURRENT_BINARY_DIR}/org.inspircd.plist)

	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/org.inspircd.plist DESTINATION .)
else(OSX)
	configure_file(cmake/inspircd.template.cmake ${CMAKE_CURRENT_BINARY_DIR}/inspircd @ONLY)

	# Install inspircd wrapper
	install(FILES ${CMAKE_CURRENT_BINARY_DIR}/inspircd DESTINATION . PERMISSIONS OWNER_EXECUTE OWNER_WRITE
		OWNER_READ GROUP_EXECUTE GROUP_READ)
endif(OSX)

# Create an empty data and logs directory and install them
file(MAKE_DIRECTORY ${DATA_PATH})
install(DIRECTORY ${DATA_PATH} DESTINATION .)
file(MAKE_DIRECTORY ${LOG_PATH})
install(DIRECTORY ${LOG_PATH} DESTINATION .)

