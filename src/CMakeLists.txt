# ParallelRngManager/src
# Main CMake for ParallelRngManager libraries

file(GLOB SRCS *.cpp) 
set(PUBLIC_HEADER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../include)
set(PUBLIC_DEBUG_HEADER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../include/debug)

include(AddSharedStaticLibraries)
add_shared_static_libraries("${SRCS}" ${PROJECT_NAME}) 

foreach(target ${LIB_TARGETS})
    target_compile_features(${target} PUBLIC cxx_std_11)
    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${PUBLIC_HEADER_SRC_DIR}> 
                                                $<BUILD_INTERFACE:${PUBLIC_DEBUG_HEADER_SRC_DIR}> 
                                                $<INSTALL_INTERFACE:include>)
    target_link_libraries(${target} PUBLIC TRNG::TRNG ) #Parallel random number generation
    target_link_libraries(${target} PUBLIC OpenMP::OpenMP_CXX)
endforeach()

install(TARGETS ${LIB_TARGETS} EXPORT ${PROJECT_NAME}Targets
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development)

install(DIRECTORY ${PUBLIC_HEADER_SRC_DIR}/ DESTINATION include COMPONENT Development)
install(DIRECTORY ${PUBLIC_DEBUG_HEADER_SRC_DIR}/ DESTINATION include CONFIGURATIONS Debug COMPONENT Development)
