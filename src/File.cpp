/*
 *  File.cxx
 *  ers
 *
 *  Created by Matthias Wiesmann on 04.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <unistd.h>
#include <stdio.h>
#include <libgen.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <assert.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <iostream>
#include <sstream>
#include <fstream>
#include <pwd.h>

#include "ers/ers.hpp"

#include "okssystem/File.hpp"
#include "okssystem/exceptions.hpp"
#include "okssystem/Executable.hpp"
#include "okssystem/User.hpp"

#define SPACE_CHAR ' ' 

const char * const OksSystem::File::HUMAN_SIZE_STR[] = { "B", "KB", "MB", "GB" };
const char * const OksSystem::File::HUMAN_OPEN_STR[] = { "READ", "WRITE", "NOBLOCK", "APPEND", "CREATE", "TRUNCATE","EXCLUSIVE"  };
const char * const OksSystem::File::FILE_COMMAND_PATH = "/usr/bin/file";
const char * const OksSystem::File::FILE_FLAG_STR = "-rwxS";
const char * const OksSystem::File::FILE_PROTOCOL = "file";


#define READ_CHAR_CODE (OksSystem::File::FILE_FLAG_STR[1])
#define WRITE_CHAR_CODE (OksSystem::File::FILE_FLAG_STR[2])
#define EXECUTE_CHAR_CODE (OksSystem::File::FILE_FLAG_STR[3])
#define NOTHING_CHAR_CODE (OksSystem::File::FILE_FLAG_STR[0])
#define SETUID_CHAR_CODE (OksSystem::File::FILE_FLAG_STR[4])

#define SLASH_CHAR '/' 
#define COLON_CHAR ':'
#define DOT_CHAR '.' 

const int OksSystem::File::KILOBYTE = 1024;


// --------------------------------------
// Static Methods
// --------------------------------------


/** Extracts the protocol part of an url. 
  * The protocol part is everything until the colon (:)
  * \param url an url 
  * \return the protocol part of an empty string 
  */

std::string OksSystem::File::protocol(const std::string &url) throw() {
    std::string::size_type colon = url.find(COLON_CHAR);
    if (colon==std::string::npos) return std::string();
    return url.substr(0,colon);
} // protocol

/** Extracts the extension of an url or path 
  * \param url an url or file path 
  * \return the extension (without the dot) or an empty string 
  */

std::string OksSystem::File::extension(const std::string &url) throw() {
    std::string::size_type dot = url.rfind(DOT_CHAR);
    if (dot==std::string::npos) return std::string();
    return url.substr(dot+1);
} // extension

/** Extracts the short file name of an url or path 
  * \param url an url or file path 
  * \return the short name or the full url if it cannot be found 
  */

std::string OksSystem::File::short_name(const std::string &url) throw() {
    std::string::size_type slash = url.rfind(SLASH_CHAR);
    if (slash==std::string::npos) return url;
    return url.substr(slash+1);
} // extension

/** Extacts the uri part of an url.
  * The uri of an url is everything except the protocol part
  * In the case of \c file or \c http urls, the uri is the local path 
  * \param url an url
  * \return the uri or the full url if no colon in the string
  */

std::string OksSystem::File::uri(const std::string &url) throw() {
    std::string::size_type colon = url.find(COLON_CHAR);
    if (colon==std::string::npos) return url;
    return url.substr(colon+1);
} // uri


/** Calculates the depth of a file. 
  * This depth is the number of parent directories.
  * Files in the root directory have depht of 0, 
  * other files have a depth of their parent's depth + 1
  * \param path to analyze 
  * \return depth or -1 if the format of the path is not understood. 
  */

int OksSystem::File::depth(const std::string &path) throw() {
    if (path[0] == SLASH_CHAR) {
	int count = 0;
	std::string rest = path.substr(1); 
	while(! rest.empty()) {
	    std::string::size_type slash = rest.find(SLASH_CHAR);
	    if (slash==std::string::npos) return count;
	    count++;
	    rest = rest.substr(slash+1);
	} // while
	return count;	
    } // path with slash 
    if (protocol(path) == FILE_PROTOCOL) {
	return depth(uri(path));
    } //
    return -1;
} // depth

/** Extracts the first line of a text 
  * \param text the text to process
  * \return the first line of the text
  */

