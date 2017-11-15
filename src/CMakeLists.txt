#
# ParallelRngManager
#

set(HDRS ParallelRngManager.h)

set(SRCS ParallelRngManager.cpp)


if(BUILD_SHARED_LIBS)
    add_library(${PROJECT_NAME} SHARED ${SRCS})
    set(LIB_TARGETS ${PROJECT_NAME})
endif()

if(BUILD_STATIC_LIBS)
    add_library(${PROJECT_NAME}_static STATIC ${SRCS})
    set(LIB_TARGETS ${LIB_TARGETS} ${PROJECT_NAME}_static)
    set_target_properties(${PROJECT_NAME}_static PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
endif()

foreach(target ${LIB_TARGETS})
    target_compile_features(${target} PUBLIC cxx_std_11) #Declare C++11 required for building
    set_target_properties(${target} PROPERTIES PUBLIC_HEADER "${HDRS}")
    target_include_directories(${target} INTERFACE $<INSTALL_INTERFACE:include>)
    target_link_libraries(${target} PUBLIC ${TRNG_LIBRARY} ) #Parallel random number generation
    target_link_libraries(${target} PUBLIC ${OPENMP_LIBRARY}) #OpenMP (non-empty for WIN32)
    get_property(_VAR TARGET ${target} PROPERTY INCLUDE_DIRECTORIES)
    message(STATUS "${target} INCLUDE_DIRECTORIES: ${_VAR}")
    get_property(_VAR TARGET ${target} PROPERTY INTERFACE_INCLUDE_DIRECTORIES)
    message(STATUS "${target} INTERFACE_INCLUDE_DIRECTORIES: ${_VAR}")
    get_property(_VAR TARGET ${target} PROPERTY INTERFACE_LINK_LIBRARIES)
    message(STATUS "${target} INTERFACE_LINK_LIBRARIES: ${_VAR}")
    get_property(_VAR TARGET ${target} PROPERTY COMPILE_FEATURES)
    message(STATUS "${target} COMPILE_FEATURES: ${_VAR}")
    get_property(_VAR TARGET ${target} PROPERTY INTERFACE_COMPILE_FEATURES)
    message(STATUS "${target} INTERFACE_COMPILE_FEATURES: ${_VAR}")
endforeach()

install(TARGETS ${LIB_TARGETS} EXPORT ${EXPORT_TARGETS_NAME}
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME} COMPONENT Development)
