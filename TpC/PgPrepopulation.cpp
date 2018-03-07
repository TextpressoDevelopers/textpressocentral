/* 
 * File:   PgPrepopulation.cpp
 * Author: mueller
 * 
 * Created on July 17, 2015, 2:23 PM
 */

#include "PgPrepopulation.h"
#include <iostream>

PgPrepopulation::PgPrepopulation(std::string dbname, std::string tablename)
: cn_(dbname), tablename_(tablename) {
    CreatePrepopulationTable();
}

int PgPrepopulation::GetNewPrePopId() {
    int recordmax(-1);
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select prepop_id from ";
        pc << tablename_;
        r = w.exec(pc.str());
        for (int i = 0; i < r.size(); i++) {
            int j;
            if (r[i]["prepop_id"].to(j)) recordmax = (j > recordmax) ? j : recordmax;
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
    return recordmax + 1;
}

void PgPrepopulation::SaveTableData(int prepop_id, int mode, std::string data,
        std::string syndatabase, std::string syntablename,
        std::string syncolname, std::string synwhereclause) {
    bool update = false;
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select * from ";
        pc << tablename_;
        pc << " where prepop_id=" << prepop_id;
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
            pc << "set (mode, prepop_data, syndatabase, syntablename, syncolname, synwhereclause) = ";
            pc << "('";
            pc << mode << "', '";
            pc << data << "', '";
            pc << syndatabase << "', '";
            pc << syntablename << "', '";
            pc << syncolname << "', '";
            pc << synwhereclause << "')";
            pc << " where prepop_id=" << prepop_id;
        } else {
            pc << "insert into ";
            pc << tablename_ << " values (";
            pc << prepop_id << ",'";
            pc << mode << "','";
            pc << data << "','";
            pc << syndatabase << "','";
            pc << syntablename << "','";
            pc << syncolname << "','";
            pc << synwhereclause << "')";
        }
        r = w.exec(pc.str());
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void PgPrepopulation::ReadTableData(int prepop_id, int & mode, std::string & data,
        std::string & syndatabase, std::string & syntablename,
        std::string & syncolname, std::string & synwhereclause) {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select mode,prepop_data,syndatabase,syntablename,syncolname,synwhereclause from ";
        pc << tablename_;
        pc << " where prepop_id=" << prepop_id;
        r = w.exec(pc.str());
        if (r.size() == 1) {
            int x;
            std::string s;
            if (r[0]["mode"].to(x)) mode = x;
            if (r[0]["prepop_data"].to(s)) data = s;
            if (r[0]["syndatabase"].to(s)) syndatabase = s;
            if (r[0]["syntablename"].to(s)) syntablename = s;
            if (r[0]["syncolname"].to(s)) syncolname = s;
            if (r[0]["synwhereclause"].to(s)) synwhereclause = s;
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void PgPrepopulation::CreatePrepopulationTable() {
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
            pc << "prepop_id int";
            pc << ", mode int";
            pc << ", prepop_data text";
            // synonym info is in a remote database table
            pc << ", syndatabase text";
            pc << ", syntablename text";
            pc << ", syncolname text";
            pc << ", synwhereclause text";
            pc << ")";
            r = w.exec(pc.str());
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

PgPrepopulation::~PgPrepopulation() {
    cn_.disconnect();
}
