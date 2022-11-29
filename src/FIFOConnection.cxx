/*
 *  FIFOConnection.cxx
 *  System
 *
 *  Created by Matthias Wiesmann on 07.04.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <fcntl.h>

#include "ers/ers.h"

#include "system/FIFOConnection.h"
#include "system/exceptions.h"

const unsigned int System::FIFOConnection::MAX_MESSAGE_LEN = 512;

/** Constructor, does not actually create the named pipe in the file system. 
  * To create it, use the \c make method
  * \param name the name (full path) of the named pipe
  */

System::FIFOConnection::FIFOConnection(const std::string &name) : System::File(name) {
  m_fifo_fd = 0;
  m_is_blocking = true;
} 

/** \overload */

System::FIFOConnection::FIFOConnection(const File &file) : System::File(file) {
  m_fifo_fd = 0;
  m_is_blocking = true;
} 

System::FIFOConnection::~FIFOConnection() {
  delete m_fifo_fd;
}

void System::FIFOConnection::make(mode_t perm) const {
    File::make_fifo(perm); 
} // make

/** Blocking read of a single message (string) from a FIFO
  * \return the actual message 
  * \note maximum message length is MAX_MESSAGE_LEN-1 bytes
  */

std::string System::FIFOConnection::read_message() const {
    ERS_ASSERT_MSG(exists(),"FIFO "<< c_full_name() << " does not exist."); 
    if (! is_fifo()) {
	ers::warning(System::Exception(ERS_HERE, std::string(c_full_name()) + std::string(" is not a FIFO"))); 
    } // should probably be FIFO
    System::Descriptor connection_fd(this,O_RDONLY,0); 
    char buffer[MAX_MESSAGE_LEN];
    while(true) {
	const int status = connection_fd.read(buffer,sizeof(buffer)-1);
	if (status>0) {
	    ERS_ASSERT(status<(int) sizeof(buffer)); 
	    buffer[status] = '\0';// we make sure we have a C-string
	    return std::string(buffer); 
	} // if 
	usleep(100000);
    } // while 
} // read_message

void System::FIFOConnection::send_message(const std::string &message) const {
    ERS_ASSERT_MSG(exists(),"FIFO "<< c_full_name() << " does not exist. Cannot put "<< message << " into FIFO.");
    if (! is_fifo()) {
	ers::warning(System::Exception(ERS_HERE, std::string(c_full_name()) + std::string(" is not a FIFO"))); 
    } // should probably be FIFO 
    const unsigned int l = message.size();
    ERS_RANGE_CHECK(1,l,MAX_MESSAGE_LEN); 
    System::Descriptor connection_fd(this,O_WRONLY,0);
    connection_fd.write(message.data(),l); 
} // send_message 

/**
 * \brief It writes a message to a FIFO.
 * \param message The message to write into the FIFO.
 * \note \li The maximum message length is MAX_MESSAGE_LEN-1 bytes.
 *       \li This method does not take care of opening the FIFO (as it happens in send_message()),
 *           and should be used only when the FIFO is opened using one of the open_w() or
 *           open_wr() methods (see linux "open", "fifo" and "write" man/info pages for more details).
 *       \li This method will block or not depending on the way the FIFO has been opened.
 */

void System::FIFOConnection::send(const std::string &message) const {
    ERS_ASSERT(m_fifo_fd);
    const unsigned int l = message.size();
    ERS_RANGE_CHECK(1,l,MAX_MESSAGE_LEN); 
    m_fifo_fd->write(message.data(),l); 
} // send

/**
 * \brief It reads a single message (string) from a FIFO.
 * \return The actual message.
 * \note \li The maximum message length is MAX_MESSAGE_LEN-1 bytes.
 *       \li This method does not take care of opening the FIFO (as it happens in read_message()),
 *           and should be used only when the FIFO is opened using one of the open_r() or
 *           open_wr() methods (see linux "open", "fifo" and "read" man/info pages for more details).
 *       \li This method will block or not depending on the way the FIFO has been opened.
 */

std::string System::FIFOConnection::read() const {
    ERS_ASSERT(m_fifo_fd);    
    char buffer[MAX_MESSAGE_LEN];
    while(true) {
      const int status = m_fifo_fd->read(buffer,sizeof(buffer)-1);
      if (status>0) {
	ERS_ASSERT(status<(int) sizeof(buffer)); 
	buffer[status] = '\0';// we make sure we have a C-string
	return std::string(buffer); 
      } // if 
      if (m_is_blocking == false && status == 0) return std::string("");
      usleep(100000); // Slow down the loop 
    } // while 
} // read

/**
 * \brief It opens the FIFO in read-only mode.
 * \return A pointer to a System::Descriptor object holding the FIFO file descriptor.
 * \param block If \c TRUE the FIFO will be opened in blocking mode.
 */

System::Descriptor* System::FIFOConnection::open_r(bool block) {

  ERS_ASSERT(!m_fifo_fd);

  int flags = O_RDONLY; 
  
  if(block == false) {
    flags = O_RDONLY|O_NONBLOCK;
    m_is_blocking = false;
  }

  try {
    m_fifo_fd = new System::Descriptor(this,flags,0);
  }
  catch(System::OpenFileIssue &ex) {
    delete m_fifo_fd;
    m_fifo_fd = 0;
    throw;
  }

  return m_fifo_fd;
  
}

/**
 * \brief It opens the FIFO in write-only mode.
 * \return A pointer to a System::Descriptor object holding the FIFO file descriptor.
 * \param block If \c TRUE the FIFO will be opened in blocking mode.
 */


System::Descriptor* System::FIFOConnection::open_w(bool block) {
  
  ERS_ASSERT(!m_fifo_fd);

  int flags = O_WRONLY;
  
  if(block == false) {
    flags = O_WRONLY|O_NONBLOCK;
    m_is_blocking = false;
  }

  try {
    m_fifo_fd = new System::Descriptor(this,flags,0);
  }
  catch(System::OpenFileIssue &ex) {
    delete m_fifo_fd;
    m_fifo_fd = 0;
    throw;
  }

  return m_fifo_fd;

}

/**
 * \brief It opens the FIFO in read and write mode.
 * \return A pointer to a System::Descriptor object holding the FIFO file descriptor.
 * \param block If \c TRUE the FIFO will be opened in blocking mode.
 */

System::Descriptor* System::FIFOConnection::open_rw(bool block) {
  
  ERS_ASSERT(!m_fifo_fd);

  int flags = O_RDWR;
  
  if(block == false) {
    flags = O_RDWR|O_NONBLOCK;
    m_is_blocking = false;
  }

  try {
    m_fifo_fd = new System::Descriptor(this,flags,0);
  }
  catch(System::OpenFileIssue &ex) {
    delete m_fifo_fd;
    m_fifo_fd = 0;
    throw;
  }
  
  return m_fifo_fd;
  
}

/**
 * \brief It closes the FIFO file descriptor.
 * \note This method has to be used to close the FIFO file descriptor only when
 *       it is opened using one of the open_r(), open_w() or open_wr() methods.
 */

void System::FIFOConnection::close() {

  delete m_fifo_fd;
  m_fifo_fd = 0;

}

/**
 * \brief It gets the FIFO file descriptor.
 * \note This method can to be used only when the FIFO 
 *       is opened using the open_r(), open_w() or open_rw() methods.
 * \return The FIFO file descriptor.
 */

int System::FIFOConnection::fd() const {

  ERS_ASSERT(m_fifo_fd);
  return m_fifo_fd->fd();

}
