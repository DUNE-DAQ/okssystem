/*
 *  Executable.h
 *  System
 *
 *  Created by Matthias Wiesmann on 06.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_EXECUTABLE
#define SYSTEM_EXECUTABLE

#include <map>
#include <vector>

#include "system/File.h"
#include "system/Process.h"

namespace System {
    
    /** This class represents an executable file, that is a program. 
      * The class offers facilities to launch the program with given parameters
      * environnement variables
      * \author Matthias Wiesmann
      * \version 1.0
      * \brief Wrapper for executable file manipulation. 
      */
    
    class Executable : public File {
	
public:
	typedef std::vector<std::string> param_collection ; 
	typedef std::map<std::string, std::string> env_collection ; 
protected:
	void exec(char** argv) const ;                        /**< \brief does the actual \c exec */
	void exec(char** const argv, char** const env) const ;/**< \brief does the actual \c exec setting the right environment for the child process*/
	static const char* const SHELL_COMMAND ;              /**< \brief command to execute in a shell */
	static const char* const SHELL_COMMAND_PARAM ;        /**< \brief parameter to execute in a shell */
	static void copy_fd(int fd, std::ostream &target) ;   /**< \brief copies the content of a file descriptor into a STL stream */
public:
	static std::string system(const std::string &command);                 /**< \brief execute a command in a shell */

	Executable(const System::File &file) ; 
	Executable(const char* filename) ;               
	Executable(const std::string &filename) ;
	~Executable() ;
	
	void exec() const ;                                                           /**< \brief run the executable */
	void exec(const param_collection &params) const ;                             /**< \brief run the executable  */ 
	std::string pipe_in(const param_collection &params) const ;                   /**< \brief run the executable and pipe results back */
	std::string pipe_in(const param_collection &params, const env_collection &envs) const ; 
	Process pipe_out(const param_collection &params, const File &input_file, const File &output_file, const File &error_file, mode_t perm) const ; 
	Process pipe_out(const param_collection &params, const env_collection &envs, const File &input_file, const File &output_file, const File &error_file, mode_t perm) const ;
	void exec(const param_collection &params, const env_collection &envs) const ; /**< \brief run the executable  */
	Process start(const param_collection &params) const ;                 /**< \brief start the executable in another process */
	Process start_and_forget(const param_collection &params) const ;     /**< \brief start the executable in another process; do not wait for termination of child */
	Process start(const param_collection &params, const env_collection &envs) const ; /**< \brief start the executable in another process */
	
	std::string to_string(const param_collection &params) const ;  /**< \brief converts executable name and a parameter sequence into a string */
	
    } ; // Executable
    
} // System

#endif

