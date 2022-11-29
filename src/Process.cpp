/*
 *  Process.cxx
 *  System 
 *
 *  Created by Matthias Wiesmann on 18.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */
 
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <sysexits.h>

#include <iostream>
#include <sstream>

#include "ers/ers.hpp"

#include "system/exceptions.hpp"
#include "system/Process.hpp"


const int System::Process::TEST_BASE_VALUE = 182;/**< Lowest value for test manager result code  */
const int System::Process::TEST_MAX_VALUE = 186; /**< Highest value for test manager result code */

const int System::Process::TERMINATION_WAIT = 100000; /**< Wait time before deciding a termination signal did not work (in nanoseconds) */

const char * const System::Process::SYS_EXITS_NAMES[] = { "command line usage error", "data format error", "cannot open input", "addressee unknown", "host name unknown", "service unavailable", "internal software error", "system error", 
    "critical OS file missing", "can't create (user) output file", "input/output error", "temp failure; user is invited to retry", "remote error in protocol", "permission denied ", "configuration error" };
const char * const System::Process::TEST_EXITS_NAMES[] = { "undefined test", "test failed", "test unresolved", "test untested", "unsupported test" };
const char * const System::Process::OK_EXIT_NAME = "ok";

System::Process *System::Process::s_instance = 0;

/** Gives textual representation of exit values. 
  * This method recognises return codes from \c sysexits and the values returned by the text-manager 
  * \param return_value the integer returned by a program
  * \return a pointer to the string desribing the code, or a null pointer if the code is not recognised
  */

const char* System::Process::exit_text(int return_value) {
    if ( 0==return_value) return OK_EXIT_NAME;
    if ((return_value >= EX__BASE)  && (return_value <= EX__MAX)) return SYS_EXITS_NAMES[return_value-EX__BASE];
    if ((return_value >= TEST_BASE_VALUE) && (return_value <= TEST_MAX_VALUE)) return TEST_EXITS_NAMES[return_value-TEST_BASE_VALUE];
    return 0;
} // return_text

/** Builds a pretty string for an exit value 
  * \param return_value the integer returned by a program
  * \return if the code is recognised by \c exit_text, this text and the number between parenthesis 
  *         if not, the \c return_value converter into a string 
  */

std::string System::Process::exit_pretty(int return_value) {
    std::ostringstream stream;
    const char* text = System::Process::exit_text(return_value);
    if (text) {
	stream << text << '(' << return_value << ')';
    } else {
	stream << return_value ;
    }
    return stream.str(); 
} // exit_string


/** Default current process instance 
  * If needed the instance is created 
  * \return singleton instance for current process
  */

const System::Process *System::Process::instance() throw() {
    if (0==s_instance) {
	s_instance = new System::Process(); 
    } // if
    return s_instance;
} // instance


/** Sets the process name of the current process 
  */

void System::Process::set_name(const std::string &name) throw() {
    (void) instance();
    s_instance->m_process_name = name;
} // set_name


/** Constructor from current process 
 *  For the current process, the singleton factory method \c instance should be used 
 */

System::Process::Process() {
    m_process_id = ::getpid();
} // Process

/** Copy constructor 
  * \param other original instance 
  */

System::Process::Process(const Process &other) {
    m_process_id = other.m_process_id;
    m_process_name = other.m_process_name;
} // Process

/** Constructor for pid
  * \param id pid of the process
  */

System::Process::Process(pid_t id){
    m_process_id = id;
} // Process 

/** Constructor from pid and process name 
  * \param id pid of the process 
  * \param name display name of the process 
  */

System::Process::Process(pid_t id, const std::string &name) {
    m_process_id = id;
    m_process_name = name;
} 


/** Destructor 
  */

System::Process::~Process() throw() {
    m_process_id = 0;
} // ~Process

/** Cast operator - converts to process-id 
  */

System::Process::operator pid_t() const throw() {
    return m_process_id;
} // operator pid_t


