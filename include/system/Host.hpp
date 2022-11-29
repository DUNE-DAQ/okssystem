/*
 *  Host.h
 *  System
 *
 *  Created by Matthias Wiesmann on 03.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_HOST
#define SYSTEM_HOST

#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>
#include <vector>

namespace System {
    
    /** This class represents an network host. 
      * It offers facilities to translate name to addresses and vice versa.
      * \author Matthias Wiesmann
      * \version 1.0
      * \brief Network host
      */
    
    class Host {
protected:
	std::string m_name ;                      ///< \brief name of the host
	mutable std::string m_full_name ;         ///< \brief cached fully qualified host name
	Host() throw() ;                          ///< \brief constructor for current host 
public:
	static struct sockaddr_in resolve(const std::string &name) throw() ;  ///< \brief name to ip conversion 
	static std::string resolve(struct sockaddr_in address) throw() ;      ///< \brief ip to name conversion
	static std::string expand(const std::string &name) throw() ;          ///< \brief expands to full name
	static std::string to_string(struct sockaddr_in ip_addr) ;            ///< \brief ip to string conversion
	Host(const Host &other) ;                                             ///< \brief copy constructor 
	Host(const std::string &name);                                        ///< \brief constructor from name 
	Host(struct sockaddr_in ip_addr);                                     ///< \brief constructor from ip address
	virtual ~Host() throw();                                                      ///< \brief destructor 
	operator struct sockaddr_in() const throw();                          ///< \brief cast to ip address 
	bool equals(const Host &other) const throw() ;                        ///< \brief equality method
	struct sockaddr_in ip() const throw() ;                               ///< \brief get ip address 
	const std::string & name() const throw() ;                            ///< \brief get name 
	const std::string & full_name()  const throw() ;                      ///< \brief get fully qualified name 
	std::string ip_string()  const throw() ;                              ///< \brief get ip in string mode 
    } ; // Host
    
    
    bool operator ==(const Host &a, const Host &b) throw();                   ///< \brief equality operator
    bool operator !=(const Host &a, const Host &b) throw();                   ///< \brief inequality operator
    
    
    /** This class represents the local host 
      * It has some additional fields with additional information about the host
      * \version 1.1 
      * \brief Network host - localhost
      * \author Matthias Wiesmann
      */
    
    class LocalHost : public Host {
	
protected:
	static LocalHost* s_instance ;                                   ///< \brief singleton for local host
	std::string m_os_name ;                                          ///< \brief name of the operating system
	std::string m_release ;                                          ///< \brief release of the operating system
	std::string m_version ;                                          ///< \brief version of the operating system
	std::string m_machine ;                                          ///< \brief machine type
	mutable std::string m_description ;                              ///< \brief machine description (cached)
public:
	static const std::string & local_name() throw() ;                ///< \brief localhostname
	static const std::string & full_local_name() throw() ;           ///< \brief fully qualified local host name
	static const LocalHost* instance() throw() ;                     ///< \brief pointer to the singleton local host
	LocalHost() throw() ;
	~LocalHost() throw();
	const std::string & os_name() const throw() ;                    ///< \brief name of the operating system
	const std::string & os_release() const throw() ;                 ///< \brief release of the operating system
	const std::string & os_version() const throw() ;                 ///< \brief version of the operating system
	const std::string & machine() const throw() ;                    ///< \brief version of the operating system
	const std::string & description() const throw() ;                ///< \brief machine type */

    } ; // LocalHost

    const char* getfullhost() throw() ; ///< \brief get fully qualified host name 

    bool operator ==(const LocalHost &a, const LocalHost &b)  throw(); 
    bool operator !=(const LocalHost &a, const LocalHost &b)  throw(); 

    
} // System 


std::ostream& operator<<(std::ostream& stream, const System::Host& host) ; 

#endif
