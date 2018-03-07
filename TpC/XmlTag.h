/* 
 * File:   XmlTag.h
 * Author: mueller
 *
 * Created on January 29, 2014, 6:53 PM
 */

#ifndef XMLTAG_H
#define	XMLTAG_H

#include <uima/api.hpp>
#include "Chunk.h"

class XmlTag {
public:
    XmlTag();
    XmlTag(uima::ANIterator anit, std::vector<uima::ANIterator> & lexannits);
    XmlTag(const XmlTag & orig);
    virtual ~XmlTag();
    int32_t GetBeginPosition() { return begin_; }
    int32_t GetEndPosition() { return end_; }
    uima::UnicodeStringRef GetValue() { return value_; }
    uima::UnicodeStringRef GetTerm() { return term_; }
    uima::UnicodeStringRef GetContent() { return content_; }
    std::vector<Chunk*> GetChunks() { return chunks_; }
private:
    // the following information should be available:
    // * begin and end position of xml tag;
    int32_t begin_;
    int32_t end_;
    // * value, term and content
    uima::UnicodeStringRef value_;
    uima::UnicodeStringRef term_;
    uima::UnicodeStringRef content_;
    // * a list of chunks with chopped up terms, positions and lexanns
    std::vector<Chunk*> chunks_;
    void ProcessTermSniplet(int32_t old, int32_t curr, std::vector<uima::ANIterator> & lexannits);
};

#endif	/* XMLTAG_H */

