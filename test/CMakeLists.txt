cmake_minimum_required(VERSION 3.0)

project(InspIRCd CXX)

set(CONFIG_DIR "conf" CACHE PATH "Configuration file path")
set(MODULE_DIR "modules" CACHE PATH "Module path")
set(DATA_DIR "data" CACHE PATH "Data path")
set(LOG_DIR "logs" CACHE PATH "Log file path")

set(INSPIRCD_BASE "${CMAKE_CURRENT_SOURCE_DIR}/../")

set(SYSTEM_NAME_VERSION ${CMAKE_SYSTEM})
set(SOCKETENGINE "select")

# Grab version info from version.sh
file(STRINGS "${INSPIRCD_BASE}/src/version.sh" VERSIONSH)
string(REGEX REPLACE ".*InspIRCd-([0-9]*).*" "\\1" VERSION_MAJOR "${VERSIONSH}")
string(REGEX REPLACE ".*InspIRCd-[0-9]*\\.([0-9]*).*" "\\1" VERSION_MINOR "${VERSIONSH}")
string(REGEX REPLACE ".*InspIRCd-[0-9]*\\.[0-9]*\\.([0-9]*).*" "\\1" VERSION_PATCH "${VERSIONSH}")

file(GLOB INSPIRCD_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
"${INSPIRCD_BASE}/src/*.cpp"
"${INSPIRCD_BASE}/src/modes/*.cpp"
"${INSPIRCD_BASE}/src/socketengines/socketengine_select.cpp"
"${INSPIRCD_BASE}/src/threadengines/threadengine_pthread.cpp"
"${INSPIRCD_BASE}/test/*.cpp"
"${INSPIRCD_BASE}/src/modules/m_stripcolor.cpp"
)
list(SORT INSPIRCD_SOURCES)

set_property(SOURCE "${INSPIRCD_BASE}/src/modules/m_stripcolor.cpp" APPEND PROPERTY COMPILE_DEFINITIONS MODNAME="m_stripcolor")

include_directories("${INSPIRCD_BASE}/include")

configure_file("${INSPIRCD_BASE}/make/template/config.h" "${INSPIRCD_BASE}/include/config.h")

enable_testing()

file(GLOB TEST_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "modules/*.cpp")
list(SORT TEST_SOURCES)

add_definitions(-DINSPIRCD_TEST -DPURE_STATIC)
add_executable(inspircd_test "${TEST_SOURCES}" "${INSPIRCD_SOURCES}")
target_link_libraries(inspircd_test pthread dl gtest gtest_main)
set_property(TARGET inspircd_test PROPERTY CXX_STANDARD 11)
set_property(TARGET inspircd_test PROPERTY CXX_STANDARD_REQUIRED ON)

add_test(inspircd-tests inspircd_test)
