aux_source_directory(. SOURCES)
# set(SOURCES
#     acceptor.cpp
#     bfevent.cpp
#     buffer.cpp
#     event.cpp
#     eventloop.cpp
#     loopthread.cpp
#     looptpool.cpp
#     server.cpp
#     signalevent.cpp
#     timerevent.cpp
#     udpevent.cpp
#     wrap.cpp
# )

add_library(moonnet SHARED ${SOURCES})

add_library(moonnet_static STATIC ${SOURCES})

set_target_properties(moonnet PROPERTIES
    OUTPUT_NAME moonnet
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
    PUBLIC_HEADER ../include/moonnet.h
)

set_target_properties(moonnet_static PROPERTIES
    OUTPUT_NAME moonnet
    VERSION ${PROJECT_VERSION}
    PUBLIC_HEADER ../include/moonnet.h
)

target_include_directories(moonnet
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/moonnet>
        $<INSTALL_INTERFACE:include/moonnet>
)

target_include_directories(moonnet_static
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include/moonnet>
        $<INSTALL_INTERFACE:include/moonnet>
)

install(TARGETS moonnet
    EXPORT moonnetTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/moonnet
)

install(TARGETS moonnet_static
    EXPORT moonnetTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/moonnet
)
