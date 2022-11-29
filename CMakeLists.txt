cmake_minimum_required(VERSION 3.12)
project(system VERSION 0.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(ers REQUIRED)
find_package(logging REQUIRED)

daq_add_library( *.cpp LINK_LIBRARIES ers::ers )

daq_add_application(system_test SystemTest.cxx TEST LINK_LIBRARIES system logging::logging)

daq_install()
