/* 
 * File:   XmlTag.cpp
 * Author: mueller
 * 
 * Created on January 29, 2014, 6:53 PM
 */

#include <map>
#include <vector>

#include "XmlTag.h"

XmlTag::XmlTag() {

}

XmlTag::XmlTag(uima::ANIterator anit, std::vector<uima::ANIterator> & lexannits) {
    // fill trivial variables;
    begin_ = anit.get().getBeginPosition();
    end_ = anit.get().getEndPosition();
    uima::Type t = anit.get().getType();
    uima::Feature f = t.getFeatureByBaseName("value");
    value_ = anit.get().getStringValue(f);
    f = t.getFeatureByBaseName("term");
    term_ = anit.get().getStringValue(f);
    f = t.getFeatureByBaseName("content");
    content_ = anit.get().getStringValue(f);
    // deal with chunks_ and lexanns_;
    if (term_.length() > 0) {
        int32_t curr = 0;
        int32_t old = 0;
        curr = term_.indexOf(' ', old);
        while ((curr > 0) && (curr < end_ - begin_)) {
            ProcessTermSniplet(old, curr, lexannits);
            old = curr + 1;
            curr = term_.indexOf(' ', old);
        }
        // take care of last element
        ProcessTermSniplet(old, term_.length(), lexannits);
    }
}

XmlTag::XmlTag(const XmlTag & orig) {
}

XmlTag::~XmlTag() {
    while (!chunks_.empty()) {
        delete chunks_.back();
        chunks_.pop_back();
    }
}

void XmlTag::ProcessTermSniplet(int32_t old, int32_t curr, std::vector<uima::ANIterator> & lexannits) {
    UnicodeString extract;
    term_.extract(old, curr - old, extract);
    if (extract.length() > 0) {
        int32_t chunkb = begin_ + old;
        int32_t chunke = begin_ + curr;
        std::vector<uima::UnicodeStringRef> lexvec;
        std::vector<uima::ANIterator>::iterator j;
        for (j = lexannits.begin(); j != lexannits.end(); j++) {
            int32_t lexb = (*j).get().getBeginPosition();
            int32_t lexe = (*j).get().getEndPosition();
            // overlap exists if (StartA <= EndB) and (EndA >=StartB)
            if (chunke >= lexb) {
                if (chunkb <= lexe) {
                    uima::Type t = (*j).get().getType();
                    uima::Feature f = t.getFeatureByBaseName("category");
                    uima::UnicodeStringRef catname = (*j).get().getStringValue(f);
                    lexvec.push_back(catname);
                }
            }
        }
        Chunk * c = new Chunk(extract, chunkb, chunke, lexvec);
        chunks_.push_back(c);
    }
}