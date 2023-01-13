/*
 *  Environment.h
 *  OksSystem
 *
 *  Created by Matthias Wiesmann on 11.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef OKSSYSTEM_ENVIRONMENT
#define OKSSYSTEM_ENVIRONMENT

#include <string>
#include <map>

namespace OksSystem {
    
    /** This class contains utility methods to handle environnement variables
      * \author Matthias Wiesmann
      * \version 1.0
      * \brief Wrapper for environnement variable manipulation. 
      */
    
    struct Environment {
	static std::string get(const std::string &key);				/**< \brief get a environnement variable */
	static void set(const std::string &key, const std::string & value);	/**< \brief sets an environnement variable */
	static void set(const std::map<std::string,std::string> & values);	/**< \brief sets a collection of variables */
    } ; // Environment
} // OksSystem

#endif

