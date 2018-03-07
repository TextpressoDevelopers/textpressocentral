/* 
 * File:   PdfToken.cpp
 * Author: mueller
 * 
 * Created on June 9, 2014, 4:37 PM
 */

#include "PdfToken.h"

PdfToken::PdfToken(uima::ANIterator anit) {
    begin_ = anit.get().getBeginPosition();
    end_ = anit.get().getEndPosition();
    uima::Type t = anit.get().getType();
    uima::Feature f = t.getFeatureByBaseName("content");
    if (f.isValid())
        content_ = anit.get().getStringValue(f);
}

PdfToken::~PdfToken() {
}

