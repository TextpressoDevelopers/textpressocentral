/* 
 * File:   PdfToken.h
 * Author: mueller
 *
 * Created on June 9, 2014, 4:37 PM
 */

#ifndef PDFTOKEN_H
#define	PDFTOKEN_H

#include <uima/api.hpp>

class PdfToken {
public:
    PdfToken(uima::ANIterator anit);
    virtual ~PdfToken();

    int32_t GetBeginPosition() {
        return begin_;
    }

    int32_t GetEndPosition() {
        return end_;
    }

    uima::UnicodeStringRef GetContent() {
        return content_;
    }
private:
    // the following information should be available:
    // * begin and end position of pdftag;
    int32_t begin_;
    int32_t end_;
    // * content
    uima::UnicodeStringRef  content_;
};

#endif	/* PDFTOKEN_H */

