/* 
 * File:   CurationFieldConfiguration.cpp
 * Author: mueller
 * 
 * Created on April 7, 2015, 10:41 AM
 */

#include "CurationFieldConfiguration.h"
#include <boost/algorithm/string.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

CurationFieldConfiguration::CurationFieldConfiguration(std::string dbname,
        std::string tablename, std::string formname) : cn_(dbname) {
    tablename_ = tablename;
    formname_ = formname;
    CreateCurationFieldConfigurationTable();
    CreateCurationFieldDataTable();
    FindCurrentVersion();
    LoadCurationFieldConfigurationTableFromDb();
}

void CurationFieldConfiguration::FindCurrentVersion() {
    int currentversion = 0;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select formname from ";
        pc << tablename_ << " ";
        pc << "where formname like'" << formname_ << "%'";
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string s;
            if (r[i]["formname"].to(s)) {
                std::vector<std::string> splits;
                boost::split(splits, s, boost::is_any_of("#"));
                int i;
                std::istringstream(splits[splits.size() - 1]) >> i;
                currentversion = (i > currentversion) ? i : currentversion;
            }
        }
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    std::stringstream ss;
    ss << formname_ << "#" << currentversion;
    currentversionformname_ = ss.str();
    std::stringstream ss1;
    currentversion++;
    ss1 << formname_ << "#" << currentversion;
    nextversionformname_ = ss1.str();
}

