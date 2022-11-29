/*
 *  Environment.cxx
 *  ers
 *
 *  Created by Matthias Wiesmann on 11.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <stdlib.h>

#include "system/Environment.hpp"
#include "system/exceptions.hpp"

using namespace System;

/** Sets an environnement variable 
  * \param key the name of the variable
  * \param value the value of the variable 
  * \exception PosixIssue unable to set environnement variable 
  */

void System::Environment::set(const std::string &key, const std::string &value) {
    const char* c_key = key.c_str();
    const char* c_value = value.c_str();
    const int status = ::setenv(c_key,c_value,1);
    if (status<0) {
      std::string message = "while setting " + key + " to " + value;
      throw SystemCallIssue( ERS_HERE, errno, "setenv", message.c_str());
    }
} // set

/** Sets a collection of environnement variables. 
 * \param values The  map of string string pairs containing the name / values pairs 
 * \exception PosixIssue unable to set environnement variable 
 */

void System::Environment::set(const std::map<std::string,std::string> &values) {
    for(std::map<std::string,std::string>::const_iterator pos = values.begin();pos!=values.end();++pos) {
	set(pos->first,pos->second); 
    } // for
} // set

/** Gets an environnement variable 
  * \param key the name of the variable 
  * \return the value of the variable, of the string \c NO_VALUE if the variable is unknown. 
  */

std::string System::Environment::get(const std::string &key) {
    const char* c_value = ::getenv(key.c_str());
    if (c_value==0) return std::string();
    return std::string(c_value);
} // get
 
