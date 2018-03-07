/* 
 * File:   PdfTag.h
 * Author: mueller
 *
 * Created on June 9, 2014, 4:07 PM
 */

#ifndef PDFTAG_H
#define	PDFTAG_H

#include <uima/api.hpp>

class PdfTag {
public:
    PdfTag(uima::ANIterator anit);
    virtual ~PdfTag();

    int32_t GetBeginPosition() {
        return begin_;
    }

    int32_t GetEndPosition() {
        return end_;
    }

    uima::UnicodeStringRef GetTagType() {
        return tagtype_;
    }

    uima::UnicodeStringRef GetValue() {
        return value_;
    }
private:
    // the following information should be available:
    // * begin and end position of pdftag;
    int32_t begin_;
    int32_t end_;
    // * tagtype and value
    uima::UnicodeStringRef tagtype_;
    uima::UnicodeStringRef value_;
};

#endif	/* PDFTAG_H */

