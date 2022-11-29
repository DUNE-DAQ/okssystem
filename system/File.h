/*
 *  File.h
 *  System
 *
 *  Created by Matthias Wiesmann on 04.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_FILE
#define SYSTEM_FILE

#include <string>
#include <vector>

#include <sys/types.h>
#include <sys/stat.h>

#include "system/User.h"

namespace System {
    
    /** This class represents a file.
      * it provides tools to manipulate files in a simple way. 
      * All methods throw ers issues in case of error. 
      * Internally, files are handled as canonical paths i.e with ./ ../ and symlinks resolved.
      * \author Matthias Wiesmann
      * \version 1.1
      * \brief Wrapper for file operations. 
      */
    
    class File {
public: 
	typedef std::vector<System::File>  file_list_t ; 	
protected:
	std::string m_full_name ;                                     ///< \brief full name (path) of the file */
	void set_name(const std::string &name);                       ///< \brief sets the name of the file */
	static const char * const HUMAN_SIZE_STR[] ;                  ///< \brief strings for pretty printing file sizes */
	static const char * const HUMAN_OPEN_STR[] ;                  ///< \brief strings for pretty printing open flags */
	static const int KILOBYTE ;                                   ///< \brief number of bytes in a kilobyte */
	static int unit(int u) ;                                      ///< \brief calculates the value of a computer unit of order n (i.e KB,GB, etc */
	mode_t get_mode() const ;                                     ///< \brief get mode associated with file (permission + type) */
	static const char * const FILE_COMMAND_PATH ; 
public:
        static const char * const FILE_FLAG_STR ;                     ///< \brief column headers for display of permissions */
	static const char * const FILE_PROTOCOL ;                     ///< \brief string for the file protocol */

	static std::string protocol(const std::string &url) throw();   ///< \brief extracts the protocol part of an url */
	static std::string extension(const std::string &url) throw();  ///< \brief extracts the extension of a path or an url */
	static std::string uri(const std::string &url) throw() ;       ///< \brief extracts the local path of an url */
	static std::string short_name(const std::string &url) throw(); ///< \brief extracts the short file name of an url */
	static int depth(const std::string &path) throw() ;            ///< \brief calculates depth of a path */
	    
	static std::string first_line(const std::string &text);       ///< \brief extracts the first line of a text */
	
	static File from_url(const std::string &url);                 ///< \brief build a file out of an URL */

	static std::string working_directory() ;                      ///< \brief current directory of process */
	static void working_directory(const File &dir);              ///< \brief set working directory of process */
	static std::string expand_home(const std::string path) ;      ///< \brief resolve home directory */
	static std::string pretty_permissions(mode_t permissions);    ///< \brief pretty prints permissions */
	static std::string to_string(mode_t permissions) throw() ;    ///< \brief converts permission to string */
	static std::string pretty_open_flag(int flags) ;              ///< \brief pretty prints open flags */
	static std::string pretty_size(size_t size, bool cut_small);  ///< \brief pretty prints a file size */
	    
	File(const std::string &name); 
	File(const char* name) ; 
	File(const File& other) ; 
	virtual ~File() {} 
	operator std::string() const throw() ; 
	operator const char* () const throw() ; 
	operator bool() const throw() ;     
	bool equals(const File &other) const throw() ;                ///< \brief compare two files */
	operator size_t() const ; 
	
	const std::string &full_name() const throw() ;                ///< \brief full name for file */
	const char* c_full_name() const throw() ; 
	std::string short_name() const throw() ;                      ///< \brief short name for file */
	std::string parent_name() const throw() ;                     ///< \brief path of directory containing file */
	std::string extension() const throw();                        ///< \brief extension for file */
	int depth() const throw() ;                                   ///< \brief depth of the file */
    
	System::File parent() const ;                                 ///< \brief parent of the current file */
	System::File child(const std::string &name) const ;           ///< \brief named child of the current directory */
	System::File temporary(const std::string &prefix) const ; 
	
	bool exists() const throw() ;                                 ///< \brief does the file exist */
	mode_t permissions() const ;                                  ///< \brief permissions for the file */
	std::string pretty_permissions() const ;                      ///< \brief pretty permissions for the file */
	size_t size() const ;                                         ///< \brief size of file */
	uid_t owner_id() const ;                                      ///< \brief owner id of file */
	User owner() const ;                                          ///< \brief owner of the file */
	gid_t group() const ;                                         ///< \brief group of file */
	bool is_regular() const ;                                     ///< \brief is the file a directory */
	bool is_directory() const ;                                   ///< \brief is the file a regular file */
	bool is_fifo() const ;                                        ///< \brief is the file a named pipe */
	std::string file_type() const ;                               ///< \brief type of the file */
	file_list_t directory() const ;                               ///< \brief list of file in directory */
	
	void unlink() const ;                                         ///< \brief deletes (unlinks) file */
	void rmdir() const ;                                          ///< \brief deletes directory */

	void remove() const ;                                         ///< \brief recursively delete files and directories */
	void rename(const File &other) const ;                        ///< \brief rename or moves the file */
	void permissions(mode_t permissions) const ;                  ///< \brief sets the type of the file */
	void make_dir(mode_t permissions) const ;                     ///< \brief creates a directory */
	void make_path(mode_t permissions) const ;                    ///< \brief creates a full path */
	void make_fifo(mode_t permissions) const ;                    ///< \brief creates a FIFO (named pipe) */
	
	void ensure_path(mode_t permissions) const ;                  ///< \brief creates the parent path */
	
	std::istream* input() const ;                                 ///< \brief returns an input stream from the file*/
	std::ostream* output(bool append=false) const ;               ///< \brief returns an output stream to the file*/
    } ; // File
} // System

std::ostream& operator<<(std::ostream& stream, const System::File& file);
bool operator ==(const System::File &a, const System::File &b)  throw(); 
bool operator !=(const System::File &a, const System::File &b)  throw(); 


#endif



