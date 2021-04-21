/* 
 * File:   Preference.cpp
 * Author: mueller
 * 
 * Created on March 12, 2015, 2:34 PM
 */

#include "Preference.h"
#include <iostream>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string.hpp>

#include "IndexManager.h"

/*!
 * this class can be used to retrieve generic user preferences from the db
 *
 * table containing preferences must have at least a 'userid' field containing the username of the currently logged in
 * user and a 'preference' field containing the value of the preference for the user. For consistency among tables, the
 * preferences should be strings containing '|'-separated values
 *
 * @param dbname the database name
 * @param tablename the preference table name
 * @param uid the id of the logged in user
 */
Preference::Preference(std::string dbname, std::string tablename, std::string uid) : cn_(dbname) {
    tablename_ = tablename;
    CreatePreferenceTable();
    LoadPreferencesFromDb(uid);
}

/*!
 * create the table if not exists
 */
void Preference::CreatePreferenceTable() {
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    pc << "select * from pg_tables where tablename='";
    pc << tablename_ << "'";
    r = w.exec(pc.str());
    if (r.size() != 0) {
        std::cerr << tablename_ << " table already exists." << std::endl;
    } else {
        pc.str("");
        pc << "create table ";
        pc << tablename_ << " ";
        pc << "(";
        pc << "userid text";
        pc << ", preference text";
        pc << ")";
        r = w.exec(pc.str());
    }
    w.commit();

}

/*!
 * save preferences to database
 *
 * @param uid the id of the user to whom the specified preferences must be associated
 * @param prefs the preferences to be saved, as a set of strings
 */
void Preference::SavePreferences(std::string uid, std::set<std::string> & prefs) {
    std::string pref;
    if (!prefs.empty()) {
        std::set<std::string>::iterator its(prefs.begin()), it;
        pref = *its;
        for (it = ++its; it != prefs.end(); it++)
            pref += "|" + *it;
    } else
        pref = "";
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    if (!preferences_.empty()) {
        pc << "update ";
        pc << tablename_ << " ";
        pc << "set preference='";
        pc << pref << "' ";
        pc << "where userid='";
        pc << uid << "'";
    } else {
        pc << "insert into ";
        pc << tablename_ << " values ('";
        pc << uid << "','" << pref << "')";
    }
    r = w.exec(pc.str());
    w.commit();
}

/*!
 * save preferences to database from a vector of strings
 *
 * @param uid the id of the user to whom the specified preferences must be associated
 * @param prefs the preferences to be saved, as a vector of strings
 */
void Preference::SavePreferencesVector(const std::string& uid, const std::vector<std::string>& prefs) {
    std::string pref;
    if (!prefs.empty()) {
        pref = boost::algorithm::join(prefs, "|");
    } else
        pref = "";
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    if (!preferences_.empty()) {
        pc << "update ";
        pc << tablename_ << " ";
        pc << "set preference='";
        pc << pref << "' ";
        pc << "where userid='";
        pc << uid << "'";
    } else {
        pc << "insert into ";
        pc << tablename_ << " values ('";
        pc << uid << "','" << pref << "')";
    }
    r = w.exec(pc.str());
    w.commit();
}

/*!
 * check if a preference value is already present in the record associated with the previously specified user
 *
 * @param s the value to check
 * @return whether the value is contained in the recorded preference
 */
bool Preference::IsPreference(std::string s) {
    return (preferences_.find(s) != preferences_.end());
}

/*!
 * check if the record associated with the previously specified user has any stored preferences
 *
 * @return whether the record has any preferences
 */
bool Preference::HasPreferences() {
    return !preferences_.empty();
}

/*!
 * get the set of preference values in the record of the previously specified user
 *
 * @return the set of preference values
 */
std::set<std::string> Preference::GetPreferences() {
    return preferences_;
}

/*!
 * get the vector of preference values in the record of the previously specified user
 *
 * @return the vector of preference values
 */
std::vector<std::string> Preference::GetPreferencesVec() {
    return preferencesVector;
}

/*!
 * load the preferences associated with a user from the database
 *
 * @param uid the id of the user
 */
void Preference::LoadPreferencesFromDb(std::string uid) {
//    if (uid.compare("default") == 0) { // set default preferences as available corpora
//        for (const std::string& corpus : tpc::index::IndexManager::get_available_corpora(tpc::index::CAS_ROOT_LOCATION.c_str())) {
//            preferences_.insert(corpus);
//            preferencesVector.push_back(corpus);
//        }
//    } else {
        try {
            pqxx::work w(cn_);
            pqxx::result r;
            std::stringstream pc;
            pc << "select preference from ";
            pc << tablename_ << " ";
            pc << "where userid='" << uid << "'";
            r = w.exec(pc.str());
            for (pqxx::result::size_type i = 0; i != r.size(); i++) {
                std::string cname;
                if (r[i]["preference"].to(cname)) {
                    std::vector<std::string> s1;
                    boost::split(s1, cname, boost::is_any_of("|"));
                    std::vector<std::string>::iterator it;
                    for (it = s1.begin(); it != s1.end(); it++) {
                        preferences_.insert(*it);
                        preferencesVector.push_back(*it);
                    }
                }
            }
            w.commit();
        } catch (const std::exception &e) {
            std::cerr << e.what() << std::endl;
        }
//    }
}

Preference::~Preference() {
    cn_.disconnect();
}

