/*
 *  Path.cxx
 *  Test
 *
 *  Created by Matthias Wiesmann on 09.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */
#include <sstream>
#include <iostream>

#include "system/Path.h"
#include "system/exceptions.h"

const char System::Path::PATH_SEPARATOR = ':';

/** Empty constructor
  */

System::Path::Path() {} 

/** Copy constructor
  * \param other the original object
  */

System::Path::Path(const Path& other) {
    m_directories = other.m_directories;
}

/** Constructor 
  * \param path_list string containing the list of directories
  */

System::Path::Path(const std::string &path_list) {
    parse_path_list(path_list); 
} // Path

System::Path::operator std::string() const {
    return to_string(); 
} // operator std::string()



/** Add a directory to the path 
  * \param dir the file to add
  */

void System::Path::add(const System::File &dir) {
    m_directories.push_back(dir); 
} // add


/** Parses a string containing the path separated by semi-colons.
  * \param path_list the path
  */

void System::Path::parse_path_list(const std::string &path_list)  {
    std::string rest = path_list;
    while(! rest.empty()) {
	std::string::size_type semi_colon = rest.find(PATH_SEPARATOR);
	std::string name;
	
	if(semi_colon==std::string::npos) {
	    name = rest;
	    rest.clear(); 
	} else {
	    name = rest.substr(0,semi_colon);
	    rest = rest.substr(semi_colon+1); 
	}

	if(!name.empty()) {
	  const System::File directory(name); 
	  add(directory);
	}
    } // while
} // parse_path_list

/** Prints the path into a stream
  * \param stream destination stream
  */

void System::Path::write_to(std::ostream &stream) const {
    bool first = true;
    for(File::file_list_t::const_iterator pos = m_directories.begin(); pos!=m_directories.end();pos++) {
	if (!first) {
	    stream << PATH_SEPARATOR;
	} else {
	    first = false;
	} 
	stream  << pos->full_name();
    } // for
} // write_to

/** Prints the path into a string
  * \return string containing description
  * \see write_to()
  */

std::string System::Path::to_string() const {
    std::ostringstream stream;
    write_to(stream);
    return stream.str(); 
} // to_string

/** Finds the first occurence of a named file in the path.
  * \param name the name of the file to search for
  * \return a File object representing the file 
  * \exception EntityNotFoundIssue if no matching file is found
  */

System::File System::Path::which(const std::string &name) const {
    for(File::file_list_t::const_iterator pos = m_directories.begin(); pos!=m_directories.end();pos++) {
	File child = pos->child(name);
	if (child.exists()) return child;
    } // for
    throw System::NotFoundIssue( ERS_HERE, name.c_str() ); 
} // which 

/** STL output operator
  * \param stream destination stream 
  * \param path the path to print
  */

std::ostream& operator<<(std::ostream& stream, const System::Path& path) {
    path.write_to(stream);
    return stream;
} // operator<<


