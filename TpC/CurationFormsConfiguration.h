/* 
 * File:   CurationFormsConfiguration.h
 * Author: mueller
 *
 * Created on April 16, 2015, 12:25 PM
 */

#ifndef CURATIONFORMSCONFIGURATION_H
#define	CURATIONFORMSCONFIGURATION_H

#include <pqxx/pqxx>

class CurationFormsConfiguration {
public:

    enum parameterformat {
        JSON, URI
    };
    CurationFormsConfiguration(std::string dbname, std::string tablename);
    virtual ~CurationFormsConfiguration();

    long unsigned int GetTableSize() {
        return formname_.size();
    }
    bool FoundFormName(std::string s);

    std::string GetFormName(long unsigned int i) {
        return formname_[i];
    }

    std::string GetOwner(long unsigned int i) {
        return owner_[i];
    }

    std::string GetPrivileges(long unsigned int i) {
        return privileges_[i];
    }

    std::string GetPostUrl(long unsigned int i) {
        return posturl_[i];
    }

    long unsigned int GetParameterFormat(long unsigned int i) {
        return format_[i];
    }

    std::string GetFormatName(long unsigned int i) {
        return formatnames[i];
    }
    
    long unsigned int GetNumberOfFormatNames() {
        return formatnames.size();
    }

    void SaveForm(std::string formname, std::string owner,
            std::string privileges, std::string posturl, std::string format);
private:
    // database connection
    pqxx::connection cn_;
    std::string tablename_;
    std::vector<std::string> formname_;
    std::vector<std::string> owner_;
    std::vector<std::string> privileges_;
    std::vector<std::string> posturl_;
    std::vector<int> format_;
    void CreateCurationFormsConfigurationTable();
    void LoadCurationFormsConfigurationTableFromDb();
    std::vector<std::string> formatnames;
};

#endif	/* CURATIONFORMSCONFIGURATION_H */