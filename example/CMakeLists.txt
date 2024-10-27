aux_source_directory(. EXAMPLE_SOURCES)
# set(EXAMPLE_SOURCES
#     tcpserver.cpp
#     timerevent_test.cpp
#     udpserver.cpp
# )

foreach(EXAMPLE_SRC ${EXAMPLE_SOURCES})
    get_filename_component(EXAMPLE_NAME ${EXAMPLE_SRC} NAME_WE)
    
    add_executable(${EXAMPLE_NAME} ${EXAMPLE_SRC})
    
    target_link_libraries(${EXAMPLE_NAME} PRIVATE moonnet)
    
    set_target_properties(${EXAMPLE_NAME} PROPERTIES
      RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin
    )
    
endforeach()
