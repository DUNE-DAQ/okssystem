
tdaq_package()

tdaq_add_library(osw src/*.cxx  LINK_LIBRARIES tdaq-common::ers)
tdaq_add_executable(system_test NOINSTALL test/SystemTest.cxx LINK_LIBRARIES osw)
