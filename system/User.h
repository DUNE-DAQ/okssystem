/*
 *  User.h
 *  System
 *
 *  Created by Matthias Wiesmann on 03.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_USER
#define SYSTEM_USER

#include <unistd.h>
#include <sys/types.h>
#include <string>

namespace System {
    
    /** This class represents an user 
      * \brief User
      * \author Matthias Wiesmann
      * \version 1.0
      */
    
    class User {
	
protected:
	uid_t m_user_id ;                         ///< \brief the actual user id 
	mutable std::string m_user_name ;         ///< \brief cached user name 
	mutable std::string m_user_home ;         ///< \brief cached user home directory 
	mutable std::string m_user_real_name ;    ///< \brief cached real user name 
	void resolve() const ;                    ///< \brief resolve information from user-id 
	void resolve_safe() const throw() ;       ///< \brief resolve without throwing exceptions 
public:
	static const User ROOT ;                  ///< \brief constant for root user 
	User() throw() ;                          ///< \brief constructor for current user 
	User(uid_t user_id) throw() ;             ///< \brief constructor from uid 
	User(const std::string &name);            ///< \brief constructor from name 
	User(const User &other) throw() ;         ///< \brief copy constructor 
	operator uid_t() const throw() ;                   ///< \brief cast to user id
	operator std::string() const ;                     ///< \brief cast to string
	operator const char *() const ;                    ///< \brief cast to c-string
	uid_t identity() const throw() ;                   ///< \brief user-id
	const std::string & name() const ;                 ///< \brief username
	const std::string & name_safe() const throw() ;    ///< \brief username - no exception
	const std::string & home() const ;                 ///< \brief home directory name
	const std::string & real_name() const ;            ///< \brief real name
	void setuid() const ;                              ///< \brief tries to set the uid of current process 
    }; // User
    
}  // System 

std::ostream& operator<<(std::ostream& stream, const System::User& user) throw() ; 


bool operator ==(const System::User &a, const System::User &b)  throw();
bool operator !=(const System::User &a, const System::User &b)  throw();


#endif

