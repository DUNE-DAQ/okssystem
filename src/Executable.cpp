/*
 *  Executable.cxx
 *  ers
 *
 *  Created by Matthias Wiesmann on 06.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdlib>
#include <unistd.h>
#include <cstdio>
#include <sstream>
#include <iostream>

#include "ers/Assertion.hpp"
#include "ers/ers.hpp"

#include "okssystem/Environment.hpp"
#include "okssystem/Executable.hpp"
#include "okssystem/Process.hpp"
#include "okssystem/Descriptor.hpp"
#include "okssystem/exceptions.hpp"

const char* const OksSystem::Executable::SHELL_COMMAND = "/bin/sh";
const char* const OksSystem::Executable::SHELL_COMMAND_PARAM = "-c";

/** This method is a safe replacement for the okssystem function.
  * It basically offers the same functionality with better error handling.
  * The command is executed in a shell via \c /bin/sh \c -c. 
  * If the command is successfull (return value 0), this method returns the resulting string.  
  * If the command fails, this method throws an exception of type \c okssystem::ExecutionIssue.
  * This exception contains the return code and the content of stderr. 
  * \param command the command to execute 
  * \return the content of the stdout stream for the executed command 
  * \see pipe_in()
  */ 


std::string OksSystem::Executable::okssystem(const std::string &command) {
    OksSystem::Executable shell(SHELL_COMMAND);
    std::vector<std::string> params;
    params.push_back(SHELL_COMMAND_PARAM);
    params.push_back(command);
    return shell.pipe_in(params); 
} // std::string

OksSystem::Executable::Executable(const OksSystem::File &file) : OksSystem::File(file) {}
OksSystem::Executable::Executable(const char* filename) : OksSystem::File(filename) {}
OksSystem::Executable::Executable(const std::string &filename) : OksSystem::File(filename) {}
OksSystem::Executable::~Executable() {}


/** Core execution method. 
  * This method assumes the datastructures have been setup correctly for \c execv.
  * It basically adds some error handling to \c execv
  * \param argv array of arguments, with name of the executable in first position
  * \note this method should not be called directly 
  */

void OksSystem::Executable::exec(char** argv) const {
    ERS_PRECONDITION(argv);
    ERS_PRECONDITION(argv[0]);
    const int status = ::execv(argv[0],argv);
    throw OksSystem::ExecutionIssue( ERS_HERE, errno, argv[0], status ); 
} // exec

/** Core execution method. 
  * This method assumes the datastructures have been setup correctly for \c execve.
  * It basically adds some error handling to \c execve.
  * Look at the \c execve man pages for details.
  * \param argv array of arguments, with name of the executable in first position
  * \param env array containing the environment the process will be started with
  */
void OksSystem::Executable::exec(char** const argv, char** const env) const {
    ERS_PRECONDITION(argv);
    ERS_PRECONDITION(argv[0]);
    ERS_PRECONDITION(env);
    const int status = ::execve(argv[0],argv,env);
    throw OksSystem::ExecutionIssue( ERS_HERE, errno, argv[0], status );   
}

/** Simple execution method. 
  * This methods converts the vector of strings into the correct data structures for \c execv. 
  * The \c argv structure is allocated dynamically and new string copied into it. 
  */

void OksSystem::Executable::exec(const param_collection &params) const {
    const int argc = params.size(); // number of parameters
    const int argclen = argc+2;   // size of array parameters + program name + null pointer
    char **argv = (char **) calloc(sizeof(char*),argclen);
    OKSSYSTEM_ALLOC_CHECK(argv,sizeof(char*)*argclen);
    const char* name = *this;
    argv[0] = strdup(name);
    OKSSYSTEM_ALLOC_CHECK(argv[0],strlen(name));
    for(int i=0;i<argc;i++) {
	argv[i+1] = strdup(params[i].c_str());
    } // loop over args
    argv[argc+1] = 0;
    try {
	exec(argv); 
    } 
    catch(OksSystem::ExecutionIssue &ex) {
      for(int i=0;argv[i]!=0;i++) {
    	free(argv[i]); 
      } // for
      free(argv);
      throw;
    }
    catch (ers::Issue &issue) { // there was a problem so we deallocate the argc array. 
      for(int i=0;argv[i]!=0;i++) {
	free(argv[i]); 
      } // for
      free(argv);
      throw;
    } // catch
} // exec 

/** This execution method calls the 'execve' function executing the 
  * \param params the parameters to use when launching the executable
  * \param envs table of the environnement variables set for the executable
  */

