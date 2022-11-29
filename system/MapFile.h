/*
 *  MapFile.h
 *  System
 *
 *  Created by Matthias Wiesmann on 07.01.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_MAP_FILE
#define SYSTEM_MAP_FILE

#include "system/File.h"

namespace System {
    
    class Descriptor ; 
    
    /** This class offers facilities to handle memory mapped files.
      * They can be used to map a file into memory
      * \author Matthias Wiesmann
      * \version 1.0
      * \brief Wrapper for memory map operations. 
      */
    
    class MapFile : public File {

    public:

      MapFile(const std::string &file, size_t size, size_t offset, bool read_mode, bool write_mode, mode_t permissions = 0666) ;
      ~MapFile(); 
      
      void zero() const ;                      /**< \brief builds a zeroed file with correct length */
      void map();                              /**< \brief maps the file in memory */
      void unmap();                            /**< \brief unmaps the file */
      bool is_mapped() const;                  /**< \brief is the file mopped im memory */
      bool is_loaded() const throw() ;         /**< \brief is the map loaded in memory */
      void *address() const throw() ;          /**< \brief the address of the memory mapped file */
      size_t memory_size() const throw() ;     /**< \brief the size of the map */
      System::Descriptor* fd() const throw() ; /**< \brief the file descriptor. This method returns a valid pointer only if called after map(). */
      
    protected:

      void    open_fd();           /**< \brief opens the file descriptor for the map */
      void    close_fd();          /**< \brief closes the file descriptor for the map */
      void    map_mem();           /**< \brief maps the file into memory */
      void    unmap_mem();         /**< \brief unmaps the file into memory */

      void *  m_map_address ;      /**< \brief the address of the map in memory */
      size_t  m_map_size ;         /**< \brief the size of the map */
      size_t  m_map_offset ;       /**< \brief offset in the file of the map */
      Descriptor *m_map_descriptor ;/**< \brief internal file descriptor */
      mode_t  m_map_permission ;   /**< \brief permissions associated with the file */
      bool    m_map_read ;         /**< \brief is the map readable  */
      bool    m_map_write ;        /**< \brief is the map writable  */
      bool    m_is_mapped ;        /**< \brief is the file mapped in memory */

    private:

      static const char* const MAP_LOAD_MSG ;       /**< \brief message pattern - map is loaded */
      static const char* const MAP_NOT_LOAD_MSG ;   /**< \brief message pattern - map is not loaded */
      static const char* const PAGE_SIZE_MSG ;      /**< \brief message pattern - illegal page size */
      static const char* const READ_OR_WRITE_MSG ;  /**< \brief message pattern - map mode is neither read nor write */
      static const char* const MAP_NOT_WRITE_MSG ;  /**< \brief message pattern - map is not writable */

    } ; // MapFile
    
} // ers

#endif
