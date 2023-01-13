/*
 *  Process.h
 *  OksSystem
 *
 *  Created by Matthias Wiesmann on 18.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef OKSSYSTEM_PROCESS
#define OKSSYSTEM_PROCESS

#include <sys/types.h>
#include <string>

namespace OksSystem {
    
    /** This class represents a process 
      * It offers basic methods to manipulate a process
      * \author Matthias Wiesmann
      * \version 1.0
      * \brief Wrapper for process manipulation. 
      */
    
    class Process {
protected:
	pid_t              m_process_id ;                    ///< \brief process identifier
	std::string        m_process_name ;                  ///< \brief process name - only to be used for display
	static const char* const SYS_EXITS_NAMES[] ;         ///< \brief names for standard system exit values
	static const char* const TEST_EXITS_NAMES[] ;        ///< \brief names for testing exit values
	static const char* const OK_EXIT_NAME ;              ///< \brief string to describe 0 exit value 
	static const char* const SIGNAL_EXIT_NAME ;          ///< \brief string to describe signal exited value
	static const int TEST_BASE_VALUE ;                   ///< \brief first value for test manager exit codes
	static const int TEST_MAX_VALUE ;                    ///< \brief last value for test manager exit codes
	static const int TERMINATION_WAIT ;                  ///< \brief wait time before deciding a termination signal did not work
	static Process *s_instance ;                         ///< \brief singleton instance
public:
	static const char* exit_text(int return_value) ;     ///< \brief the textual description of standard exit codes
	static std::string exit_pretty(int return_value) ; 
	static const Process *instance() throw() ;
	static void set_name(const std::string &name) throw() ; 
	Process();                                           ///< \brief Builds a process representing the current process
	Process(const Process &other) ;                      ///< \brief copy constructor 
	Process(pid_t pid) ;                                 ///< \brief Builds a process with process id \c pid
	Process(pid_t pid, const std::string &name);         ///< \brief Builds a process with process id and process name 
	~Process() throw() ;                                 ///< \brief Destructor         
	operator pid_t() const throw() ;                     ///< \brief Cast operator to pid 
	int join(bool throw_non_zero=false) const ;          ///< \brief Waits for the process to terminate
	void signal(int signal) const ;                      ///< \brief Sends a signal to the process
	bool exists() const ;                                ///< \brief Does the process exist? 
	bool equals(const Process &other) const throw() ;    ///< \brief Comparison method 
	void terminate() const ;                             ///< \brief Terminate the process 
	pid_t process_id() const throw();                    ///< \brief the id of the process
	std::string to_string() const throw();               ///< \brief String conversion method 
    } ; // Process
    
} // OksSystem

std::ostream& operator<<(std::ostream& stream, const OksSystem::Process &proc) ;        ///< \brief STL stream operator */
bool operator ==(const OksSystem::Process &a, const OksSystem::Process &b)  throw(); 
bool operator !=(const OksSystem::Process &a, const OksSystem::Process &b)  throw(); 

#endif
