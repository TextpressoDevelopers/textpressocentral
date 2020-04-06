/* 
 * File:   OboEntry.cpp
 * Author: mueller
 * 
 * Created on October 14, 2013, 4:08 PM
 */

#include "OboEntry.h"
#include <boost/algorithm/string.hpp>
#include <sstream>

OboEntry::OboEntry(std::string s) {
    std::vector<std::string> splits;
    boost::split(splits, s, boost::is_any_of("\n"));
    while (!splits.empty()) {
        std::string saux = splits.back();
        int i = saux.find_first_of(':');
        std::string key = saux.substr(0, i);
        std::string content = saux.substr(i + 1, saux.length() - i - 1);
        boost::trim(key);
        boost::trim(content);
        mmdata_.insert(std::make_pair(key, content));
        splits.pop_back();
    }
    std::vector<std::string> ids = GetData("id");
    std::vector<std::string> names = GetData("name");
    if (!names.empty() && !ids.empty())
        id_name_.insert(std::make_pair(ids[0], names[0]));
}

OboEntry::OboEntry(const OboEntry & orig) {
}

OboEntry::~OboEntry() {
}

std::vector<std::string> OboEntry::GetKeys() {
    std::vector<std::string> ret;
    mmss::iterator mmsit;
    for (mmsit = mmdata_.begin(); mmsit != mmdata_.end(); mmsit++)
        ret.push_back((*mmsit).first);

    return ret;
}

std::string OboEntry::PrintEntry2String() {
    std::stringstream ret;
    mmss::iterator mmsit;
    for (mmsit = mmdata_.begin(); mmsit != mmdata_.end(); mmsit++)
        if (!(((*mmsit).second).empty()))
            ret << (*mmsit).first << ": " << (*mmsit).second << std::endl;
    return ret.str();
}

std::vector<std::string> OboEntry::GetData(std::string inp) {
    std::vector<std::string> ret;
    std::pair<mmss::iterator, mmss::iterator> mmsii = mmdata_.equal_range(inp);
    mmss::iterator mmsit;
    for (mmsit = mmsii.first; mmsit != mmsii.second; mmsit++)
        ret.push_back((*mmsit).second);
    return ret;
}

void OboEntry::SetData(std::string key, std::string value) {
    mmdata_.insert(std::make_pair(key, value));
}

bool OboEntry::IsInSubset(const std::string subsetname) {
    std::vector<std::string> subsetdata(GetData("subset"));
    return (std::find(subsetdata.begin(), subsetdata.end(), subsetname) != subsetdata.end());
}

bool OboEntry::IsInSubsets(const std::set<std::string> & subsetnames) {
    std::vector<std::string> subsetdata(GetData("subset"));
    for (auto it = subsetnames.begin(); it != subsetnames.end(); it++)
        if (std::find(subsetdata.begin(), subsetdata.end(), *it) != subsetdata.end())
            return true;
    return false;
}