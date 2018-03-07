/* 
 * File:   Preference.h
 * Author: mueller
 *
 * Created on March 12, 2015, 2:34 PM
 */

#ifndef PREFERENCE_H
#define	PREFERENCE_H

#include <set>
#include <pqxx/pqxx>
#include <boost/algorithm/string/join.hpp>

class Preference {
public:
    Preference(std::string dbname, std::string tablename,  std::string uid);
    virtual ~Preference();
    void CreatePreferenceTable();
    void SavePreferences(std::string uid, std::set<std::string> & preferences);
    void SavePreferencesVector(const std::string&, const std::vector<std::string>&);
    bool IsPreference(std::string s);
    bool HasPreferences();
    std::set<std::string> GetPreferences();
    std::vector<std::string> GetPreferencesVec();
private:
   // database connection
    pqxx::connection cn_;
    std::string tablename_;
    std::set<std::string> preferences_;
    std::vector<std::string> preferencesVector;
    void LoadPreferencesFromDb(std::string uid);
};

#endif	/* PREFERENCE_H */

