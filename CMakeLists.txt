cmake_minimum_required(VERSION 3.12)
project(okssystem VERSION 1.0.0)

find_package(daq-cmake REQUIRED)

daq_setup_environment()

find_package(ers REQUIRED)
find_package(logging REQUIRED)

daq_add_library( *.cpp LINK_LIBRARIES ers::ers )

daq_add_application(okssystem_test OksSystemTest.cxx TEST LINK_LIBRARIES okssystem logging::logging)

daq_install()
