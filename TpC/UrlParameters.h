/* 
 * File:   UrlParameters.h
 * Author: mueller
 *
 * Created on February 12, 2016, 1:39 AM
 */

#ifndef URLPARAMETERS_H
#define	URLPARAMETERS_H

#include <string>
#include <Wt/WEnvironment>

class UrlParameters {
public:
    UrlParameters();
    UrlParameters(const Wt::WEnvironment & env);
    UrlParameters(const UrlParameters& orig);
    virtual ~UrlParameters();
    std::vector<std::string> GetParmVector();
    std::string FormURLParameterString();
    bool IsRoot() { return rootmode_; }
    bool IsEmpty() { return (parameters_.size() == 1); }
    Wt::Http::ParameterValues & GetParameterValues(std::string s) { return parameters_[s]; }
    void SetParameterValues(std::string s, Wt::Http::ParameterValues & values);
    Wt::Http::ParameterMap GetMap() { return parameters_; }
private:
    bool rootmode_;
    Wt::Http::ParameterMap parameters_;
};

#endif	/* URLPARAMETERS_H */