std::string OksSystem::File::first_line(const std::string &text) {
    std::string::size_type nl = text.find('\n');
    if (nl>0) return text.substr(0,nl); 
    return text;
} // first_line


/** Builds a \c File object ou of an url 
  * \param url the url, it must start with the file protocol
  * \return a File object
  */

OksSystem::File OksSystem::File::from_url(const std::string &url) {
    std::string prot = protocol(url);
    ERS_PRECONDITION(prot==FILE_PROTOCOL); 
    const std::string path = uri(url); 
    return File(path);
} // from_url

/** \return a string containing the working directory for the process */

std::string OksSystem::File::working_directory() {
    char *wd_buffer = getcwd(0,MAXPATHLEN);  
    if (wd_buffer) {
    	std::string directory = std::string(wd_buffer);
    	free(wd_buffer); 
	return directory;
    }
    std::string message("getting the current working directory");
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "getcwd", message.c_str() );
} // working_directory 

void OksSystem::File::working_directory(const File &dir) {
    const char * path = dir.c_full_name(); 
    const int status = ::chdir(path); 
    if (status<0) {
      std::string message = "on directory " + dir.full_name(); 
      throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "chdir", message.c_str() );
    }
} // working_directory


/** Expands a file containing a home directory reference (~user). 
  */


std::string OksSystem::File::expand_home(const std::string path) {
    ERS_PRECONDITION(! path.empty());
    ERS_PRECONDITION(path[0] == '~'); 
    std::string::size_type slash = path.find('/');
    const std::string user_name = path.substr(1,slash-1); 
    const std::string rest_path = path.substr(slash);  
    OksSystem::User user;
    if (! user_name.empty()) {
	user = OksSystem::User(user_name);
    } 
    return user.home() + rest_path;
} // home_directory


/** Builds a prettyfied version of permissions. 
* This should look similar to what \c ls returns with the -l flag
* \param permissions the permission to beautify 
* \return a string containing the beautified permissions 
*/

std::string OksSystem::File::pretty_permissions(mode_t permissions) {
    char buffer[11];
    buffer[0] = (permissions & S_ISUID) ? SETUID_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[1] = (permissions & S_IRUSR) ? READ_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[2] = (permissions & S_IWUSR) ? WRITE_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[3] = (permissions & S_IXUSR) ? EXECUTE_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[4] = (permissions & S_IRGRP) ? READ_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[5] = (permissions & S_IWGRP) ? WRITE_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[6] = (permissions & S_IXGRP) ? EXECUTE_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[7] = (permissions & S_IROTH) ? READ_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[8] = (permissions & S_IWOTH) ? WRITE_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[9] = (permissions & S_IXOTH) ? EXECUTE_CHAR_CODE : NOTHING_CHAR_CODE;
    buffer[10] = '\0';
    return std::string(buffer); 
} // pretty_permissions

/** Builds a prettyfied version of flags used by the \c open OksSystem call
* \param flags as passed to \c open
* \return a string containing the beautified flags
* \note This method only handles flags available accross plateforms. 
*/

std::string OksSystem::File::pretty_open_flag(int flags) {
    std::ostringstream flag_str;
    if (flags & O_WRONLY) {
        flag_str << HUMAN_OPEN_STR[1];
    } else if (flags & O_RDWR) {
        flag_str << HUMAN_OPEN_STR[0] << "-" << HUMAN_OPEN_STR[1];
    } else {
        flag_str << HUMAN_OPEN_STR[0];
    }
    if (flags & O_NONBLOCK) {
        flag_str << "/" << HUMAN_OPEN_STR[2];
    } 
    if (flags & O_APPEND) {
        flag_str << "/" << HUMAN_OPEN_STR[3];
    }
    if (flags & O_CREAT) {
        flag_str << "/" << HUMAN_OPEN_STR[4];
    }
    if (flags & O_TRUNC) {
        flag_str << "/" << HUMAN_OPEN_STR[5];
    } 
    if (flags & O_EXCL) {
        flag_str << "/" << HUMAN_OPEN_STR[6];
    }
    flag_str << "(" << flags << ")";
    return flag_str.str();
} // pretty_open_permission


int  OksSystem::File::unit(int u) {
    ERS_PRECONDITION(u >= 0);
    if (u==0) return 1;
    return KILOBYTE * unit(u-1);
} // unit

