/*
 *  StringMemoryArea.cxx
 *  System
 *
 *  Created by Matthias Wiesmann on 17.02.05.
 *  Copyright 2005 CERN. All rights reserved.
 *
 */

#include <sstream>
#include "ers/ers.h"

#include "system/StringMemoryArea.h"

const char System::StringMemoryArea::STRING_SEPARATOR = (char) 3;
const char System::StringMemoryArea::MAP_ENTRY_SEPARATOR = (char) 4; 

/** Inserts a string at the end of the string region  
 * \param str the string to insert 
 * \return the offset in the string region of the string 
 */

System::StringMemoryArea::offset_t System::StringMemoryArea::add(const char *str) {
    ERS_PRECONDITION(str); 
    const size_t l = strlen(str); 
    const offset_t offset = last_string(); 
    const offset_t start = offset+1; 
    const offset_t end = start + l ; 

    ERS_RANGE_CHECK(1, end, string_area_size());
    char *data = string_area_write();
    char *current = &data[start];
    strcpy(current,str);
    last_string(end);
    return start; 
} // add

/** Clears a string in the string region 
 * \param offset the offset of the start of the string
 * \return the lenght of the cleared string
 */

size_t System::StringMemoryArea::clear(offset_t offset) {
    char *data = string_area_write(); 
    size_t count = 0; 
    while(data[count+offset]) {
	data[count+offset] = '\0'; 
	count++; 
    } // while
    return count; 
} // clear_string

/** Finds a string in the string area 
 * \param offset the starting offset of the string
 * \return pointer to string 
 * \note the actual characters are in the shared area 
 */

const char* System::StringMemoryArea::get_string(offset_t offset) const {
    if (offset==0) return 0; 
    ERS_RANGE_CHECK(1, offset, string_area_size());
    const char* str = string_area_read();
    if (str == NULL){
      return 0;
    }
    return &(str[offset]); 
} // get_string

/** Inserts a string in the string area.
 * \param offset offset of the existing string to replace - 0 means no offset defined 
 * \param str the string to insert 
 * \return the actual offset where the string was inserted
 * \note An existing string is replaced only if the next string can fit in the space. 
 *       if not, the old string is zeroed and the new string inserted at the end of the area. 
 *       This can lead to fragmentation, so you should avoid replacing strings. 
 */

System::StringMemoryArea::offset_t System::StringMemoryArea::insert(offset_t offset, const char* str) {
    ERS_PRECONDITION(str);
    if (offset==0) return add(str); 
    ERS_RANGE_CHECK(1, offset, string_area_size());
    size_t available = clear(offset); 
    if (available>=strlen(str)) {
	char *data = string_area_write();
	char *dest = &data[offset]; 
	strcpy(dest,str); 
	return offset; 
    } // if 
    return add(str); 
} // insert_string

/** Inserts a string into memory 
  * This version updates the memory reference
  * \param address of the offset of the string
  * \param str the string to insert
  */

void System::StringMemoryArea::insert(offset_t *offset, const char* str) {
    ERS_PRECONDITION(offset); 
    const offset_t n_offset = insert(*offset,str); 
    *offset = n_offset; 
} // insert

/** Extracts a vector of string from memory 
  * \param offset offset of the vector
  * \return a vector of strings 
  */

System::StringMemoryArea::str_vector System::StringMemoryArea::get_vector(offset_t offset) const {

    System::StringMemoryArea::str_vector vector; 
    const char *str = get_string(offset);

    if(str != 0) {

      const std::string text(str);

      std::string::size_type start_p = 0;
      std::string::size_type end_p = 0;
      
      while(start_p < text.size()) {
	end_p = text.find(STRING_SEPARATOR,start_p);
	if (end_p == std::string::npos) {
	  end_p = text.length();
	}
	std::string extract = text.substr(start_p, end_p - start_p);
	if(extract.size() > 0) {
	  vector.push_back(extract);
	}
	start_p = end_p + 1;
      } // while
      
    } // if

    return vector;

} // get_vector

/** Inserts a vector in the string area 
  * \param offset the offset of the string
  * \param vect the vector to insert 
  * \note the strings should not contain the caracter STRING_SEPARATOR (03)
  */

System::StringMemoryArea::offset_t System::StringMemoryArea::insert(offset_t offset, const str_vector &vect) {
  //    ERS_RANGE_CHECK(0, offset, string_area_size());
    str_vector::const_iterator pos; 
    std::ostringstream stream; 
    for(pos=vect.begin();pos!=vect.end();pos++) {
	if (vect.begin()!=pos) {
	    stream << STRING_SEPARATOR;
	} // if first
	stream << (*pos); 
    } // for
    return insert(offset,stream.str().c_str());
} // insert

/** Inserts a vector into memory 
 * This version updates the memory reference
 * \param address of the offset of the string
 * \param str the string to insert
 * \note the strings should not contain the caracter STRING_SEPARATOR (03)
 */

void System::StringMemoryArea::insert(offset_t *offset, const str_vector &vect) {
    ERS_PRECONDITION(offset); 
    const offset_t n_offset = insert(*offset,vect); 
    *offset = n_offset; 
} // insert

/** Reads a string map from memory 
  * \param offset offset of the map in memory 
  * \return a map of strings 
  */

System::StringMemoryArea::str_map System::StringMemoryArea::get_map(offset_t offset) const {

    ERS_RANGE_CHECK(1, offset, string_area_size());
    System::StringMemoryArea::str_map map; 
    const char* str = get_string(offset); 
    
    if(str != 0) {

      const std::string text(str);
      std::string::size_type start_p = 0;
      std::string::size_type end_p = 0;
      
      while(start_p < text.size()) {
	end_p = text.find(MAP_ENTRY_SEPARATOR,start_p);
	if (end_p == std::string::npos) {
	  end_p = text.length();
	}
	const std::string entry(text.substr(start_p, end_p - start_p));
	const std::string::size_type entrySize = entry.size();
	if(entrySize > 0) {
	  const std::string::size_type separator_p = entry.find(STRING_SEPARATOR);
	  if(separator_p != std::string::npos) {
	    if((separator_p + 1) < entrySize) {
	      map[entry.substr(0, separator_p)] = entry.substr(separator_p + 1);
	    } else {
	      map[entry.substr(0, separator_p)] = "";
	    }
	  }
	}
	start_p = end_p + 1;
      } // while

    } // if

    return map;
    
} // get_map

/** Inserts a map into memory 
  * \param offset current offset of the map 
  * \param map the map to insert 
  * \return the new offset of the map 
  * \note the strings should not contain the caracter STRING_SEPARATOR (03)
  */

System::StringMemoryArea::offset_t System::StringMemoryArea::insert(offset_t offset, const str_map &map) {
  //    ERS_RANGE_CHECK(0, offset, string_area_size());
    str_map::const_iterator pos; 
    std::ostringstream stream; 
    for(pos=map.begin();pos!=map.end();pos++) {
	if (map.begin()!=pos) {
	    stream << MAP_ENTRY_SEPARATOR;
	} // if first
	stream << (pos->first);
	stream  << STRING_SEPARATOR;
	stream << (pos->second);
    } // for
    return insert(offset,stream.str().c_str()); 
} // insert

/** Inserts a vector into memory 
 * This version updates the memory reference
 * \param address of the offset of the string
 * \param str the string to insert
 * \note the strings should not contain the caracter STRING_SEPARATOR (03)
 */
 
void System::StringMemoryArea::insert(offset_t *offset, const str_map &map) {
    ERS_PRECONDITION(offset); 
    const offset_t n_offset = insert(*offset,map); 
    *offset = n_offset; 
} // insert








