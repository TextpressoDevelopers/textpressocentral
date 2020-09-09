/* 
 * File:   PgCurationDataFromViewer.cpp
 * Author: mueller
 * 
 * Created on May 19, 2015, 11:09 AM
 */

#include "PgCurationDataFromViewer.h"
#include <iostream>

PgCurationDataFromViewer::PgCurationDataFromViewer(std::string dbname,
        std::string tablename) : cn_(dbname) {
    tablename_ = tablename;
    CreateCurationDataFromViewerTable();
}

void PgCurationDataFromViewer::CreateCurationDataFromViewerTable() {
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
            pc << "record_id int";
            pc << ", term text";
            pc << ", bestring text";
            pc << ", additional_annotations text";
            pc << ", filename text";
            pc << ", owner varchar(255)";
            pc << ", paper_id text";
            pc << ", last_update int";
            pc << ", version varchar(255)";
            pc << ")";
            r = w.exec(pc.str());
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

PgCurationDataFromViewer::~PgCurationDataFromViewer() {
    cn_.disconnect();
}

void PgCurationDataFromViewer::SaveData(int record_id, std::string term,
        std::string bestring, std::string additionalannotations,
        std::string filename, std::string owner,
        std::string paper_id, time_t last_update, std::string version) {
    bool update = false;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from ";
        pc << tablename_;
        pc << " where record_id=" << record_id;
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
            pc << tablename_ << " ";
            pc << "set (term, bestring, additional_annotations, filename, owner, paper_id, last_update, version) = ";
            pc << "('";
            pc << term << "', '";
            pc << bestring << "', '";
            pc << additionalannotations << "', '";
            pc << filename << "', '";
            pc << owner << "', '";
            pc << paper_id << "', '";
            pc << last_update << "', '";
            pc << version << "')";
            pc << " where record_id=" << record_id;
        } else {
            pc << "insert into ";
            pc << tablename_ << " values (";
            pc << record_id << ",'";
            pc << term << "','";
            pc << bestring << "','";
            pc << additionalannotations << "','";
            pc << filename << "','";
            pc << owner << "','";
            pc << paper_id << "','";
            pc << last_update << "','";
            pc << version << "')";
        }
        r = w.exec(pc.str());
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void PgCurationDataFromViewer::ReadData(const int & record_id,
        std::string & term, std::string & bestring, std::string & additionalannotations,
        std::string & filename, std::string & owner, std::string & paper_id,
        time_t last_update, std::string & version) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select term,bestring,filename,owner,paper_id,last_update,version from ";
        pc << tablename_;
        pc << " where record_id=" << record_id;
        r = w.exec(pc.str());
        if (r.size() == 1) {
            time_t t;
            std::string s;
            if (r[0]["term"].to(s)) term = s;
            if (r[0]["bestring"].to(s)) bestring = s;
            if (r[0]["additional_annotations"].to(s)) additionalannotations = s;
            if (r[0]["filename"].to(s)) filename = s;
            if (r[0]["owner"].to(s)) owner = s;
            if (r[0]["paper_id"].to(s)) paper_id = s;
            if (r[0]["last_update"].to(t)) last_update = t;
            if (r[0]["version"].to(s)) version = s;
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

int PgCurationDataFromViewer::NewRecordId() {
    int recordmax(-1);
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select record_id from ";
        pc << tablename_;
        r = w.exec(pc.str());
        for (int i = 0; i < r.size(); i++) {
            int j;
            if (r[i]["record_id"].to(j)) recordmax = (j > recordmax) ? j : recordmax;
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
    return recordmax + 1;
}
