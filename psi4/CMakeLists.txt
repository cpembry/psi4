cmake_minimum_required(VERSION 3.3 FATAL_ERROR)
project(psi4-core
        LANGUAGES C CXX)
# no Fortran in psi4-core proper, but language needs to be declared
#   for CMAKE_Fortran_IMPLICIT_LINK_LIBRARIES to be populated so
#   static Fortran add-ons can be linked
if(${Fortran_ENABLED})
    enable_language(Fortran)
endif()

list(APPEND CMAKE_MODULE_PATH ${PSI4_ROOT}/cmake)

include(psi4OptionsTools)
include(TestRestrict)
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
include(custom_color_messages)
test_restrict(restrict)
# GNUInstallDirs "DATADIR" wrong here; CMake search path wants "share".
set(CMAKECONFIG_INSTALL_DIR "share/cmake/psi4")

# <<<  Marshal Dependencies & Add-ons  >>>

set(_addons)
find_package(TargetLAPACK CONFIG REQUIRED)
get_property(_ill TARGET tgt::lapk PROPERTY INTERFACE_LINK_LIBRARIES)
list(GET _ill 0 _ill0)
get_property(_cd TARGET tgt::lapack PROPERTY INTERFACE_COMPILE_DEFINITIONS)
if(${_cd} MATCHES "USING_LAPACK_MKL")
    set(_isMKL " MKL")
endif()
get_property(_illb TARGET tgt::blas PROPERTY INTERFACE_LINK_LIBRARIES)
list(APPEND _addons ${_ill} ${_illb})
message(STATUS "${Cyan}Using LAPACK${_isMKL}${ColourReset}: ${_ill0};...")

if(ENABLE_ambit OR ENABLE_CheMPS2)
    find_package(TargetHDF5 CONFIG REQUIRED)
    get_property(_ill TARGET tgt::hdf5 PROPERTY INTERFACE_LINK_LIBRARIES)
    list(GET _ill 0 _ill0)
    list(APPEND _addons ${_ill})
    message(STATUS "${Cyan}Using HDF5${ColourReset}: ${_ill0};... (found version ${${PN}_VERSION})")
else()
    message(STATUS "Disabled HDF5")
endif()

#  <<  Pybind11 & Python  >>
set(PYBIND11_CPP_STANDARD "-std=c++${CMAKE_CXX_STANDARD}")
find_package(pybind11 2.2.3 EXACT CONFIG REQUIRED)
message(STATUS "${Cyan}Using pybind11${ColourReset}: ${pybind11_INCLUDE_DIR} (version ${pybind11_VERSION} for Py${PYTHON_VERSION_STRING} and ${PYBIND11_CPP_STANDARD})")
message(STATUS "${Cyan}Using Python ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR}${ColourReset}: ${PYTHON_EXECUTABLE}")

set(CMAKE_THREAD_PREFER_PTHREAD ON)
find_package(Threads REQUIRED)

find_package(DL)

set_property(GLOBAL PROPERTY PSI4_MODULES "")

if(${ENABLE_ambit})
    find_package(ambit CONFIG REQUIRED)
    get_property(_loc TARGET ambit::ambit PROPERTY LOCATION)
    message(STATUS "${Cyan}Using ambit${ColourReset}: ${_loc} (version ${ambit_VERSION})")
else()
    message(STATUS "Disabled ambit")
endif ()

