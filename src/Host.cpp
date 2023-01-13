/*
 *  Host.cxx
 *  OksSystem
 *
 *  Created by Matthias Wiesmann on 03.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <netdb.h>

#include <unistd.h>

#include <iostream>
#include <sstream>
#include <strings.h>

#include "okssystem/Host.hpp"

#define BUFFER_SIZE 256 

/** Translates an hostname into an ip address
  * \param name hostname to translate
  * \return the ip addrress, 0.0.0.0 if name not found. 
  * \note only returns the first ip address associated with name 
  */

struct sockaddr_in OksSystem::Host::resolve(const std::string &name) throw() {
    struct addrinfo *info_ptr;
    const char *str = name.c_str(); 
    const int status = getaddrinfo(str,0,0,&info_ptr); 
    struct sockaddr_in address;
    bzero(&address,sizeof(address)); 
    address.sin_family = AF_INET;
    address.sin_addr.s_addr=0;
    if (status==0) {
	struct sockaddr_in *ptr = (sockaddr_in *) info_ptr->ai_addr;
	address = *ptr;
	freeaddrinfo(info_ptr); 
    } // if
    return address;
} // resolve

/** Translates an ip address into an hostname 
  * \param address the address to translate
  * \return the hostname an empty string if the hostname could not be resolved 
  */

std::string OksSystem::Host::resolve(struct sockaddr_in address) throw() {
    char buffer[NI_MAXHOST];
    const struct sockaddr *ptr = (const struct sockaddr *) &address;
    const int status= getnameinfo(ptr,sizeof(address),buffer,sizeof(buffer),0,0,0);
    if (status!=0) return to_string(address); 
    return std::string(buffer); 
} // resolve

/** Tries to build a fully qualified name.
  * This is done first by converting the name to ip then the ip to a name.
  * If this fails (basically, we cannot do DNS resolves) 
  * the non fully qualified name is returned 
  * \param name the (partial) name
  * \return fullname 
  */

std::string OksSystem::Host::expand(const std::string &name) throw() {
    const struct sockaddr_in address = resolve(name); 
    if (address.sin_addr.s_addr!=0) return resolve(address); 
    return std::string(name) ;
} // expand

/** Transliterate an ip address into the canonical dotted text version (w.x.y.z). 
* \param ip_addr the address to translate
* \return string containing the text version 
* \note Should use addr2ascii
*/

std::string OksSystem::Host::to_string(struct sockaddr_in ip_addr) {
    const char* s = inet_ntoa(ip_addr.sin_addr);
    return std::string(s);
} // to_string

// Constructors, destructors
// --------------------------

/** Constructors for the local host
  */

OksSystem::Host::Host() throw() {} // Host

OksSystem::Host::Host(const OksSystem::Host &other) {
    m_name = other.m_name;
    if (! other.m_full_name.empty()) {
	m_full_name = other.m_full_name;
    } // if
} // Host


/** Constructor for host by name
  * \param name name of the host
  */

OksSystem::Host::Host(const std::string &s_name) {
    m_name = s_name;
} // Host


OksSystem::Host::Host(struct sockaddr_in ip_addr) {
    m_name = resolve(ip_addr);
    m_full_name = m_name;
} // Host


OksSystem::Host::~Host() throw() {
} // OksSystem

// Operator
// --------------------------

/** Cast conversion into ip address 
  * \return ip address 
  */

OksSystem::Host::operator struct sockaddr_in() const throw() {
    return ip(); 
} // struct sockaddr_in

/** Comparison method
 * We try to expand both name and compare those 
 * \param other the host to compare to 
 * \return \c true if both have the same fully qualified name 
 */

bool OksSystem::Host::equals(const Host &other) const throw() {
    return full_name()==other.full_name();
} // equals


// Methods
// --------------------------

/** Name (this might be non fully qualified name)
  * \return name of the host
  */

const std::string & OksSystem::Host::name() const throw() { return m_name;} 

/** IP address of the host 
  * \return IP address of the host, or 0.0.0.0 if it cannot be resolved
  */

struct sockaddr_in OksSystem::Host::ip() const throw() { return resolve(m_name);}

/** Fully qualified name of the host
  * \return Fully qualified name of the host, or the string \"0.0.0.0\"
  */

const std::string & OksSystem::Host::full_name() const throw() {
    if (m_full_name.empty()) {
	m_full_name = expand(m_name); 
    }  // if
    return m_full_name;
} // full_name

/** The IP Address of the host, as a string 
  * \return a string containing the IP address of the host in w.x.y.z format 
  */

