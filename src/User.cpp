/*
 *  User.cxx
 *  System
 *
 *  Created by Matthias Wiesmann on 03.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <pwd.h> 

#include <iostream>
#include <sstream>

#include "ers/ers.hpp"

#include "system/User.hpp"
#include "system/exceptions.hpp"

const System::User System::User::ROOT(0);

System::User::User() throw() {
    m_user_id = ::getuid();
} // User

System::User::User(uid_t user_id) throw() {
    m_user_id = user_id;
} // User

System::User::User(const std::string &s_name) {
  
  errno = 0;
  long bufSize = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  
  if(bufSize == -1) {
    if(errno == 0) {
      bufSize = 1024;
    } else {  
      std::string message = "with argument _SC_GETPW_R_SIZE_MAX while getting info about user " + s_name;
      throw System::SystemCallIssue(ERS_HERE, errno, "sysconf", message.c_str());
    }
  }

  struct passwd pwd;
  struct passwd *result;  
  char* buf = new char[bufSize];
  errno = 0;
  int res = ::getpwnam_r(s_name.c_str(), &pwd, buf, bufSize, &result);
  if(res == 0) {
    if(result != 0) {
      m_user_id = result->pw_uid;
    } else {
      delete[] buf;
      std::string eMsg = "User " + s_name + " not found";
      throw System::SystemCallIssue(ERS_HERE, errno, "getpwnam_r", eMsg.c_str());
    }
  } else {
    delete[] buf;
    std::string message = "while getting info about user " + s_name;
    throw System::SystemCallIssue(ERS_HERE, res, "getpwnam_r", message.c_str());
  }
  delete[] buf;

  m_user_name = s_name;
} // User

System::User::User(const User &other) throw() {
    m_user_id = other.m_user_id;
    m_user_name = other.m_user_name;
} // User

// Operators
// =========


System::User::operator uid_t() const throw() {
    return m_user_id;
} // uid_t

System::User::operator std::string() const {
    return name();
} // string 

System::User::operator const char *() const {
    if (m_user_name.empty()) { resolve();}
    return m_user_name.c_str();
} // char* 

//  Methods
// =========

/** Returns the user_id for user
  * \return user-id
  */

uid_t System::User::identity() const throw() { 
    return m_user_id;
} // identity

/** This method is responsible for filling in the mutable fields of the class 
  * when needed, this is done by calling the \c getpwuid function, 
  * all fields of the object are then filled in. 
  * \throw System::SystemCallIssue if the user information cannot be found 
  */

void System::User::resolve() const {
  
  errno = 0;
  long bufSize = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  
  if(bufSize == -1) {
    if(errno == 0) {
      bufSize = 1024;
    } else {  
      std::ostringstream message;
      message << "with argument _SC_GETPW_R_SIZE_MAX while getting info about user with id " << m_user_id;
      throw System::SystemCallIssue(ERS_HERE, errno, "sysconf", message.str().c_str()); 
    }
  }

  struct passwd pwd;
  struct passwd *result;
  char* buf = new char[bufSize];
  errno = 0;
  int res = ::getpwuid_r(m_user_id, &pwd, buf, bufSize, &result);
  if(res == 0) {
    if(result != 0) {
      m_user_name = std::string(result->pw_name);
      m_user_home = std::string(result->pw_dir);
      m_user_real_name = std::string(result->pw_gecos);
    } else {
      delete[] buf;      
      std::ostringstream eMsg;
      eMsg << "User " << m_user_id << " not found";
      throw System::SystemCallIssue(ERS_HERE, errno, "getpwuid_r", eMsg.str().c_str());
    }
  } else {
    delete[] buf;
    std::ostringstream message;
    message << "while getting info about user with id " << m_user_id;
    throw System::SystemCallIssue(ERS_HERE, res, "getpwnam_r", message.str().c_str());
  }
  delete[] buf;

} // 

/** This method is the same as \c resolve() but does not throw exceptions.
  * Instead the method silently fails 
  * \see System::User::resolve()
  */

void System::User::resolve_safe() const throw() {

  errno = 0;
  long bufSize = ::sysconf(_SC_GETPW_R_SIZE_MAX);
  
  if(bufSize == -1) {
    if(errno == 0) {
      bufSize = 1024;
    } else {  
      m_user_name = "unknown";
      m_user_home = "unknown";
      m_user_real_name = "unknown"; 
      return;
    }
  }

  struct passwd pwd;
  struct passwd *result;
  char* buf = new char[bufSize];
  int res = ::getpwuid_r(m_user_id, &pwd, buf, bufSize, &result);
  if(res == 0) {
    if(result != 0) {
      m_user_name = std::string(result->pw_name);
      m_user_home = std::string(result->pw_dir);
      m_user_real_name = std::string(result->pw_gecos);
    } else {
      m_user_name = "unknown";
      m_user_home = "unknown";
      m_user_real_name = "unknown";      
    }
  } else {
    m_user_name = "unknown";
    m_user_home = "unknown";
    m_user_real_name = "unknown"; 
  }
  delete[] buf;

} // resolve_safe

/** Gets the (short) name of the user 
  * \return the name of user 
  */

const std::string & System::User::name() const {
    if (m_user_name.empty()) { resolve();}
    return m_user_name;
} // name

/** Gets the (short) name of the user - no error is thrown in case of problem
  * \return the name of user
  */

const std::string &  System::User::name_safe() const throw() {
    if (m_user_name.empty()) { resolve_safe(); }
    return m_user_name;
} // name

/** Gets the home directory of the user 
  * \return path to home directory 
  */

const std::string & System::User::home() const {
    if (m_user_home.empty()) { resolve();}
    return m_user_home;
} // home

/** Gets the 'real name' of the user 
  * \return real name 
  */

const std::string & System::User::real_name() const {
    if (m_user_real_name.empty()) { resolve();}
    return m_user_real_name;
} // real_name

/** Sets the user-identity of the current process to this user. 
  * This method will only succeed if the current user has sufficient privileges to changed uids
  * (typically because the current user is root). 
  */

void System::User::setuid() const {
    const int status = ::setuid(m_user_id); 
    if (status<0) {
      std::ostringstream message;
      message << "while setting the effective user ID to " << m_user_id << "(" << this->name_safe() << ")";
      throw System::SystemCallIssue( ERS_HERE, errno, "setuid", message.str().c_str() );
    } 
} // setuid


/** Output operator, writes username followed by uid between parenthesis
  * \param stream the stream to write into
  * \param user the user object to write
  * \return parameter stream
  */

std::ostream& operator<<(std::ostream& stream, const System::User& user) throw() {
    unsigned int i = (unsigned int) user.identity(); 
    const std::string name = user.name_safe();
    stream << name << '(' << i << ')';
    return stream;
} // operator<< 

/** Equality operator 
  * \param a first user to compare 
  * \param b second user to compare 
  * \return true if they are equal
  */

bool operator ==(const System::User &a, const System::User &b)  throw() {
    return a.identity() == b.identity();
} // 

/** Inequality operator 
 * \param a first user to compare 
 * \param b second user to compare 
 * \return true if they are not equal
 */

bool operator !=(const System::User &a, const System::User &b)  throw() {
    return a.identity() != b.identity();
} // 


