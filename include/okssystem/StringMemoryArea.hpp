/*
 *  StringMemoryArea.h
 *  System
 *
 *  Created by Matthias Wiesmann on 17.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#ifndef SYSTEM_STRING_MEMORY_AREA
#define SYSTEM_STRING_MEMORY_AREA

#include <map>
#include <vector>

namespace System {
    
    /** This class offers facilities to store variable length strings into a memory area. 
      * The class can also store vector of strings and arrays. 
      * Strings are manipulated using \e offsets those represent offset in the memory area.
      * As the addresses are relative, they are suitable for use in memory mapped files. 
      * 
      * \note The class is abstract, concrete subclasses need to implement methods that actually manipulate 
      * the memory area. 
      * \author Matthias Wiesmann
      * \version 1.0
      * \brief String memory storage facility 
      */
     
    class StringMemoryArea  {
public:
	typedef unsigned int offset_t ; 
	typedef std::vector<std::string> str_vector ; 
	typedef std::map<std::string, std::string> str_map ; 
protected:
	static const char STRING_SEPARATOR ;                     /**< \brief character used to separate strings (i.e., to tokenize strings in vectors) */
	static const char MAP_ENTRY_SEPARATOR ;                  /**< \brief character used to separate map entries */
	virtual const char * string_area_read() const = 0 ;      /**< \brief read pointer for the string area */
	virtual char* string_area_write() = 0 ;                  /**< \brief write pointer for the string area */
	virtual offset_t last_string() const = 0 ;               /**< \brief offset of the end of the last string */
	virtual void last_string(offset_t offset) = 0 ;          /**< \brief sets the offset of the end of last string */
	virtual size_t string_area_size() const = 0 ;            /**< \brief size of the string area */
public:
	offset_t add(const char *str) ; 
	size_t clear(offset_t offset) ; 
	const char* get_string(offset_t offset) const ; 
	offset_t insert(offset_t offset, const char* str) ; 
	void insert(offset_t *offset, const char* str) ; 
	offset_t insert(offset_t offset, const str_vector &vect) ; 
	void insert(offset_t *offset, const str_vector &vect) ; 
	str_vector get_vector(offset_t offset) const ; 
	offset_t insert(offset_t offset, const str_map &map) ;
	void insert(offset_t *offset, const str_map &map) ;
	str_map get_map(offset_t offset) const ; 
    } ; // StringMemoryArea
    
} // system

#endif
