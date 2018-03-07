/* 
 * File:   Chunk.h
 * Author: mueller
 *
 * Created on January 30, 2014, 6:36 PM
 */

#ifndef CHUNK_H
#define	CHUNK_H

#include <uima/api.hpp>
#include <sstream>
class Chunk {
public: 
    Chunk(UnicodeString term, int32_t b, int32_t e, std::vector<uima::UnicodeStringRef> & lexvec);
    Chunk();
    Chunk(const Chunk& orig);
    virtual ~Chunk();
    UnicodeString GetChunkTerm() { return chunk_; }
    int32_t GetE() { return e_; }
    int32_t GetB() { return b_; }
    std::vector<uima::UnicodeStringRef> GetLexAnns() { return lexanns_; }
    long unsigned int GetLexAnnSize() { return lexanns_.size(); }
    std::string Print();
private:
    UnicodeString chunk_;
    int32_t b_;
    int32_t e_;
    std::vector<uima::UnicodeStringRef> lexanns_;
};

#endif	/* CHUNK_H */

