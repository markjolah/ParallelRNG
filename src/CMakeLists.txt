# ParallelRngManager/src
# Main CMake for ParallelRngManager libraries

file(GLOB SRCS *.cpp) 

include(AddSharedStaticLibraries)
# add_shared_static_libraries()
# * Add shared and static library targets to project namespace
# * Sets target_include_directories
# * Sets target_compile_features and target_link_libraries
# * Installs target libraries and headers
# * returns lsit SHARED_STATIC_LIB_TARGETS of created targets for adjusting additional target properties
add_shared_static_libraries(SOURCES ${SRCS}
                            PUBLIC_COMPILE_FEATURES cxx_std_11
                            PUBLIC_LINK_LIBRARIES OpenMP::OpenMP_CXX TRNG::TRNG
                            PUBLIC_HEADER_DIR ${CMAKE_SOURCE_DIR}/include
                            PUBLIC_DEBUG_HEADER_DIR $${CMAKE_SOURCE_DIR}/include/debug)
