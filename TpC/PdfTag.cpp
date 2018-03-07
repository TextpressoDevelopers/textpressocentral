/* 
 * File:   PdfTag.cpp
 * Author: mueller
 * 
 * Created on June 9, 2014, 4:07 PM
 */

#include "PdfTag.h"

PdfTag::PdfTag(uima::ANIterator anit) {
    begin_ = anit.get().getBeginPosition();
    end_ = anit.get().getEndPosition();
    uima::Type t = anit.get().getType();
    uima::Feature f = t.getFeatureByBaseName("tagtype");
    if (f.isValid())
        tagtype_ = anit.get().getStringValue(f);
    f = t.getFeatureByBaseName("value");
    if (f.isValid())
        value_ = anit.get().getStringValue(f);
}

PdfTag::~PdfTag() {
}

