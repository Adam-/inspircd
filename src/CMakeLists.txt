
# Get main sources
file(GLOB INSPIRCD_SOURCES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "*.cpp" "modes/*.cpp")

# Add socket engine
list(APPEND INSPIRCD_SOURCES "socketengines/socketengine_${SOCKETENGINE}.cpp")

# Add OS specific files
if(WIN32)
	list(APPEND INSPIRCD_SOURCES
		"${PROJECT_SOURCE_DIR}/win/inspircd_win32wrapper.cpp"
		"${PROJECT_SOURCE_DIR}/win/win32service.cpp"
		"${PROJECT_SOURCE_DIR}/src/threadengines/threadengine_win32.cpp"
		"${PROJECT_SOURCE_DIR}/win/inspircd.rc")
else(WIN32)
	list(APPEND INSPIRCD_SOURCES "threadengines/threadengine_pthread.cpp")
endif(WIN32)

# Sort all of the sources so they build alphabetically
list(SORT INSPIRCD_SOURCES)

add_executable(inspircd ${INSPIRCD_SOURCES})
if(WIN32)
	target_link_libraries(inspircd win32_memory)
else(WIN32)
	target_link_libraries(inspircd dl pthread)
	if(HAS_CLOCK_GETTIME)
		target_link_libraries(inspircd rt)
	endif(HAS_CLOCK_GETTIME)
endif(WIN32)
set_target_properties(inspircd PROPERTIES ENABLE_EXPORTS ON)
install(TARGETS inspircd DESTINATION ${BIN_PATH})

add_subdirectory(modules)

