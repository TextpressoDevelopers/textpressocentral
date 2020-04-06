/* 
 * File:   OboFileSegmentation.h
 * Author: mueller
 *
 * Created on October 14, 2013, 12:09 PM
 */

#ifndef OBOFILESEGMENTATION_H
#define	OBOFILESEGMENTATION_H

#include <string>
#include <vector>
#include "OboEntry.h"

typedef std::multimap<std::string, std::string> mms;

class OboFileSegmentation {
public:
    OboFileSegmentation(const char * filename);
    OboFileSegmentation(const OboFileSegmentation & orig);
    virtual ~OboFileSegmentation();
    long unsigned int GetTermSetSize() { return termset_.size(); }
    OboEntry * GetEntry(long unsigned int i) { return termset_[i]; }
    const mms & GetElementaryPcRelationships () { return pcelementaryrelationships_; }
    OboEntry* GetOePtr(std::string id);
    void PrintElementaryPcRelationships(const char * filename);
    void CalculateAndPrintCompositePcRelationships(const char * filename);
    void PrintIdNames (const char * filename);
    void PrintSingleTerms(const char * filename);
    bool IsInSubset(std::string id, std::string subsetname);
//    void PrintCompositePcRelationships(const char * filename);
private:
    std::vector<OboEntry*> termset_;
    //
    std::map<std::string, OboEntry*> id2oboentryptr_;
    //
    mms pcelementaryrelationships_;
    //
    mms ignoredkeyphrases_;
    mms pckeyphrases_;
    mms cpkeyphrases_;
    mms sbkeyphrases_;
    void InitializeKeyphrases();
    void PopulateId2OboEntryPtr();
    void PopulateElementaryPcRelationships();
    std::vector<std::string> ReturnAllChildren(std::string parent);
    void Print2FilePcRelations(std::string fnroot, mms & pcr);
};

#endif	/* OBOFILESEGMENTATION_H */