std::string OksSystem::Host::ip_string() const throw() {
    const struct sockaddr_in address = ip(); 
    return to_string(address);
} // ip_string



bool OksSystem::operator ==(const Host &a, const Host &b)  throw() {
    return a.equals(b); 
} // operator ==

bool OksSystem::operator !=(const Host &a, const Host &b) throw() {
    return ! a.equals(b);
} // operator ≠

// Local Host
// --------------------------

/** Instance of local host
  * \brief LocalHost singleton
  */

OksSystem::LocalHost* OksSystem::LocalHost::s_instance = 0;

/** Short-cut method - gives the local hostname
 * This name is not guaranteed to be a fully qualified name.
 * \return hostname 
 * \see OksSystem::LocalHost::name()
 */

const std::string & OksSystem::LocalHost::local_name() throw() {
    return instance()->name(); 
} // localhostname

/** Short-cut method - gives the fully qualified local hostname 
 * \return fully qualified hostname
 * \see OksSystem::LocalHost::full_name()
 */

const std::string & OksSystem::LocalHost::full_local_name() throw() {
    return instance()->full_name(); 
} // fulllocalhostname

/** This method returns a pointer to the singleton instance.
  * There is no need to ever create any instance of LocalHost
  * \return pointer to singleton instance 
  * \note utility methods like \c full_local_name use this instance class 
  */

const OksSystem::LocalHost* OksSystem::LocalHost::instance() throw() {
    if (0==s_instance) {
	s_instance = new LocalHost(); 
    } // 
    return s_instance;
} // instance

/** Constructor 
  * You should \b not construct new instances, use \c instance to get the 
  * singleton instance. 
  */
 
OksSystem::LocalHost::LocalHost() throw() : Host() {
    struct utsname u_name_data;
    const int status = ::uname(&u_name_data);
    if (status==0) { 
	m_name =    u_name_data.nodename;
	m_os_name = u_name_data.sysname;
	m_release = u_name_data.release;
	m_version = u_name_data.version;
	m_machine = u_name_data.machine;
    }  else {
	char buffer[NI_MAXHOST];
	const int host_status = gethostname(buffer,sizeof(buffer));
	if (host_status==0) {
	    m_name = buffer;
	} else { // both uname and gethostname screwed 
	    m_name.clear(); 
	} // check for host_name
    } // uname failed. 
} // LocalHost

OksSystem::LocalHost::~LocalHost() throw() {
  //  delete s_instance;
}

/** \return Operating OksSystem name */

const std::string & OksSystem::LocalHost::os_name() const throw() {
    return m_os_name;
} // os_name

/** \return Operating OksSystem release */

const std::string & OksSystem::LocalHost::os_release() const throw() {
    return m_release;
} // os_release

/** \return Operating OksSystem version */

const std::string & OksSystem::LocalHost::os_version() const throw() {
    return m_version;
} // os_version

/** \return machine name */

const std::string & OksSystem::LocalHost::machine() const throw() {
    return m_machine;
} // machine

/** This method builds a description of the localhost 
  * This contains all the of the information handled by this class. 
  * \return description text
  */

const std::string & OksSystem::LocalHost::description() const throw() {
    if (m_description.empty()) {
	std::ostringstream stream;
	stream << m_os_name << " " << m_release << "/" << m_machine;
	m_description = stream.str(); 
    } 
    return m_description;
} // description

/** Convenience method, finds the fully qualified host name for this node 
  * \return c-string with fully qualified hostname 
  * \note the string is owned by the singleton object it should not be deleted
  * \note if resolve fails (typically because there is no network) this method might return 
  *       the short (unqualified) host name. 
  */

const char* OksSystem::getfullhost() throw() {
    const std::string & str = OksSystem::LocalHost::full_local_name();
    return str.c_str(); 
} // full_host_name

/** Comparison operator 
  * Simple optimisation - if we compare two localhost instances, they are always equal
  * \param a first instance
  * \param b second instance
  */

bool OksSystem::operator ==(const LocalHost &, const LocalHost & ) throw()
{
    return true;
}

/** Comparison operator 
 * Simple optimisation - if we compare two localhost instances, they are always equal
 * \param a first instance
 * \param b second instance
 */

bool OksSystem::operator !=(const LocalHost &, const LocalHost & ) throw()
{
    return false;
}

std::ostream& operator<<(std::ostream& stream, const OksSystem::Host& host) {
    stream << host.full_name();  
    return stream;
} // operator<<



