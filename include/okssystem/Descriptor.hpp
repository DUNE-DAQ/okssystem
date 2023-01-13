/*
 *  Descriptor.h
 *  Test
 *
 *  Created by Matthias Wiesmann on 08.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef OKSSYSTEM_DESCRIPTOR
#define OKSSYSTEM_DESCRIPTOR

#include <string>
#include <sys/types.h>
#include <sys/stat.h>

namespace OksSystem { 

  class File;
  
  /** This class represents a low level file descriptor.
   * The descriptor is opened when the object is created. 
   * It can be close explicitely, or implicetly when the object is destroyed
   * \author Matthias Wiesmann
   * \version 1.0
   * \brief File descriptor / Socket wrapper
   */
  
  class Descriptor  {

  public:

    Descriptor(const File * file, int flags, mode_t perm );     
    ~Descriptor();  
    
    static int flags(bool read_mode, bool write_mode); 

    operator int() const throw();
    
    void close();						/**< \brief close the descriptor */
    void close_safe() throw();				        /**< \brief close the descriptor no exception */

    int read(void* buffer, size_t number) const;
    int write(const void * buffer, size_t number) const;  

    int fd() const throw();					/**< \brief file descritptor */    
    
    void closeOnExec();

  protected:
    
    void open(const File * file, int flags, mode_t perm);	/**< \brief internal open method */

  private:
    
    int m_fd;
    std::string m_name;					        /**< \brief internal file descriptor */	
    
  };// Descriptor
  
} // OksSystem

#endif