/** Builds a human readable textual description of a file size.
* The file size is expressed in Gigabytes (GB), Megabytes (MB) and Kilobytes (KB) and Bytes (B).
* \param size the size of the file
* \param cut_small should the display only contain the highest quantity? 
* For example if a file is 1 gigabyte and 20 bytes, if \c cut_small is \c true, then
* the 20 bytes are ignored. 
* \return a string containing the textual description
*/

std::string OksSystem::File::pretty_size(size_t size, bool cut_small) {
    if (0==size) return "0";
    std::ostringstream size_str;
    bool writen_something = false;
    for(int i=3;i>=0;i--) {
	const unsigned int size_unit = unit(i);
	if (size > size_unit && ! (cut_small && writen_something)) {
	    const int amount = size / size_unit;
	    size = size % size_unit;
	    size_str << amount << SPACE_CHAR << HUMAN_SIZE_STR[i];
	    writen_something = true;
	} // if unit is present
    } // loop
    return size_str.str(); 
} // pretty_size

// --------------------------------------
// Constructors
// --------------------------------------


/** Constructor 
  * \note This method does not create a file, it simply builds an object describing a file 
  */

OksSystem::File::File(const std::string &name) {
    set_name(name); 
} // File

/** Constructor 
  * \overload
  */

OksSystem::File::File(const char* c_name) {
    ERS_PRECONDITION(c_name); 
    std::string name = std::string(c_name);
    set_name(name); 
} // File

/** Constructor 
* \overload
*/

OksSystem::File::File(const File& other) {
    set_name(other.m_full_name);
} // File

// --------------------------------------
// Operators
// --------------------------------------



OksSystem::File::operator std::string() const throw() {
    return m_full_name;
} // operator std::string

OksSystem::File::operator const char*() const throw() {
    return m_full_name.c_str();
} // operator const char*

/** Cast to boolean, checks if the file exists 
  * \return \c true if the file exists 
  */

OksSystem::File::operator bool() const throw() {
    return exists(); 
} // operator bool

/** Comparison operator 
  * As path are canonicalized, we simply compare the full paths. 
  * \param other file to compare this file to
  * \return true if both file are equal 
  * \note This comparison method does not taken hard links into account 
  */

bool OksSystem::File::equals(const File &other) const throw() {
    return (m_full_name == other.m_full_name);
} // operator==

/** Cast to size type 
  * \return the size of the file
  * \see size()
  */

OksSystem::File::operator size_t() const {
    return size();
} // operator size_t




// --------------------------------------
// Methods
// --------------------------------------


/** Sets the name for a file.
  * The path name is first expanded and the canonlicalized. 
  * Expension means the following
  * \li If the path starts with / nothing is done 
  * \li If the path starts with ~ the username is resolved and the path starts from there
  * \li If the path starts with any other character, the working directory is prepended. 
  * 
  * The path is then made canonical, this implies removing all ./ and ../ sequences, 
  * and resolving all symbolic links. 
  * \param name the name of the file
  */

void OksSystem::File::set_name(const std::string &name) {
    ERS_PRECONDITION(! name.empty()); 
    const char c = name[0];
    std::string long_path;
    switch (c) {
	case '/' :
	    long_path = name;
	    break;
	case '~' : 
	    long_path = expand_home(name); 
	    break;
	default:
	    long_path = working_directory() + "/" + name;
	    break;
    } // switch     
    char buffer[PATH_MAX];
    const char* result = realpath(long_path.c_str(),buffer); 
    if (result==0) { // could not resolve path 
	m_full_name = long_path;
    } else { // we could canonicalize the path 
	m_full_name = buffer;
    } // 
} // set_name

/** \return the full (absolute) path of the file */

const std::string & OksSystem::File::full_name() const throw() {
    return m_full_name;
} // full_name

/** \return the full (absolute) path of the file */

const char* OksSystem::File::c_full_name() const throw() {
    return m_full_name.c_str(); 
} // c_full_name


/** \return the short name of the file - that is the name of the file in its directory */

std::string OksSystem::File::short_name() const throw() {
    return short_name(m_full_name);
} // short_name

/** Finds the name of the enclosing directory 
  * \return the path of the directory containing the file 
  */