void CurationFieldConfiguration::CreateCurationFieldConfigurationTable() {
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
            pc << ", formname varchar(128)";
            pc << ", fieldname varchar(128)";
            pc << ", fieldtype int";
            pc << ", x int";
            pc << ", y int";
            pc << ", checkboxdefault bool";
            // sgg table is for autocomplete, val table is for validation
            pc << ", sggcol text";
            pc << ", sggwhereclause text";
            pc << ", sggdatabase text";
            pc << ", sggtablename text";
            pc << ", valcol text";
            pc << ", valwhereclause text";
            pc << ", valdatabase text";
            pc << ", valtablename text";
            pc << ", comboboxchoices text";
            pc << ", prepop_id int";
            pc << ")";
            r = w.exec(pc.str());
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void CurationFieldConfiguration::CreateCurationFieldDataTable() {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from pg_tables where tablename='";
        pc << tablename_ << "_data'";
        r = w.exec(pc.str());
        if (r.size() == 0) {
            pc.str("");
            pc << "create table ";
            pc << tablename_ << "_data ";
            pc << "(";
            pc << "record_id int";
            pc << ", formname varchar(128)";
            pc << ", fieldname varchar(128)";
            pc << ", fieldtype int";
            pc << ", checkboxdata bool";
            pc << ", textdata text";
            pc << ")";
            r = w.exec(pc.str());
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void CurationFieldConfiguration::SaveFieldData(int record, fieldtype fieldtype,
        std::string fieldname, bool d, std::string data) {
    bool update = false;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from ";
        pc << tablename_ << "_data";
        pc << " where (record_id=" << record;
        pc << ") AND (fieldtype=" << fieldtype;
        pc << ") AND (fieldname='" << fieldname << "')";
        r = w.exec(pc.str());
        update = (r.size() != 0);
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        if (update) {
            pc << "update ";
            pc << tablename_ << "_data ";
            pc << "set (checkboxdata, textdata) = ";
            pc << "('";
            pc << d << "', '";
            pc << data << "')";
            pc << " where (record_id=" << record;
            pc << ") AND (formname='" << currentversionformname_ << "'";
            pc << ") AND (fieldtype=" << fieldtype;
            pc << ") AND (fieldname='" << fieldname << "')";
        } else {
            pc << "insert into ";
            pc << tablename_ << "_data values ('";
            pc << record << "','";
            pc << currentversionformname_ << "','";
            pc << fieldname << "','";
            pc << fieldtype << "','";
            pc << d << "','";
            pc << data << "')";
        }
        r = w.exec(pc.str());
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void CurationFieldConfiguration::ReadFieldData(const int & record,
        const fieldtype & fieldtype, const std::string & fieldname,
        bool & d, std::string & data) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select checkboxdata,textdata from ";
        pc << tablename_ << "_data";
        pc << " where (record_id=" << record;
        pc << ") AND (fieldtype=" << fieldtype;
        pc << ") AND (fieldname='" << fieldname << "')";
        r = w.exec(pc.str());
        if (r.size() == 1) {
            bool b;
            std::string s;
            if (r[0]["checkboxdata"].to(b)) d = b;
            if (r[0]["textdata"].to(s)) data = s;
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void CurationFieldConfiguration::LoadCurationFieldConfigurationTableFromDb() {
    try {
        fieldname_.clear();
        fieldtype_.clear();
        x_.clear();
        y_.clear();
        suggestionboxinfo_.clear();
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from ";
        pc << tablename_ << " ";
        pc << "where formname='" << currentversionformname_ << "'";
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string s;
            int x;
            bool b;
            if (r[i]["fieldname"].to(s)) fieldname_.push_back(s);
            if (r[i]["fieldtype"].to(x)) fieldtype_.push_back(fieldtype(x));
            if (r[i]["x"].to(x)) x_.push_back(x);
            if (r[i]["y"].to(x)) y_.push_back(x);
            if (r[i]["checkboxdefault"].to(b)) checkboxdefault_.push_back(b);
            pginfo sgginfo;
            if (r[i]["sggcol"].to(s)) sgginfo.col = s;
            if (r[i]["sggwhereclause"].to(s)) sgginfo.whereclause = s;
            if (r[i]["sggdatabase"].to(s)) sgginfo.database = s;
            if (r[i]["sggtablename"].to(s)) sgginfo.tablename = s;
            suggestionboxinfo_.push_back(sgginfo);
            bool sgallempty = (sgginfo.col.empty() && sgginfo.database.empty()
                    && sgginfo.tablename.empty() && sgginfo.whereclause.empty());
            hassuggestionboxinfo_.push_back(!sgallempty);
            pginfo valinfo;
            if (r[i]["valcol"].to(s)) valinfo.col = s;
            if (r[i]["valwhereclause"].to(s)) valinfo.whereclause = s;
            if (r[i]["valdatabase"].to(s)) valinfo.database = s;
            if (r[i]["valtablename"].to(s)) valinfo.tablename = s;
            validationinfo_.push_back(valinfo);
            bool vlallempty = (valinfo.col.empty() && valinfo.database.empty()
                    && valinfo.tablename.empty() && valinfo.whereclause.empty());
            hasvalidationinfo_.push_back(!vlallempty);
            if (r[i]["comboboxchoices"].to(s)) comboboxchoices_.push_back(s);
            if (r[i]["prepop_id"].to(x)) prepopids_.push_back(x);
        }
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
}

void CurationFieldConfiguration::SaveField(std::string fieldname, fieldtype ft,
        int x, int y, bool b, pginfo s, pginfo v, std::string combochoices, int prepopid) {
    if (!fieldname.empty()) {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "insert into ";
        pc << tablename_ << " values (DEFAULT,'";
        pc << nextversionformname_ << "','";
        pc << fieldname << "','";
        pc << ft << "','";
        pc << x << "','";
        pc << y << "','";
        pc << b << "','";
        pc << s.col << "','";
        pc << s.whereclause << "','";
        pc << s.database << "','";
        pc << s.tablename << "','";
        pc << v.col << "','";
        pc << v.whereclause << "','";
        pc << v.database << "','";
        pc << v.tablename << "','";
        pc << combochoices << "','";
        pc << prepopid << "')";
        r = w.exec(pc.str());
        w.commit();
    }
}

bool CurationFieldConfiguration::HasFields() {
    return !fieldname_.empty();
}

long unsigned int CurationFieldConfiguration::NumberOfFields() {
    return fieldname_.size();
}

std::string CurationFieldConfiguration::GetFieldName(long unsigned int i) {
    return fieldname_[i];
}

CurationFieldConfiguration::fieldtype CurationFieldConfiguration::GetFieldType(long unsigned int i) {
    return fieldtype_[i];
}

int CurationFieldConfiguration::GetX(long unsigned int i) {
    return x_[i];
}

int CurationFieldConfiguration::GetY(long unsigned int i) {
    return y_[i];
}

bool CurationFieldConfiguration::GetCheckboxDefault(long unsigned int i) {
    return checkboxdefault_[i];
}

CurationFieldConfiguration::pginfo CurationFieldConfiguration::GetSuggestionBoxInfo(long unsigned int i) {
    return suggestionboxinfo_[i];
}

CurationFieldConfiguration::pginfo CurationFieldConfiguration::GetValidationInfo(long unsigned int i) {
    return validationinfo_[i];
}

bool CurationFieldConfiguration::HasSuggestionBoxInfo(long unsigned int i) {
    return hassuggestionboxinfo_[i];
}

bool CurationFieldConfiguration::HasValidationInfo(long unsigned int i) {
    return hasvalidationinfo_[i];
}

std::string CurationFieldConfiguration::GetComboChoice(long unsigned int i) {
    return comboboxchoices_[i];
}

int CurationFieldConfiguration::GetPrePopId(long unsigned int i) {
    return prepopids_[i];
}

std::string CurationFieldConfiguration::GetCurrentVersionFormName() {
    return currentversionformname_;
}

CurationFieldConfiguration::~CurationFieldConfiguration() {
    cn_.disconnect();
}
