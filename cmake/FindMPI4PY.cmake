# - Find MPI4PY
# Find the native MPI4PY includes and library
#
# MPI4PY_INCLUDE_DIR - where to find mpi4py.h
# MPI4PY_LIBRARIES - List of libraries when using MPI4PY.
# MPI4PY_FOUND - True if MPI4PY found.

if (MPI4PY_INCLUDES)
  # Already in cache, be silent
  set (MPI4PY_FIND_QUIETLY TRUE)
endif (MPI4PY_INCLUDES)

if(Python3_EXECUTABLE)
  execute_process(COMMAND ${Python3_EXECUTABLE} 
      -c "import distutils.sysconfig as cg; print(cg.get_python_lib(1,0))"
		OUTPUT_VARIABLE Python3_SITEDIR OUTPUT_STRIP_TRAILING_WHITESPACE)

  execute_process(COMMAND "${Python3_EXECUTABLE}" 
                -c "import sys, mpi4py; sys.stdout.write(mpi4py.__version__)"
                    OUTPUT_VARIABLE MPI4PY_VERSION
                    RESULT_VARIABLE _MPI4PY_VERSION_RESULT
                    ERROR_QUIET)
  if(NOT _MPI4PY_VERSION_RESULT)
    message("-- mpi4py version: " ${MPI4PY_VERSION})
  else()
    set(MPI4PY_VERSION 0.0)
    message("-- mpi4py version: " ${MPI4PY_VERSION})
  endif()

  execute_process(COMMAND
      "${Python3_EXECUTABLE}" "-c" "import mpi4py; print(mpi4py.get_include())"
      OUTPUT_VARIABLE MPI4PY_INCLUDE_DIR
      OUTPUT_STRIP_TRAILING_WHITESPACE)
  message(STATUS "MPI4PY_INCLUDE = ${MPI4PY_INCLUDE_DIR}")
endif(Python3_EXECUTABLE)

find_path(MPI4PY_INCLUDE_DIR mpi4py/mpi4py.h HINTS ${MPI4PY_INCLUDE_DIR} ${Python3_SITEDIR}/mpi4py/include )
if(NOT MPI4PY_INCLUDES)
  message("     mpi4py.h not found. Please make sure you have installed the developer version of mpi4py")
endif()

message(STATUS "Looking for MPI.cpython-${PYTHON_VERSION_NO_DOT}-x86_64-linux-gnu.so")
find_file (MPI4PY_LIBRARY NAMES MPI.cpython-${PYTHON_VERSION_NO_DOT}-x86_64-linux-gnu.so MPI.cpython-${PYTHON_VERSION_NO_DOT}m-x86_64-linux-gnu.so HINTS ${MPI4PY_INCLUDE_DIR}/.. ${Python3_SITEDIR}/mpi4py)

# handle the QUIETLY and REQUIRED arguments and set MPI4PY_FOUND to TRUE if
# all listed variables are TRUE
include (FindPackageHandleStandardArgs)
find_package_handle_standard_args(MPI4PY REQUIRED_VARS MPI4PY_LIBRARY MPI4PY_INCLUDE_DIR VERSION_VAR MPI4PY_VERSION)

# Copy the results to the output variables and target.
if(MPI4PY_FOUND)
  set(MPI4PY_LIBRARIES ${MPI4PY_LIBRARY} )
  set(MPI4PY_INCLUDE_DIRS ${MPI4PY_INCLUDE_DIR} )

  if(NOT TARGET MPI4PY::mpi4py)
    add_library(MPI4PY::mpi4py UNKNOWN IMPORTED)
    set_target_properties(MPI4PY::mpi4py PROPERTIES
      IMPORTED_LOCATION "${MPI4PY_LIBRARY}"
      INTERFACE_INCLUDE_DIRECTORIES "${MPI4PY_INCLUDE_DIRS}")
  endif()
  if(NOT TARGET MPI4PY::include)
    add_library(MPI4PY::include INTERFACE IMPORTED)
    set_target_properties(MPI4PY::include PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${MPI4PY_INCLUDE_DIRS}")
  endif()
endif()

mark_as_advanced (MPI4PY_LIBRARIES MPI4PY_INCLUDES)
