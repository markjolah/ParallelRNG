# ParallelRngManager/src
# Main CMake for ParallelRngManager libraries

file(GLOB SRCS *.cpp) 

include(AddSharedStaticLibraries)
# add_shared_static_libraries()
# * Add shared and static library targets to project namespace
# * Sets target_include_directories
# * Sets target_compile_features
# * Installs target libraries and headers
# * returns list: lib_targets - of created targets for adjusting additional target properties
add_shared_static_libraries(SOURCES ${SRCS}
                            COMPILE_FEATURES cxx_std_11
                            PUBLIC_HEADER_DIR ${CMAKE_SOURCE_DIR}/include
                            PUBLIC_DEBUG_HEADER_DIR ${CMAKE_SOURCE_DIR}/include/debug
                            RETURN_TARGETS lib_targets)

#Custom target settings for each lib_target from add_shared_static_libraries
foreach(target IN LISTS lib_targets)
    target_compile_definitions(${target} PUBLIC $<$<CONFIG:Debug>:PARALLEL_RNG_DEBUG>)
    target_link_libraries(${target} PUBLIC OpenMP::OpenMP_CXX)
    target_link_libraries(${target} INTERFACE Armadillo::Armadillo)
    target_link_libraries(${target} PUBLIC TRNG::TRNG)
endforeach()
