#
# ParallelRngManager
#

set(PRNG_HDRS ParallelRngManager.h)

set(PRNG_SRCS ParallelRngManager.cpp)

add_library(PRNGObjects OBJECT ${PRNG_SRCS})
target_compile_features(PRNGObjects PUBLIC cxx_std_11) #Declare C++11 required for building
set_target_properties(PRNGObjects PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
add_library(PRNGStatic STATIC $<TARGET_OBJECTS:PRNGObjects>)
add_library(PRNGShared SHARED $<TARGET_OBJECTS:PRNGObjects>)
set_target_properties(PRNGStatic PROPERTIES OUTPUT_NAME ParallelRngManager_static)
set_target_properties(PRNGShared PROPERTIES OUTPUT_NAME ParallelRngManager)

set(LIB_TARGETS PRNGStatic PRNGShared)
foreach(target ${LIB_TARGETS})
    target_compile_features(${target} PUBLIC cxx_std_11) #Declare C++11 required for building
    set_target_properties(${target} PROPERTIES PUBLIC_HEADER "${PRNG_HDRS}")
    target_link_libraries(${target} PUBLIC ${TRNG_LIBRARY} ) #Parallel random number generation
    target_link_libraries(${target} PUBLIC ${OPENMP_LIBRARY}) #OpenMP (non-empty for WIN32)
endforeach()

install(TARGETS ${LIB_TARGETS} EXPORT ${EXPORT_TARGETS_NAME}
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development
        PUBLIC_HEADER DESTINATION include/${PROJECT_NAME} COMPONENT Development)