std::string OksSystem::File::parent_name() const throw() {
  std::string::size_type size = m_full_name.size();
  std::string::size_type slash = m_full_name.rfind(SLASH_CHAR,size-1);
  if (slash==std::string::npos || slash==0) return ("/"); 
  return m_full_name.substr(0,slash); 
} // directory

/** \return the extension of the file */

std::string OksSystem::File::extension() const throw() {
    return extension(m_full_name);
} // extension

int OksSystem::File::depth() const throw() {
    return depth(m_full_name);
} // depth

/** \return the parent directory */

OksSystem::File OksSystem::File::parent() const {
    return File(parent_name());
} // parent

OksSystem::File OksSystem::File::child(const std::string &name) const {
    std::string child_name = m_full_name + "/" + name;
    return OksSystem::File(child_name);
} // child

OksSystem::File OksSystem::File::temporary(const std::string &prefix) const {
    char *tmp_name = tempnam(m_full_name.c_str(),prefix.c_str());
    if ( !tmp_name ) {
      std::string message = "while creating a valid filename in directory " + m_full_name;
      throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "tempnam", message.c_str() );
    }
    OksSystem::File tmp_file(tmp_name);
    free(tmp_name);
    return tmp_file;
} // temporary


/** Checks if the file exists in the fileOksSystem
  * \return \c true if the file exists, false otherwise 
  * \exception ers::IOIssue if an error occurs 
  */

bool OksSystem::File::exists() const throw() {
    struct stat file_status;
    const int result = stat(m_full_name.c_str(),&file_status);
    if (0==result) return true;
    return false;
} // exists

/** Extracts the mode information of the file. 
  * This is used to determine both the file type and the file permissions 
  * \return the mode of the file
  * \exception OksSystem::FStatFail if fstat fails
  */

mode_t OksSystem::File::get_mode() const {
    struct stat file_status;
    const int result = stat(m_full_name.c_str(),&file_status);
    if (0==result) {
	return file_status.st_mode;
    } // if
    std::string message = "on file/directory " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "stat", message.c_str() );
} // mode


/** \return the permissions for the file
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

mode_t OksSystem::File::permissions() const {
    mode_t mode = get_mode();
    return (mode & 07777);
} // permissions

/** \return the prettfied permissions for the file
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

std::string OksSystem::File::pretty_permissions() const {
    return pretty_permissions(permissions()); 
} // pretty_permissions

/** Converts permissions into a string 
  * \param permissions the permissions
  * \return a string contaning the permissions in octal form
  */

std::string OksSystem::File::to_string(mode_t permissions) throw() {
    std::ostringstream stream;
    stream.setf(std::ios::oct,std::ios::basefield);
    stream << permissions;
    return stream.str();
} // to_string


/** \return the size (in bytes) of the file
  * \exception OksSystem::IOIssue if an error occurs or the file does not exist 
  */

size_t OksSystem::File::size() const {
    struct stat file_status;
    const int result = stat(m_full_name.c_str(),&file_status);
    if (0==result) return file_status.st_size;
    std::string message = "on file/directory " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "stat", message.c_str() );
} // size 

/** \return the user-id of the owner of the file
  * \exception OksSystem::IOIssue if an error occurs or the file does not exist 
  */

uid_t OksSystem::File::owner_id() const {
    struct stat file_status;
    const int result = stat(m_full_name.c_str(),&file_status);
    if (0==result) return file_status.st_uid;
    std::string message = "on file/directory " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "stat", message.c_str() );
} // owner_id

OksSystem::User OksSystem::File::owner() const {
    return User(owner_id());
} // owner


/** \return the group-id of the group of the file
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

gid_t OksSystem::File::group() const {
    struct stat file_status;
    const int result = stat(m_full_name.c_str(),&file_status);
    if (0==result) return file_status.st_gid;
    std::string message = "on file/directory " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "stat", message.c_str() );
} // owner


/** Checks if the file a regular file (i.e not a directory, named pipe or anything else). 
  * \return \c true if the file is regular 
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

bool OksSystem::File::is_regular() const {
    const mode_t mode = get_mode();
    return (mode & S_IFREG);
} // is_regular


/** Checks if the file a directory 
 * \return \c true if the file is a directory  
 * \exception ers::IOIssue if an error occurs or the file does not exist 
 */