/** Waits for a process to terminate. 
  * If the process terminates normally, the return value is the termination status of the process. 
  * If the process stops or is signaled, an SignalIssue is thrown. 
  * \return the termination status of the process 
  */

int System::Process::join(bool throw_non_zero) const {
    ERS_PRECONDITION(! equals(*instance())); 
    int status;
    errno = 0;
    pid_t pid = ::waitpid(m_process_id,&status,0); 
    if (pid!=m_process_id) {
        std::string message = "on process " + this->to_string();
	throw System::SystemCallIssue( ERS_HERE, errno, "waitpid", message.c_str() ); 
    } // if 
    if (WIFEXITED(status)) {
	int exit_status = WEXITSTATUS(status);
	if ((throw_non_zero==false) || (exit_status==0)) return exit_status;
	throw System::TerminationIssue(ERS_HERE, errno, exit_status);
    } // exit status
    if (WIFSIGNALED(status)) throw System::SignalIssue(ERS_HERE,errno,WTERMSIG(status));
    if (WIFSTOPPED(status)) throw System::SignalIssue(ERS_HERE,errno,WSTOPSIG(status));
    return WEXITSTATUS(status);
} // join

/** Sends a signal to the process 
  * \param signal_number number of the signal 
  */

void System::Process::signal(int signal_number) const {
    const int status = ::kill(m_process_id,signal_number);
    if (status < 0) {
      std::string message = "on process " + this->to_string();
      throw System::SystemCallIssue(ERS_HERE, errno, "kill", message.c_str());
    }
} // signal

/** Checks if the process actually exists on the host
  * \return true if the process exists 
  * \note currently the check is done by sending signal 0 to the process
  *       in case of failure, we know process does not exist 
  */

bool System::Process::exists() const {
    const int status = ::kill(m_process_id,0);
    return (status>=0);
} // exists

/** Checks if two process are equal 
  * \param other the process to compare this process to
  * \return true if both proceses are equal (same pid).
  */

bool System::Process::equals(const Process &other) const throw() {
    return m_process_id == other.m_process_id;
} // equals

/** Terminates a process 
  * Termination is implemented by
  * \li sending the TERM signal
  */

void System::Process::terminate() const {
  /*
    struct timespec wait_time;
    struct timespec remain_time;
    wait_time.tv_sec = 0;
    wait_time.tv_nsec = TERMINATION_WAIT;
  */
    if (exists()) {
	signal(SIGTERM);
    }
    return;
} 

/* This is a piece of crap 
    nanosleep(&wait_time,&remain_time); 
    if (exists()) {
	signal(SIGQUIT);
    } else {
	return;
    } //
    nanosleep(&wait_time,&remain_time); 
    if (exists()) {
	signal(SIGKILL);
    } // exists
} // terminate
*/
/** Builds a string description of the process 
  * \return textual description
  */

std::string System::Process::to_string() const throw() {
    std::ostringstream stream;
    if (! m_process_name.empty()) {
	stream << m_process_name << ' ';
    } 
    stream << "pid: " << m_process_id;
    return stream.str(); 
} // to_string

/** \return the process id for the process 
  */

pid_t System::Process::process_id() const throw() { return m_process_id;} 

/** Streaming operator
  * \param out destination stream
  * \param proc the process to stream 
  * \see Process::to_string()
  */

std::ostream& operator<<(std::ostream& out, const System::Process &proc) {
    out << proc.to_string();
    return out;
} // operator<<

/** Comparison operator 
  * \param a first process to compare
  * \param b second process to compare 
  * \return true if they are equal 
  * \see System::Process::equals
  */

bool operator ==(const System::Process &a, const System::Process &b)  throw() {
    return a.equals(b);
} // operator ==

/** Comparison operator 
  * \param a first process to compare
  * \param b second process to compare 
  * \return false if they are equal 
  * \see System::Process::equals
  */

bool operator !=(const System::Process &a, const System::Process &b)  throw() {
    return ! a.equals(b); 
} // operator !=