void OksSystem::Executable::exec(const param_collection &params, const env_collection &envs) const {

  // Elaborate the environment
  const unsigned int envArraySize = envs.size() + 1; // The last elements must be NULL
  char** const env = new char*[envArraySize];

  {
    const env_collection::const_iterator b = envs.begin();
    const env_collection::const_iterator e = envs.end();
    env_collection::const_iterator it;
    
    unsigned int counter = 0;
    for(it = b; it != e; ++it) {
      const std::string entry = it->first + "=" + it->second;
      env[counter] = new char[entry.size() + 1]; // Add the NULL terminator
      ::strcpy(env[counter], entry.c_str());
      ++counter;
    }
    
    env[envArraySize - 1] = (char*) 0;
  }

  // Elaborate the paramenters
  const unsigned int paramArraySize = params.size() + 2; // Add the executable name and the NULL terminator
  char** const par = new char*[paramArraySize]; 

  {
    const std::string& binName = this->full_name();
    par[0] = new char[binName.size() + 1]; // Add the NULL terminator
    ::strcpy(par[0], binName.c_str());

    const param_collection::const_iterator b = params.begin();
    const param_collection::const_iterator e = params.end();
    param_collection::const_iterator it;

    unsigned int counter = 1;
    for(it = b; it != e; ++it) {
      const std::string& value = *it;
      par[counter] = new char[value.size() + 1]; // Add the NULL terminator
      ::strcpy(par[counter], value.c_str());
      ++counter;
    }

    par[paramArraySize - 1] = (char*) 0;
  }

  try {
    exec(par, env);
  }
  catch(OksSystem::ExecutionIssue& ex) {
    // if we are here it means that the exec call failed!!!
    // Free the allocated memory
    for(unsigned int i = 0; i < envArraySize; ++i) {
      delete[] env[i];
    }
    delete[] env;
    
    for(unsigned int i = 0; i < paramArraySize; ++i) {
      delete[] par[i];
    }
    delete[] par;
  
    // Re-throw the exception
    throw ex;
  }

}


/** This method starts the executable in another process using \c fork 
  * \param params the parameters to use when launching the executable
  * \return A process object representing the started process
  */

OksSystem::Process OksSystem::Executable::start(const param_collection &params) const {

  // Block all signals before fork()
  sigset_t new_set;
  sigset_t old_set;
  sigfillset(&new_set);
  pthread_sigmask(SIG_SETMASK, &new_set, &old_set);

  const pid_t child_id = fork();
  if (0 == child_id) { // we are the child
    
    // Put some signals to their default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    
    // Restore the original signal mask in child
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);
    
    try {
      exec(params);
    }
    catch(OksSystem::ExecutionIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(ers::Issue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    } 
    
  } // we are the child 
  if (child_id > 0) { // we are the parent
    
    // Restore the original signal mask in parent
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);
    
    return Process(child_id,to_string(params));
    
  } // we are the parent
  
  // Restore the original signal mask in case of a fork() failure
  pthread_sigmask(SIG_SETMASK, &old_set, NULL);
  
  throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "fork", "" );// We are screwed up 
} // start


/** This method starts the executable in another process using \c fork. The parent will not wait on the child termination.
  * \param params the parameters to use when launching the executable
  * \return A process object representing the started process
  */

OksSystem::Process OksSystem::Executable::start_and_forget(const param_collection &params) const {
  
  // Block all signals before fork()
  sigset_t new_set;
  sigset_t old_set;
  sigfillset(&new_set);
  pthread_sigmask(SIG_SETMASK, &new_set, &old_set);

  signal(SIGCLD, SIG_IGN);  /* now I don't have to wait()! */

  const pid_t child_id = fork();
  if (0 == child_id) { // we are the child

    // Put some signals to their default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGCLD, SIG_DFL); /* Restore default SIGCLD handling in the child process */ 

    // Restore the original signal mask in child
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);

    try {
      exec(params);
    }
    catch(OksSystem::ExecutionIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(ers::Issue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }

  } // we are the child 
  if (child_id > 0) { // we are the parent

    // Restore the original signal mask in parent
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);

    return Process(child_id,to_string(params));
  } // we are the parent

  // Restore the original signal mask in case of a fork() failure
  pthread_sigmask(SIG_SETMASK, &old_set, NULL);

  throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "fork", "" );// We are screwed up 
} // start


/** Copies the content of a file descriptor into a STL stream
  * \param fd the source file descriptor
  * \param target the target stream
  * \note a more efficient version could use the underlying STL buffer
  */

