/* 
 * File:   TpOntEntry.cpp
 * Author: mueller
 * 
 * Created on May 29, 2013, 12:15 PM
 */

#include <vector>

#include "TpOntEntry.h"
#include <boost/algorithm/string.hpp>

TpOntEntry::TpOntEntry() {
}

TpOntEntry::TpOntEntry(const TpOntEntry & orig) {
}

void TpOntEntry::SetAnnotationType(std::string saux) {
    boost::algorithm::to_lower(saux);
    if (saux.find("lex") != std::string::npos)
        anntype_ = "lexical";
    else if (saux.find("man") != std::string::npos)
        anntype_ = "manual";
    else if (saux.find("comp") != std::string::npos)
        anntype_ = "computational";
}

void TpOntEntry::SetStatus(std::string saux) {
    boost::to_lower(saux);
    if (saux.find("preliminary") != std::string::npos)
        status_ = "preliminary";
    else if (saux.find("exp") != std::string::npos)
        status_ = "experimental";
    else if (saux.find("fin") != std::string::npos)
        status_ = "final";
}


void TpOntEntry::PopulateColumn(std::string mheader, std::string data) {
    if (mheader.compare("eid") == 0)
        SetEntryId(data);
    else if (mheader.compare("dbxref") == 0)
        SetDbXref(data);
    else if (mheader.compare("owner") == 0)
        SetOwner(data);
    else if (mheader.compare("source") == 0)
        SetSource(data);
    else if (mheader.compare("version") == 0)
        SetVersion(data);
    else if (mheader.compare("term") == 0)
        SetTerm(data);
    else if (mheader.compare("category") == 0)
        SetCategory(data);
    else if (mheader.compare("attributes") == 0)
        SetAttributes(data);
    else if (mheader.compare("annotationtype") == 0)
        SetAnnotationType(data);
    else if (mheader.compare("lexicalvariations") == 0)
        SetLexicalVariations(data);
    else if (mheader.compare("last_update") == 0)
        UpdateLastUpdateTime();
    else if (mheader.compare("curation_status") == 0)
        SetStatus(data);
    else if (mheader.compare("curation_use") == 0)
        SetUse(data);
    else if (mheader.compare("comment") == 0)
        SetComment(data);
}
