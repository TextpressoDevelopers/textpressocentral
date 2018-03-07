/* 
 * File:   PgColorPick.cpp
 * Author: mueller
 * 
 * Created on January 27, 2016, 11:06 AM
 */

#include "PgColorPick.h"
#include <iostream>

PgColorPick::PgColorPick(std::string dbname, std::string tablename, std::string username) : cn_(dbname), tablename_(tablename) {
    CreateColorPickTable();
}

void PgColorPick::CreateColorPickTable() {
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
            pc << "userid text";
            pc << ", colorid int";
            pc << ", red int";
            pc << ", green int";
            pc << ", blue int";
            pc << ")";
            r = w.exec(pc.str());
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

void PgColorPick::ReadTable() {
}

void PgColorPick::SaveTable() {
}

PgColorPick::PgColorPick(const PgColorPick& orig) {
}

PgColorPick::~PgColorPick() {
}

