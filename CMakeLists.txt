cmake_minimum_required(VERSION 3.25)
project(osimperf2 VERSION 0.0.1 LANGUAGES CXX C)

if(NOT (UNIX AND NOT APPLE))
    message(FATAL_ERROR "Only Linux builds are currently supported (check CMAKE_INSTALL_RPATH etc.)")
endif()

include(GNUInstallDirs)  # CMAKE_INSTALL_BINDIR

find_package(OpenSim REQUIRED)

set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH OFF)

add_executable(osimperf2-runner osimperf2-runner.cpp)
set_target_properties(osimperf2-runner PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS OFF)
target_compile_features(osimperf2-runner PRIVATE cxx_std_17)
target_link_libraries(osimperf2-runner PRIVATE ${OpenSim_LIBRARIES})
target_include_directories(osimperf2-runner PRIVATE ${OpenSim_INCLUDE_DIRS})

foreach(path IN LISTS CMAKE_PREFIX_PATH)
    target_link_directories(osimperf2-runner PRIVATE "${path}/${CMAKE_INSTALL_LIBDIR}")
endforeach()

install(TARGETS osimperf2-runner RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

