/*
 *  OksSystem.h
 *  OksSystem
 *
 *  Created by Matthias Wiesmann on 26.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

/** \file OksSystem.h This file includes all the main headers of the OksSystem package.
  * It does not declare anything <em>per se</em>, but is simply meant to be a convenient 
  * way to include the main functionality of the package. 
  * \author Matthias Wiesmann
  * \version 1.0
  */

#include "okssystem/File.hpp"
#include "okssystem/Executable.hpp"
#include "okssystem/Process.hpp"
#include "okssystem/MapFile.hpp"
#include "okssystem/Environment.hpp"
#include "okssystem/User.hpp"
#include "okssystem/Host.hpp"
#include "okssystem/Path.hpp"
#include "okssystem/Descriptor.hpp"

/** \page Sys_package The OksSystem package
  The OksSystem package contains C++ wrappers for POSIX functions and general utility classes. 
  The objects all use safe types (i.e STL strings) and throw ERS issues in case of error. 
  The goal is to have Drop-in remplacements for often used calls (like the \c okssystem call). 
  The package also offers ERS Issues to represent POSIX error code (i.e the content or errno).
  \section File File Classes
  The file classes offer tools to manipulate safely files, this includes:
  \li file name and path manipulation (finding canonical path, short name, extension, parent directory)
  \li file manipulation (creation of files, directory, removing of files, symbolic links). 
  \li creation of stream of file-descriptors 
  
  OksSystem::Executable is a subclass of OksSystem::File that implements functionalities 
  to manipulate executable file and start them with different parameters. 

  OksSystem::MapFile is a subclass of OksSystem::File that implements functionalities 
  to manipulate memory mapped files. 

  \see OksSystem::File
  \see OksSystem::MapFile
  \see OksSystem::Executable

  \section Envs Environment Variables
  
  The OksSystem::Environment class gives utility method to set and get the content of environment 
  variables safely. 
  \see OksSystem::Environment

  \section Process Process 
  
  The OksSystem::Process class encapsulates a running Unix process. 
  It offers methods to send it signals and wait for its termination. 
  \see OksSystem::Process
 
  \section Descriptor 
  The OksSystem::Descriptor class offers method to manipulate a Unix file-descriptor / socket. 
  \see OksSystem::Descriptor 

  \section Host Host

  The OksSystem::Host class gives tools to manipulate hostnames it offers the following features:
  \li hostname to ip translation
  \li hostname to fully qualified hostname
  \li ip to hostname
  
  The LocalHost subclass also gives basic tool to query local host information, like 
  \li operating okssystem information
  \li hardware plateform
  \li machine description

  \see OksSystem::Host
  \see OksSystem::LocalHost
  
  \section User User
  The OksSystem::User class gives tool to manipulate user information, like resolving 
  UIDs to names, find the home directory, etc. 
  \see OksSystem::User
  
  \section ersStream ERS streams
  The OksSystem package offers a certain number of additional streams for the ERS okssystem.
  The two main area are streams to write into files and STL streams and the syslog
  stream. 
  \see OksSystem::STLStream
  \see OksSystem::SyslogStream
 
*/

