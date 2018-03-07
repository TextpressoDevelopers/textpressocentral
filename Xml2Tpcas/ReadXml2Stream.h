/* 
 * File:   ReadXml2Stream.h
 * Author: mueller
 *
 * Created on April 29, 2013, 10:28 AM
 */

#ifndef READXML2STREAM_H
#define	READXML2STREAM_H

#include "pugixml.hpp"
#include "sstream"

class ReadXml2Stream {
public:
    ReadXml2Stream(const pugi::char_t * pszSource);
    ReadXml2Stream();
    ReadXml2Stream(const ReadXml2Stream & orig);
    void GetStream (std::stringstream & s) { s.clear(); s << sout_.rdbuf();};
    virtual ~ReadXml2Stream();
private:
    void TraverseTree(pugi::xml_node pnode);
    std::stringstream sout_;

};

#endif	/* READXML2STREAM_H */