if(${ENABLE_CheMPS2})
    find_package(CheMPS2 1.8.3 CONFIG REQUIRED)
    get_property(_loc TARGET CheMPS2::chemps2 PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using CheMPS2${ColourReset}: ${_loc} (version ${CheMPS2_VERSION})")
else()
    message(STATUS "Disabled CheMPS2")
endif ()

if(${ENABLE_dkh})
    find_package(dkh 1.2 CONFIG REQUIRED)
    get_property(_loc TARGET dkh::dkh PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using dkh${ColourReset}: ${_loc} (version ${dkh_VERSION})")
else()
    message(STATUS "Disabled dkh")
endif()

if(${ENABLE_libefp})
    find_package(libefp 1.5.0 CONFIG REQUIRED COMPONENTS shallow)
    get_property(_loc TARGET libefp::efp PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using libefp${ColourReset}: ${_loc} (version ${libefp_VERSION})")
else()
    message(STATUS "Disabled libefp")
endif()

if(${ENABLE_erd})
    find_package(erd 3.0.6 CONFIG REQUIRED)
    get_property(_loc TARGET erd::erd PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using erd${ColourReset}: ${_loc} (version ${erd_VERSION})")
else()
    message(STATUS "Disabled erd")
endif()

find_package(gau2grid 1.0 CONFIG REQUIRED)
get_property(_loc TARGET gau2grid::gg PROPERTY LOCATION)
list(APPEND _addons ${_loc})
message(STATUS "${Cyan}Using gau2grid${ColourReset}: ${_loc} (version ${gau2grid_VERSION})")

if(${ENABLE_gdma})
    find_package(gdma 2.2.6 CONFIG REQUIRED)
    get_property(_loc TARGET gdma::gdma PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using gdma${ColourReset}: ${_loc} (version ${gdma_VERSION})")
else()
    message(STATUS "Disabled gdma")
endif()

find_package(Libint 1.2.0 CONFIG REQUIRED COMPONENTS ${MAX_AM_ERI})
get_property(_loc TARGET Libint::int PROPERTY LOCATION)
list(APPEND _addons ${_loc})
message(STATUS "${Cyan}Using Libint ${Libint_MAX_AM_ERI}${ColourReset}: ${_loc} (version ${Libint_VERSION})")

if(${ENABLE_PCMSolver})
    find_package(PCMSolver 1.2.1 CONFIG REQUIRED)
    get_property(_loc TARGET PCMSolver::pcm PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using PCMSolver${ColourReset}: ${_loc} (version ${PCMSolver_VERSION})")
else()
    message(STATUS "Disabled PCMSolver")
endif ()

if(${ENABLE_simint})
    find_package(simint 0.7 CONFIG REQUIRED COMPONENTS am${MAX_AM_ERI} der0)
    get_property(_loc TARGET simint::simint PROPERTY LOCATION)
    list(APPEND _addons ${_loc})
    message(STATUS "${Cyan}Using simint ${simint_MAXAM}${ColourReset}: ${_loc} (version ${simint_VERSION}; vectorization ${simint_VECTOR})")
else()
    message(STATUS "Disabled simint")
endif()

find_package(Libxc 4.0.2 CONFIG REQUIRED)
get_property(_loc TARGET Libxc::xc PROPERTY LOCATION)
list(APPEND _addons ${_loc})
message(STATUS "${Cyan}Using Libxc${ColourReset}: ${_loc} (version ${Libxc_VERSION})")

if(APPLE)
    set(PRE_LIBRARY_OPTION -Wl,-all_load)
elseif(UNIX)
    set(PRE_LIBRARY_OPTION -Wl,--whole-archive)
    set(POST_LIBRARY_OPTION -Wl,--no-whole-archive)
endif()

# <<<  Build  >>>

include_directories(include)
include_directories(src)
add_subdirectory(src)

# <<<  Version  >>>
# * computes version from metadata.py and git info
# * calls cmake to run write_basic_package_version_file

add_custom_target(update_version ALL
                  COMMAND ${PYTHON_EXECUTABLE} versioner.py --metaout ${CMAKE_CURRENT_BINARY_DIR}/metadata.py
                                                            --cmakeout ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake
                  COMMAND cmake -DWTO="${CMAKE_CURRENT_BINARY_DIR}/${CMAKECONFIG_INSTALL_DIR}"
                                -DPN="psi4"
                                -P ${CMAKE_CURRENT_BINARY_DIR}/metadata.cmake
                  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                  COMMENT "Generating version info")
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/metadata.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${PYMOD_INSTALL_LIBDIR}/psi4)

# <<<  Install  >>>

    # <<<  install bin/  >>>
configure_file(run_psi4.py psi4 @ONLY)
install(PROGRAMS ${CMAKE_BINARY_DIR}/psi4
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_BINDIR})

    # <<<  install lib/  >>>
file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${PYMOD_INSTALL_LIBDIR}/psi4)

install(DIRECTORY share/psi4/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/psi4
        PATTERN "*.pyc" EXCLUDE)

install(DIRECTORY include/psi4/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/psi4)

install(DIRECTORY driver
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${PYMOD_INSTALL_LIBDIR}/psi4
        FILES_MATCHING PATTERN "*.py")

install(FILES header.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${PYMOD_INSTALL_LIBDIR}/psi4)

configure_file(__init__.py __init__.py @ONLY)
configure_file(extras.py extras.py @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/__init__.py
              ${CMAKE_BINARY_DIR}/extras.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${PYMOD_INSTALL_LIBDIR}/psi4)

install(FILES ../tests/pytest/conftest.py
              ../tests/pytest/test_psi4.py
              ../tests/pytest/test_aaa_profiling.py
              ../tests/pytest/test_addons.py
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_LIBDIR}${PYMOD_INSTALL_LIBDIR}/psi4/tests/)

    # <<<  install psi4 share/ & include/  >>>

install(DIRECTORY share/psi4/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/psi4
        MESSAGE_NEVER
        PATTERN "*.pyc" EXCLUDE)

install(DIRECTORY include/psi4/
        DESTINATION ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}/psi4)

    # <<<  install external's share/ to psi4 share/  >>>

if(TARGET libefp::efp)
    # bring libefp's fraglib to PSIDATADIR's attention
    set(_fraglib_in_psi4_dir ${DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_DATADIR}/psi4/efpfrag)

    add_custom_target(consolidate_psidatadir ALL
                      VERBATIM
                      COMMAND ${CMAKE_COMMAND} -E make_directory ${_fraglib_in_psi4_dir}
                      COMMENT "Symlink external resources into PSIDATADIR")

    file(GLOB _dotefps "${libefp_FRAGLIB_DIRS}/*.efp")
    foreach(_dotefp ${_dotefps})
        get_filename_component(_efpfile ${_dotefp} NAME)
        add_custom_command(TARGET consolidate_psidatadir
                           POST_BUILD
                           COMMAND ${CMAKE_COMMAND} -E create_symlink ${_dotefp} ${_fraglib_in_psi4_dir}/${_efpfile})
    endforeach()
endif()

# <<<  Export Config  >>>

if((DEFINED ENV{CONDA_BUILD}) AND (UNIX AND NOT APPLE))
    # installed-alongside GCC compilers compatible with the Intel compilers used in
    #   https://github.com/psi4/psi4meta/blob/master/conda-recipes/psi4-multiout/build.sh
    set(cache_CMAKE_C_COMPILER       "\${GCC}")
    set(cache_CMAKE_C_FLAGS          " -march=native -fopenmp")
    set(cache_CMAKE_CXX_COMPILER     "\${GXX}")
    set(cache_CMAKE_CXX_FLAGS        " -march=native -fopenmp")
    set(cache_CMAKE_Fortran_COMPILER "\${GFORTRAN}")
    set(cache_pybind11_DIR           "/opt/anaconda1anaconda2anaconda3/share/cmake/pybind11")
elseif((DEFINED ENV{CONDA_BUILD}) AND (APPLE) AND (CMAKE_CXX_COMPILER_ID MATCHES AppleClang))
    # for stdlib compatibility, must be consistent with Darwin section of
    #   https://github.com/psi4/psi4meta/blob/master/conda-recipes/psi4-core/build.sh
    set(cache_CMAKE_C_COMPILER       "/usr/bin/clang")
    set(cache_CMAKE_CXX_COMPILER     "/usr/bin/clang++")
    set(cache_CMAKE_CXX_FLAGS        " -stdlib=libc++")
    set(cache_CMAKE_Fortran_COMPILER "/opt/anaconda1anaconda2anaconda3/bin/gfortran")
    set(cache_pybind11_DIR           "/opt/anaconda1anaconda2anaconda3/share/cmake/pybind11")
else()
    # normal plugin use. this initializes the plugin compilation from the
    #   same-computer parent Psi4 compilation
    set(cache_CMAKE_C_COMPILER       "${CMAKE_C_COMPILER}")
    set(cache_CMAKE_C_FLAGS          "${CMAKE_C_FLAGS}")
    set(cache_CMAKE_CXX_COMPILER     "${CMAKE_CXX_COMPILER}")
    set(cache_CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS}")
    set(cache_CMAKE_Fortran_COMPILER "${CMAKE_Fortran_COMPILER}")
    set(cache_CMAKE_Fortran_FLAGS    "${CMAKE_Fortran_FLAGS}")
    set(cache_pybind11_DIR           "${pybind11_DIR}")
endif()

configure_file(psi4PluginCache.cmake.in psi4PluginCache.cmake @ONLY)
install(FILES ${CMAKE_BINARY_DIR}/psi4PluginCache.cmake
              ${PSI4_ROOT}/cmake/psi4OptionsTools.cmake
              ${PSI4_ROOT}/cmake/custom_static_library.cmake
              ${PSI4_ROOT}/cmake/custom_cxxstandard.cmake
              ${PSI4_ROOT}/cmake/autocmake_omp.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

    # <<<  Export Config for Plugins  >>>
# Determine relative path from Psi4's include directory to PyBind11's
file(RELATIVE_PATH RELATIVE_PYBIND11_INCLUDE_DIR
    ${DESTDIR}${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR}
    ${pybind11_INCLUDE_DIR})
configure_package_config_file(
        psi4Config.cmake.in
        ${CMAKE_CURRENT_BINARY_DIR}/share/cmake/psi4/psi4Config.cmake
        INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/share/cmake/psi4/psi4Config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/share/cmake/psi4/psi4ConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

