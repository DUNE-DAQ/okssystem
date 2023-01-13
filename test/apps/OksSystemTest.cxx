/*
 *  OksSystemTest.cxx
 *  Test
 *
 *  JCF, Oct-17-2022: OksSystemTest as written by Matthias Wiesmann has
 *  been modified to work in the DUNE DAQ framework
 *
 *  Created by Matthias Wiesmann on 24.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */
#include <iostream>
#include <sstream>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "ers/ers.hpp"
#include "logging/Logging.hpp"

#include "okssystem/OksSystem.hpp"
#include "okssystem/exceptions.hpp"

#include "OksSystemTest.hpp"


void test_okssystem(const char* path, const char *text) {
  TLOG_DEBUG( 1) << "Testing executable \"" << path << "\" with command " << text ; 
    std::ostringstream stream ;
    stream << "echo \""  << text << "\" > " << path;
    std::string result = OksSystem::Executable::okssystem(stream.str()); 
    TLOG_DEBUG( 1) << "result: " << result ; 
} // test_okssystem 

void test_write_chmod(const OksSystem::File &file) {
  TLOG_DEBUG( 1) << "Testing OksSystem::File::output " << file.c_full_name() ;
    std::ostream* stream= file.output();
    (*stream) << "#!/bin/sh\necho $1\nexit $1\n";
    delete(stream); 
    mode_t permission = 0700;
    TLOG_DEBUG( 1) << "Testing OksSystem::File::permissions " << file.c_full_name();
    TLOG_DEBUG( 1) << "Setting permissions: " << OksSystem::File::pretty_permissions(permission).c_str() << " " << permission;
    file.permissions(permission);
    TLOG_DEBUG( 1) << "Checking permisions: " << file.pretty_permissions().c_str() << " " << file.permissions();
    if (file.permissions()==permission) {
      TLOG_DEBUG( 1) << "Permission check: ok";
    } else {
	ers::warning(OksSystem::Exception(ERS_HERE, std::string("Permission check: fail")));
	exit (183);
    } 
} // text_exec

int test_exec(const OksSystem::Executable &executable, int status ) {
  TLOG_DEBUG( 1) << "Testing OksSystem::Executable::start \"" << executable.c_full_name() << "\" parameter: " << status; 
    std::vector<std::string> params;
    std::ostringstream temp_stream;
    temp_stream << status;
    params.push_back(temp_stream.str());
    OksSystem::Process p = executable.start(params);
    int exec_status = p.join(true) ;
    TLOG_DEBUG( 1) << "Testing executable test: result %d" << exec_status; 
    return exec_status;
} // int	
	
void test_map_file(const OksSystem::File &file) {
  TLOG_DEBUG( 1) << "Mapping file " << file.c_full_name(); 
    const int s = 4096 ;
    OksSystem::MapFile map_file(file,s,0,true,false); 
    map_file.map();
    void *a = map_file.address() ;
    char *buffer = (char *) calloc(sizeof(char), s); 
    memcpy(buffer,a,s);
    buffer[s-1] = '\0' ;
    TLOG_DEBUG( 1) << "Memory map buffer contains " << buffer; 
    map_file.unmap();
} // test_map_file

void test_mkdir(const OksSystem::File &file) {
  TLOG_DEBUG( 1) << "Creating directory " << file.c_full_name(); 
    file.make_path(0700);
    TLOG_DEBUG( 1) << "Directory depth is " << file.depth();
} //test_mkdir

void test_rmdir(const OksSystem::File &file) {
  TLOG_DEBUG( 1) << "Deleting directory " << file.c_full_name(); 
    file.remove(); 
} // test_rmdir 

void test_host() {
  TLOG_DEBUG( 1) << "Checking host information" ; 
    const OksSystem::LocalHost *host = OksSystem::LocalHost::instance();
    std::cout << "short name\t"<< host->name() << std::endl; 
    std::cout << "full name\t" << host->full_name() << std::endl;
    std::cout << "ip address\t" << host->ip_string() << std::endl;
    std::cout << "description\t" << host->description() << std::endl ;
} // test_host

void test_delete_file(const OksSystem::File &file) {
  TLOG_DEBUG( 1) << "Deleting file \"%s\"" << file.c_full_name(); 
    file.unlink(); 
} // test_delete_file

void test_path(const OksSystem::Path &path, const std::string &name) {
  TLOG() << "Testing path " << path.to_string().c_str() ; 
  TLOG() << "Searching for " << name.c_str() ; 
    OksSystem::File file = path.which(name);
    TLOG() << "Found \"" << file.c_full_name() << "\" of type " << file.file_type() ; 
}// test_path

void test_user() {
    OksSystem::User user;
    std::cout << "username\t" << user << std::endl ;
} // 

void test_process() {
    const OksSystem::Process *process = OksSystem::Process::instance(); 
    std::cout << "process\t" << *process << std::endl;
} // test_process


int main(int , char** argv) {
    
    try {
	OksSystem::Process::set_name(argv[0]); 
	OksSystem::Executable file("/tmp/okssystem_test") ;
	test_okssystem(file,"Hello world"); 
	test_map_file(file); 
	test_write_chmod(file); 
	test_exec(file,0); 
	test_delete_file(file); 
	OksSystem::File dir_a("/tmp/really/stupid/path/");
	test_mkdir(dir_a); 
	OksSystem::File dir_b("/tmp/really/");
	test_rmdir(dir_b);
	OksSystem::Path path("/bin::/usr/bin:/usr/local/bin:/sbin/");
	test_path(path,"ping"); 
	test_host();
	test_user(); 
	test_process();
        
	return 0;
    } catch (ers::Issue & e) {
	ers::error(e) ;
	return 183;
    } // catch 
} // main 