bool OksSystem::File::is_directory() const {
    const mode_t mode = get_mode();
    return (mode & S_IFDIR);
} // is_regular

/** Checks if the file a named pipe (FIFO) 
 * \return \c true if the file is named pipe (FIFO) 
 * \exception ers::IOIssue if an error occurs or the file does not exist 
 */

bool OksSystem::File::is_fifo() const {
    const mode_t mode = get_mode();
    return (mode & S_IFIFO);
} // is_pipe


std::string OksSystem::File::file_type() const {
    OksSystem::Executable file_command(FILE_COMMAND_PATH); 
    OksSystem::Executable::param_collection params;
    params.push_back("-b"); 
    params.push_back(full_name()); 
    return first_line(file_command.pipe_in(params)); 
} // file_type
    
/** Builds a vector containing all the files contained in a directory 
  * \return a vector of files contained in the directory 
  * \exception OksSystem::OpenFail if the directory could not be openend
  * \exception OksSystem::CloseFail if the directory cannot be closed 
  */

OksSystem::File::file_list_t OksSystem::File::directory() const {
    file_list_t file_vector;
    DIR* directory_ptr = opendir(m_full_name.c_str());
    if (! directory_ptr) {
      std::string message = "on directory " + m_full_name;
      throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "opendir", message.c_str() );
    }
    while(true) {
	struct dirent* directory_entry = readdir(directory_ptr); 
	if (directory_entry) {
	    const std::string name = directory_entry->d_name;
	    if ((name!=".") && (name!="..")) {
		std::string path = m_full_name + "/" + name;
		OksSystem::File file(path); 
		file_vector.push_back(file);
	    } // if
	} else {
	    break;
	} // if / else
    } // while
    int status = closedir(directory_ptr);
    if (status<0) {
      std::string message = "on directory " + m_full_name;
      throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "closedir", message.c_str() );
    }
    return file_vector;
} // directory

/** Unlinks (i.e deletes) a file. 
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

void OksSystem::File::unlink() const {
    const int result = ::unlink(m_full_name.c_str());
    if (0==result) return;
    throw OksSystem::RemoveFileIssue( ERS_HERE, errno, m_full_name.c_str() ); 
} //unlink

void OksSystem::File::rmdir() const {
    const int result = ::rmdir(m_full_name.c_str());
    if (0==result) return;
    std::string message = "on directory " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "rmdir", message.c_str() ); 
} // rmdir


/** Recursively delete files and directories.
  * If the file is a directory, all its child are deleted recursively. 
  * \exception OksSystem::UnlinkFail if \c unlink fails 
  */

void OksSystem::File::remove() const {
    if (is_directory()) {
	file_list_t childs = directory(); 
	for (file_list_t::const_iterator p=childs.begin();p!=childs.end();p++) {
	    const File f = *p;
	    if(f.exists()) {
	      f.remove(); 
	    }
	} // for
	rmdir();
    } else {
	unlink(); 
    } 
} // remove


/** Renames or moves a file. 
  * \param new_name the new name of the file 
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

void OksSystem::File::rename(const File &other) const {
    const char *source = c_full_name() ;
    const char *dest = other.c_full_name();
    const int result = ::rename(source,dest); 
    if (0==result) return;
    throw OksSystem::RenameFileIssue( ERS_HERE, errno, source, dest );
} // rename

/** Sets the permissions of the file 
  * \param permissions the new permissions
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */
 
void OksSystem::File::permissions(mode_t perm) const {
  if(perm !=  permissions()) {
    const int result = ::chmod(m_full_name.c_str(),perm); 
    if (0==result) {
      return;
    }
    std::string message = "on file/directory " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "chmod", message.c_str());
  }
} // permissions

/** Creates a directory
  * The current object is taken as the name of the directory to create. 
  * \param perm the permissions to associate with the directory 
  * \note if the directory exists, the method attempts to do a \c chmod 
  * to match the permissions
  */ 

