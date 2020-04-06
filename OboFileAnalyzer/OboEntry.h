/* 
 * File:   OboEntry.h
 * Author: mueller
 *
 * Created on October 14, 2013, 4:08 PM
 */

#ifndef OBOENTRY_H
#define	OBOENTRY_H

#include <string>
#include <vector>
#include <map>
#include <set>

typedef std::multimap<std::string, std::string> mmss;
class OboEntry {
public:
    OboEntry(std::string s);
    OboEntry(const OboEntry & orig);
    virtual ~OboEntry();
    std::vector<std::string> GetData(std::string inp);
    void SetData(std::string key, std::string value);
    std::vector<std::string> GetKeys();
    std::string PrintEntry2String();
    std::string GetName(std::string key) { return id_name_[key]; }
    bool IsInSubset(const std::string subsetname);
    bool IsInSubsets(const std::set<std::string> & subsetnames);
private:
    mmss mmdata_;
    std::map<std::string, std::string> id_name_;
};

#endif	/* OBOENTRY_H */

