/* 
 * File:   CurationFormsConfiguration.cpp
 * Author: mueller
 * 
 * Created on April 16, 2015, 12:25 PM
 */

#include "CurationFormsConfiguration.h"
#include <iostream>
#include <algorithm>

CurationFormsConfiguration::CurationFormsConfiguration(std::string dbname,
        std::string tablename) : cn_(dbname) {
    tablename_ = tablename;
    formatnames.push_back("JSON"); // parameterformat::JSON
    formatnames.push_back("URI");  // parameterformat::URI
    CreateCurationFormsConfigurationTable();
    LoadCurationFormsConfigurationTableFromDb();
}

void CurationFormsConfiguration::CreateCurationFormsConfigurationTable() {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from pg_tables where tablename='";
        pc << tablename_ << "'";
        r = w.exec(pc.str());
        if (r.size() == 0) {
            pc.str("");
            pc << "create table ";
            pc << tablename_ << " ";
            pc << "(";
            pc << "iid serial primary key";
            pc << ",formname varchar(128)";
            pc << ", owner varchar(128)";
            pc << ", privileges text";
            pc << ", posturl text";
            pc << ", parameter_format int";
            pc << ")";
            r = w.exec(pc.str());
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }

}

void CurationFormsConfiguration::SaveForm(std::string formname,
        std::string owner, std::string privileges, std::string posturl, std::string format) {
    int iformat;
    if (format.compare("JSON") == 0)
        iformat = parameterformat::JSON;
    else if (format.compare("URI") == 0)
        iformat = parameterformat::URI;
    pqxx::work w(cn_);
    pqxx::result r;
    std::stringstream pc;
    if (FoundFormName(formname)) {
        pc << "update ";
        pc << tablename_ << " ";
        pc << "set (owner,privileges,posturl,parameter_format)=('";
        pc << owner << "','";
        pc << privileges << "','";
        pc << posturl << "','";
        pc << iformat << "') ";
        pc << "where formname='";
        pc << formname << "'";
    } else {
        pc << "insert into ";
        pc << tablename_ << " values (DEFAULT,'" << formname << "','" << owner << "','";
        pc << privileges << "','" << posturl << "','" << iformat << "')";
    }
    r = w.exec(pc.str());
    w.commit();
    LoadCurationFormsConfigurationTableFromDb();
}

void CurationFormsConfiguration::LoadCurationFormsConfigurationTableFromDb() {
    try {

        formname_.clear();
        owner_.clear();
        privileges_.clear();
        posturl_.clear();
        format_.clear();
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from ";
        pc << tablename_;
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string s;
            if (r[i]["formname"].to(s)) formname_.push_back(s);
            if (r[i]["owner"].to(s)) owner_.push_back(s);
            if (r[i]["privileges"].to(s)) privileges_.push_back(s);
            if (r[i]["posturl"].to(s)) posturl_.push_back(s);
            int j;
            if (r[i]["parameter_format"].to(j)) format_.push_back(j);
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

bool CurationFormsConfiguration::FoundFormName(std::string s) {
    return (std::find(formname_.begin(), formname_.end(), s) != formname_.end());
}

CurationFormsConfiguration::~CurationFormsConfiguration() {
    cn_.disconnect();
}