void OksSystem::File::make_dir(mode_t perm) const {

    errno = 0;
    const int result = ::mkdir(m_full_name.c_str(),perm);
    int mkdir_error = errno;

    if (0==result) {
      try {
	permissions(perm);
      }
      catch (OksSystem::OksSystemCallIssue &e){
	std::string message = "on directory " + m_full_name;
	throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "mkdir -> chmod", message.c_str() ); 
      }
      return;
    } else {
      if (exists()) {
    	if (is_directory()) { // already exists we attempt a chmod
	  try {
	    permissions(perm);
	  }
	  catch (OksSystem::OksSystemCallIssue &e){
	    // ignore this case...
	  }
	  return;
	} // is directory
      } else { // exists
	std::string message = "on directory " + m_full_name;
	throw OksSystem::OksSystemCallIssue(ERS_HERE, mkdir_error, "mkdir", message.c_str()); 
      }
    }
    
} // makedir

/** Builds a full path. 
  * The current object is taken as the name of a directory to create. 
  * This method creates all the parent directories as needed
  * \param permissions the permissions to associate with the directory
  * \note If parent directories are created, their permissions will be those
  *       defined in \c permissions ORed with 0700. 
  *       This is needed to ensure we can actually write into the directories 
  *       we create. 
  */

void OksSystem::File::make_path(mode_t perm) const { 
    const File father = parent(); 
    if (! father.exists()) {
	mode_t father_permission = perm | S_IRWXU;// we need rights to write sons
	father.make_path(father_permission); 
    } // if
    make_dir(perm); 
} // makepath

/** Makes sure that the path for a file exists 
  * This is done by calling \c make_path on the parent directory 
  * \param permissions permissions used to create directories
  * \see OksSystem::File::make_path()
  */

void  OksSystem::File::ensure_path(mode_t perm) const {
    const File father = parent();
    father.make_path(perm); 
} // ensure_path


/** Creates a named pipe (FIFO) 
  * \param perm the permission to give the pipe
  */

void OksSystem::File::make_fifo(mode_t perm) const {
    if (exists()) {
	if (is_fifo()) {
	  if(perm != permissions()) {
	    permissions(perm); 
	    return;
	  } else {
	    return;
	  }
	} // is_fifo
    } // exists
    const int status = ::mkfifo(m_full_name.c_str(),perm); 
    if (status==0) {
      permissions(perm);
      return;
    } 
    std::string message = "while creating FIFO " + m_full_name;
    throw OksSystem::OksSystemCallIssue( ERS_HERE, errno, "mkfifo", message.c_str() ); 
} // makefifo
    



/** Conversion into a input stream pointer
  * This actually creates a new input stream that reads from the file. 
  * \return a dynamically allocated input stream 
  * \exception ers::IOIssue if an error occurs or the file does not exist 
  */

std::istream* OksSystem::File::input() const {
    try {
	std::ifstream *stream = new std::ifstream(m_full_name.c_str());
	stream->exceptions(std::ios::failbit | std::ios::badbit); 
	return stream;
    } catch (std::ios_base::failure &ex) {
	throw OksSystem::OpenFileIssue( ERS_HERE, errno, m_full_name.c_str(), ex ); 
    } // catch
} // std::istream*

/** Conversion into an output stream pointer
 * This actually creates a new output stream that writes to the file. 
 * \return a dynamically allocated output stream 
 * \param append is the file opened in append mode 
 * \exception ers::IOIssue if an error occurs
 */
    
std::ostream* OksSystem::File::output(bool append) const {
    try {
	std::ios::openmode mode = std::ios::out;
	if (append) {
	    mode |= std::ios::app;
	}
	std::ofstream *stream = new std::ofstream(m_full_name.c_str(),mode);
	stream->exceptions(std::ios::failbit | std::ios::badbit); 
	return stream;
    } catch (std::ios_base::failure &ex) {
	throw OksSystem::OpenFileIssue( ERS_HERE, errno, m_full_name.c_str(), ex ); 
    } // catch
} // std::ostream*

/** Stream a file object into a STL stream. 
  * \param stream destination stream.
  * \param file the file to write
  * \return the stream passed as parameter
  */

std::ostream& operator<<(std::ostream& stream, const OksSystem::File& file) {
    stream << OksSystem::File::FILE_PROTOCOL << ":/" << file.full_name();
    return stream;
} // operator<<

bool operator ==(const OksSystem::File &a, const OksSystem::File &b)  throw() {
    return a.equals(b); 
} // operator ==

bool operator !=(const OksSystem::File &a, const OksSystem::File &b)  throw() {
    return ! a.equals(b); 
} // operator !=





