SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER ${TOOLCHAIN_PREFIX}-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_PREFIX}-g++)
SET(CMAKE_RC_COMPILER ${TOOLCHAIN_PREFIX}-windres)

# http://www.cmake.org/pipermail/cmake/2007-October/016784.html
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# http://stackoverflow.com/questions/19866926/cmake-toolchain-file-wont-use-my-custom-cflags
SET(CMAKE_CXX_FLAGS "-DMINGW -std=c++11" CACHE STRING "" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

include_directories("/home/adam/gnutls/include")
link_directories("/home/adam/gnutls/lib")
#set(CMAKE_FIND_ROOT_PATH "/home/adam/gnutls/include")
