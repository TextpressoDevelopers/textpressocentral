/* 
 * File:   PgColumn.cpp
 * Author: mueller
 * 
 * Created on June 4, 2015, 3:55 PM
 */

#include "PgColumn.h"
#include <iostream>

PgColumn::PgColumn(std::string database, std::string tablename,
        std::string columnname, std::string whereclause) : cn_(database) {
    tablename_ = tablename;
    columnname_ = columnname;
    whereclause_ = whereclause;
    LoadListFromDb();

}

PgColumn::~PgColumn() {
    cn_.disconnect();
}

bool PgColumn::IsInList(std::string s) {
    return (list_.find(s) != list_.end());
}

bool PgColumn::HasList() {
    return !list_.empty();
}

void PgColumn::LoadListFromDb() {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select ";
        pc << columnname_;
        pc << " from ";
        pc << tablename_;
        if (!whereclause_.empty()) {
            pc << " where ";
            pc << whereclause_;
        }
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string cname;
            if (r[i][columnname_].to(cname)) {
                list_.insert(cname);
            }
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}
