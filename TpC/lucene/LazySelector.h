/**
    Project: textpressocentral
    File name: LazySelector.h
    
    @author valerio
    @version 1.0 6/10/17.
*/

#ifndef TEXTPRESSOCENTRAL_LAZYSELECTOR_H
#define TEXTPRESSOCENTRAL_LAZYSELECTOR_H

#include <lucene++/LuceneHeaders.h>

DECLARE_SHARED_PTR(LazySelector);
class LazySelector : public FieldSelector {
public:
    LazySelector(const String& magicField) {
        this->magicField = magicField;
    }
    virtual ~LazySelector() {
    }
    LUCENE_CLASS(LazySelector);
protected:
    String magicField;

public:
    virtual FieldSelectorResult accept(const String& fieldName) {
        if (fieldName == magicField) {
            return FieldSelector::SELECTOR_LOAD;
        } else {
            return FieldSelector::SELECTOR_NO_LOAD;
        }
    }
};

#endif //TEXTPRESSOCENTRAL_LAZYSELECTOR_H
