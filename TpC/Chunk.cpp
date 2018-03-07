/* 
 * File:   Chunk.cpp
 * Author: mueller
 * 
 * Created on January 30, 2014, 6:36 PM
 */

#include <vector>

#include "Chunk.h"

Chunk::Chunk(UnicodeString term, int32_t b, int32_t e, std::vector<uima::UnicodeStringRef> & lexvec) {
    chunk_ = term;
    b_ = b;
    e_ = e;
    std::vector<uima::UnicodeStringRef>::iterator i;
    for (i = lexvec.begin(); i != lexvec.end(); i++) lexanns_.push_back(*i);
}

Chunk::Chunk() {
}

Chunk::Chunk(const Chunk& orig) {
}

Chunk::~Chunk() {
}

std::string Chunk::Print() {
    std::stringstream ret;
    ret << "B: ";
    ret << b_;
    ret << ", E: ";
    ret << e_;
    ret << ", chunkterm: ";
    ret << uima::UnicodeStringRef(chunk_).asUTF8();
    if (!lexanns_.empty()) {
        ret << ", lexanns: ";
        std::vector<uima::UnicodeStringRef>::iterator i = lexanns_.begin();
        ret << (*i).asUTF8();
        while (++i != lexanns_.end()) ret << ", " << (*i).asUTF8();
    }
    return ret.str();
}
