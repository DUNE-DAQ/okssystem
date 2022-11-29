/*
 *  Descriptor.cxx
 *  System
 *
 *  Created by Matthias Wiesmann on 08.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */
 
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
 
#include "system/File.h"
#include "system/Descriptor.h"
#include "system/exceptions.h"

#include "ers/ers.h"

int System::Descriptor::flags(bool read_mode, bool write_mode) {
    if (read_mode && write_mode) { 
	return O_RDWR | O_CREAT; 
    }
    if (read_mode) {
	return O_RDONLY;
    } 
    if (write_mode) {
	return O_WRONLY | O_CREAT; 
    } 
    return 0;
} // flags


System::Descriptor::Descriptor(const File * file, int i_flags, mode_t perm) {
    ERS_ASSERT( file )
    open(file,i_flags,perm); 
} // Descriptor

System::Descriptor::~Descriptor() {
    if (m_fd>=0) { 
	close_safe(); 
    } 
} //  ~Descriptor

System::Descriptor::operator int() const throw() {
    return m_fd;
} // operator int()


/** Opens the descriptor 
  * \param file pointer to the file to open 
  * \param flags open flags
  * \param perm the permissions
  */

void System::Descriptor::open(const File * file, int i_flags, mode_t perm) {
    ERS_ASSERT( file )
    bool alreadyExists = file->exists();
    m_fd = ::open(*file,i_flags,perm);
    m_name = file->full_name();
    if (m_fd<0) throw System::OpenFileIssue( ERS_HERE, errno, m_name.c_str() );
    if((alreadyExists == false) && ((i_flags & O_CREAT) != 0)) {
      ::fchmod(m_fd, perm);
    }
} // open

/** Closes the descriptor
  * \param file optional pointer to the file that we close (used for pretty printing potential exceptions). 
  * \exception System::CloseFail if there is problem in the \c close system call
  */

void System::Descriptor::close() {
    const int status = ::close(m_fd); 
    if (status<0) {
	throw System::CloseFileIssue( ERS_HERE, errno, m_name.c_str() ); 
    } // 
    m_fd = -1 ;
} // close


/** Closes a descriptor safely, i.e without throwing exceptions
  * If there is a proble, the information is sent to the warning stream
  */ 

void System::Descriptor::close_safe() throw() {
    const int status = ::close(m_fd); 
    if (status<0) {
	ers::warning( System::CloseFileIssue( ERS_HERE, errno, m_name.c_str() ) ); 
    } // if
} // close_safe

int System::Descriptor::read(void* buffer, size_t number) const {
    ssize_t status = ::read(m_fd,buffer,number);
    if (status<0) throw System::ReadIssue( ERS_HERE, errno, m_name.c_str() );
    return status;
} // read


int System::Descriptor::write(const void* buffer, size_t number) const {
    ssize_t status = ::write(m_fd,buffer,number);
    if (status<0) throw System::WriteIssue( ERS_HERE, errno, m_name.c_str() );
    return status;
} // write

int System::Descriptor::fd() const throw() { 
  return m_fd;
} 

/**
 * \brief It flags the file descriptor to be closed after any call to the exec system function.
 */

void System::Descriptor::closeOnExec() {

  bool success = false;
  int storeErrno = 0;

  int oldFlags = ::fcntl(m_fd, F_GETFD);
  if(oldFlags != -1) {
    int newFlags = ::fcntl(m_fd, F_SETFD, oldFlags|FD_CLOEXEC);
    if(newFlags == -1) {
      storeErrno = errno;
    } else {
      success = true;
    }
  } else {
    storeErrno = errno;
  }  
  
  if(!success) {
    std::string eMsg = "File descriptor for file " + m_name +
      " will not be closed after exec. Reason: " + std::string(::strerror(storeErrno));
    throw System::SystemCallIssue(ERS_HERE, storeErrno, "fcntl", eMsg.c_str());
  }

}
