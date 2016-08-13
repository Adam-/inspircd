cmake_minimum_required(VERSION 2.8)

project(InspIRCd CXX)

set(BIN_PATH "bin" CACHE PATH "Binary path")
set(CONF_PATH "conf" CACHE PATH "Configuration file path")
set(MODULE_PATH "modules" CACHE PATH "Module path")
set(DATA_PATH "data" CACHE PATH "Data path")
set(LOG_PATH "logs" CACHE PATH "Log file path")

set(EXTRA_INCLUDES "" CACHE PATH "Extra include paths")
set(EXTRA_LIBS "" CACHE PATH "Extra library paths")

set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/cmake")

# Grab version info from version.sh
file(STRINGS "${PROJECT_SOURCE_DIR}/src/version.sh" VERSIONSH)
string(REGEX REPLACE ".*InspIRCd-([0-9]*).*" "\\1" VERSION_MAJOR "${VERSIONSH}")
string(REGEX REPLACE ".*InspIRCd-[0-9]*\\.([0-9]*).*" "\\1" VERSION_MINOR "${VERSIONSH}")
string(REGEX REPLACE ".*InspIRCd-[0-9]*\\.[0-9]*\\.([0-9]*).*" "\\1" VERSION_PATCH "${VERSIONSH}")
set(VERSION_FULL "${VERSION_MAJOR}.${VERSION_MINOR}.${VERSION_PATCH}")

include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

include(inspircd)
include_directories(include)

include_directories(${EXTRA_INCLUDES})
link_directories(${EXTRA_LIBS})

include(CheckCXXSourceRuns)
CHECK_CXX_SOURCE_RUNS("compiler" "${PROJECT_SOURCE_DIR}/cmake/test/compiler.cpp" HAS_COMPILER "")
CHECK_CXX_SOURCE_RUNS("eventfd" "${PROJECT_SOURCE_DIR}/cmake/test/eventfd.cpp" HAS_EVENTFD "")
CHECK_CXX_SOURCE_RUNS("clock_gettime" "${PROJECT_SOURCE_DIR}/cmake/test/clock_gettime.cpp" HAS_CLOCK_GETTIME "rt")
CHECK_CXX_SOURCE_RUNS("kqueue" "${PROJECT_SOURCE_DIR}/cmake/test/kqueue.cpp" HAS_KQUEUE "")

if(NOT HAS_COMPILER)
  message(FATAL_ERROR "Your compiler is not supported")
endif()

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

