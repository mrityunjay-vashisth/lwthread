cmake_minimum_required(VERSION 3.10)
project(lwthread VERSION 0.1.0 
        DESCRIPTION "Lightweight threading library"
        LANGUAGES C)

# Set C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared libraries" ON)
option(LWTHREAD_BUILD_EXAMPLES "Build example programs" ON)
option(LWTHREAD_BUILD_TESTS "Build test programs" OFF)

# Headers
include_directories(include)

# Core library source files
set(LWTHREAD_SOURCES
    src/lwthread.c
    src/queue.c
    src/scheduler.c
    src/thread.c
)

# Create the library
add_library(lwthread ${LWTHREAD_SOURCES})
target_include_directories(lwthread PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_link_libraries(lwthread PRIVATE pthread)

# Build examples
if(LWTHREAD_BUILD_EXAMPLES)
    add_executable(simple_threads examples/simple_threads.c)
    target_link_libraries(simple_threads PRIVATE lwthread)
    
    # Additional examples can be added here
endif()

# Installation
include(GNUInstallDirs)
install(TARGETS lwthread
    EXPORT lwthread-targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

# Export targets
install(EXPORT lwthread-targets
    FILE lwthread-targets.cmake
    NAMESPACE lwthread::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/lwthread
)