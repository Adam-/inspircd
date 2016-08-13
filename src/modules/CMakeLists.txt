# m_regex_stdlib is supported by every version of Visual Studio we support,
# so copy the file out of extra/
if(MSVC)
	file(COPY "${PROJECT_SOURCE_DIR}/src/modules/extra/m_regex_stdlib.cpp" DESTINATION "${PROJECT_SOURCE_DIR}/src/modules/")
endif(MSVC)

file(GLOB INSPIRCD_MODULES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "../commands/*.cpp" "*.cpp")
list(SORT INSPIRCD_MODULES)

add_definitions("-DDLL_BUILD")

foreach(MODULE_NAME ${INSPIRCD_MODULES})
	get_filename_component(BASE_NAME ${MODULE_NAME} NAME)
	string(REGEX REPLACE "(.+).cpp$" "\\1" BASE_NAME ${BASE_NAME})
	set(SO_NAME "${BASE_NAME}.so")
	add_library(${SO_NAME} MODULE ${MODULE_NAME})
	set_target_properties(${SO_NAME} PROPERTIES
		PREFIX ""
		SUFFIX ""
		COMPILE_DEFINITIONS "MODNAME=\"${BASE_NAME}\""
	)

	target_link_libraries(${SO_NAME} inspircd)
	add_dependencies(${SO_NAME} inspircd)

	if(MSVC)
		target_link_libraries(${SO_NAME} win32_memory)
		add_dependencies(${SO_NAME} win32_memory)
	endif(MSVC)

	find_required_libraries(${MODULE_NAME} MODULE_DEPS)
	foreach(MODULE_DEP ${MODULE_DEPS})
		find_package(${MODULE_DEP})
		string(TOUPPER ${MODULE_DEP} MODULE_DEP)
		include_directories(${${MODULE_DEP}_INCLUDE_DIR})
		target_link_libraries(${SO_NAME} ${${MODULE_DEP}_LIBRARIES})
	endforeach(MODULE_DEP)

	install(TARGETS ${SO_NAME} DESTINATION modules)
endforeach(MODULE_NAME ${INSPIRCD_MODULES})

file(GLOB INSPIRCD_MODULES_SPANNINGTREE RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} "${PROJECT_SOURCE_DIR}/src/modules/m_spanningtree/*.cpp")
list(SORT INSPIRCD_MODULES_SPANNINGTREE)

add_library(m_spanningtree.so MODULE ${INSPIRCD_MODULES_SPANNINGTREE})
set_target_properties(m_spanningtree.so PROPERTIES
	PREFIX ""
	SUFFIX ""
	COMPILE_DEFINITIONS "MODNAME=\"m_spanningtree\""
)

target_link_libraries(m_spanningtree.so inspircd)
add_dependencies(m_spanningtree.so inspircd)
if(MSVC)
	target_link_libraries(m_spanningtree.so win32_memory)
	add_dependencies(m_spanningtree.so win32_memory)
endif(MSVC)
install(TARGETS m_spanningtree.so DESTINATION modules)
