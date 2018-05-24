# ParallelRngManager/src
# Main CMake for ParallelRngManager libraries

file(GLOB SRCS *.cpp) 
set(PUBLIC_HEADER_SRC_DIR ${CMAKE_CURRENT_LIST_DIR}/../include)

foreach (TYPE IN ITEMS STATIC SHARED)
    if(BUILD_${TYPE}_LIBS)
        add_library(${PROJECT_NAME}_${TYPE} SHARED ${SRCS})
        #Alias to allow direct useage as an in-source sub-project for clients
        add_library(${PROJECT_NAME}::${PROJECT_NAME}_${TYPE} ALIAS ${PROJECT_NAME}_${TYPE})
        set_target_properties(${PROJECT_NAME}_${TYPE} PROPERTIES OUTPUT_NAME ${PROJECT_NAME})
        set(LIB_TARGETS ${LIB_TARGETS} ${PROJECT_NAME}_${TYPE})
    endif()
endforeach()

#Default the main dependency to alias to shared libraries if they are built
#This can be used by subfolders within this project or if this project is included as
#an in-source 3rd party sub-project.
if (BUILD_SHARED_LIBS)
  add_library (${PROJECT_NAME} ALIAS ${PROJECT_NAME}_SHARED)
  add_library (${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_SHARED)
else ()
  add_library (${PROJECT_NAME} ALIAS ${PROJECT_NAME}_STATIC)
  add_library (${PROJECT_NAME}::${PROJECT_NAME} ALIAS ${PROJECT_NAME}_STATIC)
endif()

foreach(target ${LIB_TARGETS})
    target_compile_features(${target} PUBLIC cxx_std_11) #Declare C++11 required for building
    target_include_directories(${target} PUBLIC $<BUILD_INTERFACE:${PUBLIC_HEADER_SRC_DIR}> 
                                                $<INSTALL_INTERFACE:include>)
    target_link_libraries(${target} PUBLIC TRNG::TRNG ) #Parallel random number generation
    target_link_libraries(${target} PUBLIC OpenMP::OpenMP_CXX)
endforeach()

install(TARGETS ${LIB_TARGETS} EXPORT ${EXPORT_TARGETS_NAME}
        LIBRARY DESTINATION lib COMPONENT Runtime
        ARCHIVE DESTINATION lib COMPONENT Development)

install(DIRECTORY ${PUBLIC_HEADER_SRC_DIR}/ DESTINATION include COMPONENT Development)
