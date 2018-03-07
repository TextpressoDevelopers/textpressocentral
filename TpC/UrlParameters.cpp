/* 
 * File:   UrlParameters.cpp
 * Author: mueller
 * 
 * Created on February 12, 2016, 1:39 AM
 */

#include "UrlParameters.h"
#include <boost/filesystem.hpp>

UrlParameters::UrlParameters() {
    rootmode_ = false;
    parameters_.clear();
}

UrlParameters::UrlParameters(const Wt::WEnvironment & env) : parameters_(env.getParameterMap()) {
    rootmode_ = false;
    Wt::Http::ParameterValues aux(GetParameterValues("tpcrootpasswd"));
    if (aux.size() > 0)
        if (boost::filesystem::exists("/tmp/" + aux[0])) {
            std::ifstream inp("/tmp/" + aux[0]);
            std::stringstream wd;
            wd << inp.rdbuf();
            inp.close();
            if (wd.str().compare("tpc4ever") == 0)
                rootmode_ = true;
        }
}

void UrlParameters::SetParameterValues(std::string s, Wt::Http::ParameterValues & values) {
    parameters_[s].clear();
    Wt::Http::ParameterValues::iterator it;
    for (it = values.begin(); it != values.end(); it++)
        parameters_[s].push_back(*it);
}

std::vector<std::string> UrlParameters::GetParmVector() {
 std::vector<std::string> vec;
    Wt::Http::ParameterMap::iterator it;
    for (it = parameters_.begin(); it != parameters_.end(); it++) {
        Wt::Http::ParameterValues::iterator it2;
        for (it2 = GetParameterValues((*it).first).begin(); it2 != GetParameterValues((*it).first).end(); it2++)
            vec.push_back((*it).first + "=" + *it2);
    }
    return vec;
} 

std::string UrlParameters::FormURLParameterString() {
    std::vector<std::string> vec(GetParmVector());
    std::string ret("?");
    for (int i = 0; i < vec.size(); i++) {
        ret += vec[i];
        if (i < vec.size() - 1) ret += "&";
    }
    return ret;
}

UrlParameters::UrlParameters(const UrlParameters & orig) {
}

UrlParameters::~UrlParameters() {
}
