
macro(find_required_libraries SRC LIBRARIES_OUT)
	set(${LIBRARIES_OUT} "")
	file(STRINGS ${SRC} RESULT REGEX "/\\* Libraries: ([^ ]*) \\*/")
	string(REGEX REPLACE "/\\* Libraries: ([^ ]*) \\*/" "\\1" LIBRARIES "${RESULT}")
	set(${LIBRARIES_OUT} ${LIBRARIES})
endmacro(find_required_libraries)
