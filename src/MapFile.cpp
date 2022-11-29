/*
 *  MapFile.cxx
 *  ers
 *
 *  Created by Matthias Wiesmann on 07.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <sys/types.h>
#include <sys/mman.h>
#include <iostream>
#include <unistd.h>


#include "system/Descriptor.hpp"
#include "system/MapFile.hpp"
#include "system/exceptions.hpp"

#include "ers/Assertion.hpp"

/** Builds a memory mapped file. 
  * \param name the path of the file to map. 
  * \param size the size of the region to memory map - should be a multiple of the pagesize. 
  * \param offset the offset in the file - should be a multiple of pagesize. 
  * \param read_mode should the map be read from the file. 
  * \param write_mode should the map be writable. 
  * \param permissions the permissions associated with the file 
  */

System::MapFile::MapFile(const std::string &name, size_t s, size_t o, bool read_mode, bool write_mode, mode_t perm) : File(name) {
    ERS_PRECONDITION((read_mode || write_mode)); 
    const int page_size = getpagesize();
    ERS_PRECONDITION((s % page_size)==0);
    ERS_PRECONDITION((o % page_size)==0);
    (void) page_size;// to suppress warning when asserts are disabled
    m_map_read = read_mode;
    m_map_write = write_mode;
    m_map_permission = perm;
    m_map_size = s;
    m_map_offset = o;
    m_map_address = 0;
    m_map_descriptor = 0;
    m_is_mapped = false;
} // MapFile

/** Destructor - the file should be unmapped before destruction. 
  */

System::MapFile::~MapFile() {
    if (m_map_descriptor!=0) {
	close_fd(); 
    } // file descriptor open 
} // ~MapFile

/** Opens the file descriptor associated with the memory map. 
  * \exception OpenFail if the \c open system call fails
  */

void System::MapFile::open_fd() {
    const int flags = System::Descriptor::flags(m_map_read,m_map_write);
    m_map_descriptor = new System::Descriptor(this, flags,m_map_permission);
} // open_fd 


/** Closes the file descriptor associated with the memory map.
  * \exception CloseFail if the \c close system call fails 
  */

void System::MapFile::close_fd() {
    m_map_descriptor->close(); 
    delete(m_map_descriptor);
    m_map_descriptor = 0;
} // close_fd

/** Internal method - does the actual memory mapping.  
  * \exception MMapFail if the \c mmap operation succeeded 
  * \exception Precondition if the file descriptor is not valid. 
  */

void System::MapFile::map_mem() {
    ERS_PRECONDITION(m_map_descriptor); 
    int prot = 0;
    int flags = MAP_FILE | MAP_SHARED;
    if (m_map_read)  { 
	prot|=PROT_READ; 
    } 
    if (m_map_write) { 
	prot|=PROT_WRITE;
    }
    m_map_address = ::mmap(0,m_map_size,prot,flags,*m_map_descriptor,m_map_offset); 
    if (m_map_address==MAP_FAILED || m_map_address==0) {
        m_is_mapped = false;
	std::string message = "on file " + this->full_name();
	throw System::SystemCallIssue( ERS_HERE, errno, "mmap", message.c_str() ); 
    } /* map_failed */ else {
        m_is_mapped = true;
    }
} // map_mem

/** Internal method - does the actual memory unmapping. 
  * \exception MUnmapFail if the \c munmap operation fails. 
  */

void System::MapFile::unmap_mem(){
    ERS_PRECONDITION(m_map_address!=0);
    const int status = munmap(m_map_address,m_map_size);
    if (status<0) {
      m_is_mapped = true;
      std::string message = "on file " + this->full_name();
      throw System::SystemCallIssue( ERS_HERE, errno, "munmap", message.c_str() );
    } else {
      m_is_mapped = false;
    }
} // unmap_mem

/** Creates a zero filled file with correct length 
  */

void System::MapFile::zero() const {
    ERS_ASSERT(m_map_write==true); 
    const int flags = System::Descriptor::flags(false,true); 
    System::Descriptor fd(this, flags,m_map_permission);
    const int page_size = ::getpagesize();
    const int page_num = (m_map_size+m_map_offset) / page_size;
    void *buffer= calloc(1,page_size);
    for(int i=0;i<page_num;i++) {
	fd.write(buffer,page_size); 
    } // for
    free(buffer);
    fd.close(); 
} // zero 


/** Maps the file into memory. 
  * This is done first by opening the file descriptor for the file, then doing the actual map. 
  * \exception OpenFail error while opening the file 
  * \exception MMapFail error while mapping the file
  */

void System::MapFile::map() {
    open_fd();
    map_mem(); 
} // map 

/** Unmaps the file from memory 
  * This is done first by unmapping the file, then closing the file-descriptor.
  * \exception CloseFail error while closing the file.
  * \exception MUnmapFail error while unmapping the file. 
  */

void System::MapFile::unmap() {
    unmap_mem();
    close_fd(); 
} // unmap

bool System::MapFile::is_loaded() const throw() {
    return (m_map_address!=0);
} // is_loaded


/** Gives the actual address of the map in memory. 
  * If the file is not yet mapped, this will be 0. 
  * \return point to the start address of the mapped file in memory. 
  */

void *System::MapFile::address() const throw() { 
    return m_map_address;
} // address

/** Gives the actual size of the map in memory. 
  * \return the size of the memory region
  */

size_t System::MapFile::memory_size()  const throw() { 
    return m_map_size;
} // memory_size

/** Returns if the file is mapped or not in memory */

bool System::MapFile::is_mapped() const {
  return m_is_mapped;
}

System::Descriptor* System::MapFile::fd() const throw() {
  return m_map_descriptor;
}
