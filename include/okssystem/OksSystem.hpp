/*
 *  System.h
 *  System
 *
 *  Created by Matthias Wiesmann on 26.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

/** \file System.h This file includes all the main headers of the System package.
  * It does not declare anything <em>per se</em>, but is simply meant to be a convenient 
  * way to include the main functionality of the package. 
  * \author Matthias Wiesmann
  * \version 1.0
  */

#include "system/File.hpp"
#include "system/Executable.hpp"
#include "system/Process.hpp"
#include "system/MapFile.hpp"
#include "system/Environment.hpp"
#include "system/User.hpp"
#include "system/Host.hpp"
#include "system/Path.hpp"
#include "system/Descriptor.hpp"

/** \page Sys_package The System package
  The System package contains C++ wrappers for POSIX functions and general utility classes. 
  The objects all use safe types (i.e STL strings) and throw ERS issues in case of error. 
  The goal is to have Drop-in remplacements for often used calls (like the \c system call). 
  The package also offers ERS Issues to represent POSIX error code (i.e the content or errno).
  \section File File Classes
  The file classes offer tools to manipulate safely files, this includes:
  \li file name and path manipulation (finding canonical path, short name, extension, parent directory)
  \li file manipulation (creation of files, directory, removing of files, symbolic links). 
  \li creation of stream of file-descriptors 
  
  System::Executable is a subclass of System::File that implements functionalities 
  to manipulate executable file and start them with different parameters. 

  System::MapFile is a subclass of System::File that implements functionalities 
  to manipulate memory mapped files. 

  \see System::File
  \see System::MapFile
  \see System::Executable

  \section Envs Environment Variables
  
  The System::Environment class gives utility method to set and get the content of environment 
  variables safely. 
  \see System::Environment

  \section Process Process 
  
  The System::Process class encapsulates a running Unix process. 
  It offers methods to send it signals and wait for its termination. 
  \see System::Process
 
  \section Descriptor 
  The System::Descriptor class offers method to manipulate a Unix file-descriptor / socket. 
  \see System::Descriptor 

  \section Host Host

  The System::Host class gives tools to manipulate hostnames it offers the following features:
  \li hostname to ip translation
  \li hostname to fully qualified hostname
  \li ip to hostname
  
  The LocalHost subclass also gives basic tool to query local host information, like 
  \li operating system information
  \li hardware plateform
  \li machine description

  \see System::Host
  \see System::LocalHost
  
  \section User User
  The System::User class gives tool to manipulate user information, like resolving 
  UIDs to names, find the home directory, etc. 
  \see System::User
  
  \section ersStream ERS streams
  The System package offers a certain number of additional streams for the ERS system.
  The two main area are streams to write into files and STL streams and the syslog
  stream. 
  \see System::STLStream
  \see System::SyslogStream
 
*/

