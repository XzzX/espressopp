file(GLOB_RECURSE ESPRESSO_SOURCES *.cpp)
file(GLOB_RECURSE NOT_ESPRESSO_SOURCES esutil/GenLogger.cpp)
file(GLOB_RECURSE DUMP_XTC_SOURCE io/DumpXTC.cpp io/DumpXTCAdress.cpp)

list(REMOVE_ITEM ESPRESSO_SOURCES ${NOT_ESPRESSO_SOURCES} ${DUMP_XTC_SOURCE})

add_custom_target(gitversion COMMAND ${CMAKE_COMMAND} -DTOP_SOURCE_DIR="${CMAKE_SOURCE_DIR}" -P ${ESPP_CMAKE_DIR}/gitversion.cmake)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/acconfig.hpp.cmakein
        ${CMAKE_CURRENT_BINARY_DIR}/acconfig.hpp)

#build .env for pipenv
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/pipenv-env.cmakein
        ${CMAKE_BINARY_DIR}/.env @ONLY)

add_library(_espressopp ${ESPRESSO_SOURCES})

if(ESPP_WERROR)
    target_compile_options(_espressopp PRIVATE "-Werror")
endif()
# compiler specific configurations
if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    if(ESPP_WALL)
        target_compile_options(_espressopp PRIVATE -Wall)
    endif()
    if(ESPP_LOCAL_ARCHITECTURE)
        target_compile_options(_espressopp PRIVATE "-march=native")
    endif()
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
    if(ESPP_LOCAL_ARCHITECTURE)
        target_compile_options(_espressopp PRIVATE "-march=native")
    endif()
endif()
if(CMAKE_CXX_COMPILER_ID MATCHES Intel)
    target_compile_options(_espressopp PRIVATE "-restrict")
    if(ESPP_LOCAL_ARCHITECTURE)
        target_compile_options(_espressopp PRIVATE "-xHost")
    endif()
    if(ESPP_VEC_REPORT)
        target_compile_options(_espressopp PRIVATE "-qopt-report-phase=vec,loop -qopt-report=5")
    endif()
endif()
get_target_property(ESPP_CFLAGS _espressopp COMPILE_OPTIONS)
message(STATUS "_espressopp compiler flags are: ${ESPP_CFLAGS}")

set_target_properties(_espressopp PROPERTIES ESP_PY_ENV "PYTHONPATH=${CMAKE_BINARY_DIR}:$ENV{PYTHONPATH}")

target_include_directories(_espressopp PUBLIC $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src> $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
     $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>)

target_link_libraries(_espressopp PUBLIC Boost::mpi Boost::serialization Boost::system Boost::filesystem Boost::python${PYTHON_VERSION_NO_DOT} Boost::numpy${PYTHON_VERSION_NO_DOT})
target_link_libraries(_espressopp PUBLIC Python3::Python)
target_link_libraries(_espressopp PUBLIC MPI::MPI_CXX)
target_link_libraries(_espressopp PRIVATE FFTW3::fftw3)

if(WITH_XTC)
    target_add_definitions(_espressopp PRIVATE -DHAS_GROMACS)
    target_link_libraries(_espressopp PRIVATE Gromacs::libgromacs)
    target_sources(_espressopp PRIVATE ${DUMP_XTC_SOURCE})
endif()
#python libs have not prefix (default would be 'lib')
set_target_properties(_espressopp PROPERTIES PREFIX "" SUFFIX ".so" LIBRARY_OUTPUT_DIRECTORY ..)
add_dependencies(_espressopp gitversion)

if(NOT PYTHON_INSTDIR)
    execute_process(COMMAND ${Python3_EXECUTABLE}
            -c "import distutils.sysconfig as cg; print(cg.get_python_lib(1,0,prefix='${CMAKE_INSTALL_EXEC_PREFIX}'))"
            OUTPUT_VARIABLE PYTHON_INSTDIR OUTPUT_STRIP_TRAILING_WHITESPACE)
endif()

install(TARGETS _espressopp LIBRARY DESTINATION ${PYTHON_INSTDIR} ARCHIVE DESTINATION ${PYTHON_INSTDIR})
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/ DESTINATION ${PYTHON_INSTDIR}/espressopp
        FILES_MATCHING PATTERN "*.py"
        PATTERN "CMakeFiles" EXCLUDE)

