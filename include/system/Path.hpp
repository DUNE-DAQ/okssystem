/*
 *  Path.h
 *  Test
 *
 *  Created by Matthias Wiesmann on 09.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_PATH_CLASS
#define SYSTEM_PATH_CLASS


#include "system/File.hpp"

namespace System {
    
    /** This class represents an ordered set of directories.
      * The main goal of this class is to proved tools to find a file in this set. 
      * \brief Path list handling mechanism 
      * \author Matthias Wiesmann
      * \version 1.0
      */
    
    class Path {
	
protected:
	File::file_list_t m_directories ;                         /**< \brief list of directories */
	void parse_path_list(const std::string &path_list);       /**< \brief parse string containing path */
public:
	static const char PATH_SEPARATOR ;                        /**< \brief char used as separator in strings (semi-colon)*/
	Path(); 
	Path(const Path& other); 
	Path(const std::string &path_list) ;                      /**< \brief contructor with a string */
	operator std::string() const ; 
	void add(const System::File &dir);                        /**< \brief add a directory to the path */
	System::File which(const std::string &name) const ;       /**< \brief resolve a name in the path */
	void write_to(std::ostream &stream) const ;               /**< \brief displays the path in a stream */
	std::string to_string() const ;                           /**< \brief converts path into a string */
    } ; // Path
} // System

std::ostream& operator<<(std::ostream& stream, const System::Path& path);

#endif