void OksSystem::Executable::copy_fd(int fd, std::ostream &target) {
    while(true) {
	char buffer[256];
	long status = read(fd,buffer,sizeof(buffer));
	if (status <= 0) return;
	ERS_ASSERT( status<=(long) sizeof(buffer) ); 
	for(int i=0;i<status;i++) {
	    target << (char) buffer[i];
	} // for
    } // while
} // copy_fd 

/** \overload */

std::string OksSystem::Executable::pipe_in(const param_collection &params) const {

    int input_pipe[2];
    int data_pipe[2];
    int error_pipe[2];
    const int input_pipe_status = pipe(input_pipe);
    if (input_pipe_status<0) throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "pipe", "" );
    const int data_pipe_status = pipe(data_pipe);
    if (data_pipe_status<0) throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "pipe", "" );
    const int error_pipe_status = pipe(error_pipe);
    if (error_pipe_status<0) throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "pipe", "" );

    // Block all signals before fork()
    sigset_t new_set;
    sigset_t old_set;
    sigfillset(&new_set);
    pthread_sigmask(SIG_SETMASK, &new_set, &old_set);

    const pid_t child_id = fork();
    if (0 == child_id) { // we are the child

      // Put some signals to their default
      signal(SIGTERM, SIG_DFL);
      signal(SIGINT, SIG_DFL); 

      // Restore the original signal mask in child
      pthread_sigmask(SIG_SETMASK, &old_set, NULL);

      dup2(input_pipe[1], fileno(stdin)); 
      dup2(data_pipe[1], fileno(stdout)); 
      dup2(error_pipe[1], fileno(stderr)); 
      
      try {
	this->exec(params);
      }
      catch(OksSystem::ExecutionIssue &ex) {
	ers::warning(ex);
	_exit(EXIT_FAILURE);
      }
      catch(ers::Issue &ex) {
	ers::warning(ex);
	_exit(EXIT_FAILURE);
      }
      
    } // we are the child 
    if (child_id > 0) { // we are the parent

      // Restore the original signal mask in parent
      pthread_sigmask(SIG_SETMASK, &old_set, NULL);

      Process child_process(child_id,to_string(params)); 
      close(input_pipe[1]);
      close(data_pipe[1]);
      close(error_pipe[1]);
      const int child_status = child_process.join();
      const int in_fd = input_pipe[0];
      const int err_fd = error_pipe[0];
      const int out_fd = data_pipe[0];
      std::ostringstream in_stream;
      std::ostringstream out_stream;
      std::ostringstream err_stream;
      copy_fd(in_fd,in_stream); 
      copy_fd(out_fd,out_stream); 
      copy_fd(err_fd,err_stream); 
      close(in_fd);
      close(out_fd);
      close(err_fd); 
      if (0==child_status) {
	return out_stream.str(); 
      } // if
      std::string command = to_string(params); 
      std::string error_str = err_stream.str(); 
      throw OksSystem::ExecutionIssue(ERS_HERE,errno,command.c_str(),child_status);
    } // we are the parent

    // Restore the original signal mask in case of a fork() failure
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);

    return std::string();
} // pipe_in

/** This method executes the process and pipes stdout and stderr back and waits for execution termination.
 * If the return value is zero then the content of stdout is copied into a string and returned.
 * In case of a non-zero value, an exception is thrown. 
 * This exception contains the return code and the content of stderr. 
 * \return content of stdout
 * \param params the parameters to the executable
 * \param envs the environnement variables to set
 * \throw OksSystem::ExecutionIssue if command execution did not return 0.
 * \throw OksSystem::ExecFail if the exec okssystem call failed.
 * \throw OksSystem::PipeIssue if the pipe okssystem call failed. 
 */

std::string OksSystem::Executable::pipe_in(const param_collection &params, const env_collection &envs) const {
    OksSystem::Environment::set(envs);
    return pipe_in(params); 
} // pipe_in

/** \overload */

