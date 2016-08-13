# Copyright (C) 2007-2012 LuaDist.
# Created by Peter Drahos <drahosp@gmail.com>
# Redistribution and use of this file is allowed according to the terms of the MIT license.
# For details see the COPYRIGHT file distributed with LuaDist.
#
# - Find tre
# Find the native TRE headers and libraries.
#
# TRE_INCLUDE_DIRS	- where to find tre.h, etc.
# TRE_LIBRARIES	- List of libraries when using tre.
# TRE_FOUND	- True if tre found.

# Look for the header file.
find_path ( TRE_INCLUDE_DIR NAMES tre/tre.h )

# Look for the library.
find_library ( TRE_LIBRARY NAMES tre )

# Handle the QUIETLY and REQUIRED arguments and set TRE_FOUND to TRUE if all listed variables are TRUE.
include ( FindPackageHandleStandardArgs )
find_package_handle_standard_args ( TRE DEFAULT_MSG TRE_LIBRARY TRE_INCLUDE_DIR )

# Copy the results to the output variables.
if ( TRE_FOUND )
  set ( TRE_LIBRARIES ${TRE_LIBRARY} )
  set ( TRE_INCLUDE_DIRS ${TRE_INCLUDE_DIR} )
else ( TRE_FOUND )
  set ( TRE_LIBRARIES )
  set ( TRE_INCLUDE_DIRS )
endif ( TRE_FOUND )

mark_as_advanced ( TRE_INCLUDE_DIRS TRE_LIBRARIES )