OksSystem::Process OksSystem::Executable::pipe_out(const param_collection &params, const File &input_file, const File &output_file, const File &error_file,mode_t perm) const {

  // Block all signals before fork()
  sigset_t new_set;
  sigset_t old_set;
  sigfillset(&new_set);
  pthread_sigmask(SIG_SETMASK, &new_set, &old_set);

  OksSystem::Descriptor in(&input_file,OksSystem::Descriptor::flags(true,false),perm);   // May throw OksSystem::OpenFileIssue
  OksSystem::Descriptor out(&output_file,OksSystem::Descriptor::flags(false,true),perm);
  OksSystem::Descriptor err(&error_file,OksSystem::Descriptor::flags(false,true),perm);
  
  const pid_t child_id = fork();
  if (0 == child_id) { // we are the child

    // Put some signals to their default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL); 
    
    // Restore the original signal mask in child
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);
  
    try {
      ::dup2(in,::fileno(stdin));
      ::dup2(out,::fileno(stdout));
      ::dup2(err,::fileno(stderr));
      ::close(in.fd());
      ::close(out.fd());
      ::close(err.fd());      
      this->exec(params);
    }
    catch(OksSystem::ExecutionIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(OksSystem::PosixIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(ers::Issue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    
  } // we are the child
  if (child_id > 0) { // we are the parent

    // Restore the original signal mask in parent
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);

    return Process(child_id,to_string(params));
  } // we are the parent

  // Restore the original signal mask in case of a fork() failure
  pthread_sigmask(SIG_SETMASK, &old_set, NULL);

  throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "fork", "" );// We are screwed up 
} // pipe_out

/** Runs the executable and redirects the two output streams 
  * \param params the parameters for the executable 
  * \param envs the environnements for the executable
  * \param input_file the input file for stdin
  * \param output_file the output file for stdout
  * \param error_file the output file for stderr
  * \param perm permissions for both output streams 
  */

OksSystem::Process OksSystem::Executable::pipe_out(const param_collection &params, const env_collection &envs, const File &input_file, const File &output_file, const File &error_file, mode_t perm) const {

  // Block all signals before fork()
  sigset_t new_set;
  sigset_t old_set;
  sigfillset(&new_set);
  pthread_sigmask(SIG_SETMASK, &new_set, &old_set);

  OksSystem::Descriptor in(&input_file,OksSystem::Descriptor::flags(true,false),perm);   // May throw OksSystem::OpenFileIssue
  OksSystem::Descriptor out(&output_file,OksSystem::Descriptor::flags(false,true),perm);
  OksSystem::Descriptor err(&error_file,OksSystem::Descriptor::flags(false,true),perm);
  
  const pid_t child_id = fork();
  if (0 == child_id) { // we are the child

    // Put some signals to their default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL); 
    
    // Restore the original signal mask in child
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);
  
    try {
      ::dup2(in,::fileno(stdin));
      ::dup2(out,::fileno(stdout));
      ::dup2(err,::fileno(stderr));
      ::close(in.fd());
      ::close(out.fd());
      ::close(err.fd());
      this->exec(params,envs);
    }
    catch(OksSystem::ExecutionIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(OksSystem::PosixIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(ers::Issue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    
  } // we are the child
  if (child_id > 0) { // we are the parent

    // Restore the original signal mask in parent
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);

    return Process(child_id,to_string(params));
  } // we are the parent

  // Restore the original signal mask in case of a fork() failure
  pthread_sigmask(SIG_SETMASK, &old_set, NULL);

  throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "fork", "" );// We are screwed up

} //pipe_out

/** This method starts the executable in another process using \c fork 
  * \param params the parameters to use when launching the executable
  * \param envs table of the environnement variables set for the executable
  */

OksSystem::Process OksSystem::Executable::start(const param_collection &params, const env_collection &envs) const {

  // Block all signals before fork()
  sigset_t new_set;
  sigset_t old_set;
  sigfillset(&new_set);
  pthread_sigmask(SIG_SETMASK, &new_set, &old_set);
  
  const pid_t child_id = fork();
  if (child_id == 0) { // we are the child

    // Put some signals to their default
    signal(SIGTERM, SIG_DFL);
    signal(SIGINT, SIG_DFL); 

    // Restore the original signal mask in child
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);

    try {
      exec(params,envs);
    }
    catch(OksSystem::ExecutionIssue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    }
    catch(ers::Issue &ex) {
      ers::warning(ex);
      _exit(EXIT_FAILURE);
    } 
 
  } // we are the child 
  if (child_id > 0) { // we are the parent
    
    // Restore the original signal mask in parent
    pthread_sigmask(SIG_SETMASK, &old_set, NULL);
    
    return Process(child_id);
  } // we are the parent

  // Restore the original signal mask in case of a fork() failure
  pthread_sigmask(SIG_SETMASK, &old_set, NULL);

  throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "fork", "" );// We are screwed up 
} // start

/** Converts the executable name and a list of parameters into a string,
  * suitable for display. 
  * \param params the list of parameters 
  * \return a string with command name and parameters separated by spaces 
  */

std::string OksSystem::Executable::to_string(const param_collection &params) const {
    std::ostringstream stream;
    stream << m_full_name;
    for(param_collection::const_iterator pos=params.begin();pos!=params.end();++pos) {
	stream << " " << (*pos);
    } // for
    return stream.str(); 
} // to_string


