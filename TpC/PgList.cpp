/* 
 * File:   PgList.cpp
 * Author: mueller
 * 
 * Created on March 30, 2015, 4:36 PM
 */

#include "PgList.h"
#include <iostream>
#include <boost/algorithm/string.hpp>

PgList::PgList(std::string dbname, std::string tablename) : cn_(dbname) {
    srand(time(NULL));
    tablename_ = tablename;
    CreateListTable();
    LoadListFromDb();
}

void PgList::CreateListTable() {
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
        pc << "list text";
        pc << ")";
        r = w.exec(pc.str());
    }
    w.commit();
}

void PgList::SaveList(std::set<std::string> & list) {
    if (!list.empty()) {
        pqxx::work w(cn_);
        pqxx::result r;
        std::set<std::string>::iterator it;
        for (it = list.begin(); it != list.end(); it++) {
            if (list_.find(*it) == list_.end()) {
                list_.insert(*it);
                std::stringstream pc;
                pc << "insert into ";
                pc << tablename_ << " values ('";
                pc << *it << "')";
                r = w.exec(pc.str());
            }
        }
        w.commit();
    }
}

void PgList::AddItem(std::string item) {
    if (!item.empty()) {
        pqxx::work w(cn_);
        pqxx::result r;
        std::set<std::string>::iterator it;
        if (list_.find(item) == list_.end()) {
            list_.insert(item);
            std::stringstream pc;
            pc << "insert into ";
            pc << tablename_ << " values ('";
            pc << item << "')";
            r = w.exec(pc.str());
        }
        w.commit();
    }
}

std::string PgList::GetRandomItem() {
    if (!list_.empty()) {
        int size = list_.size();
        int pick = rand() % size;
        std::set<std::string>::iterator it;
        int count = 0;
        for (it = list_.begin(); it != list_.end(); it++) {
            if (count++ != pick) continue;
            return *it;
        }
    } else
        return ("No item available.");
}

bool PgList::IsInList(std::string s) {
    return (list_.find(s) != list_.end());
}

bool PgList::HasList() {
    return !list_.empty();
}

void PgList::LoadListFromDb() {
    try {
        pqxx::work w(cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select list from ";
        pc << tablename_;
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string cname;
            if (r[i]["list"].to(cname))
                list_.insert(cname);
        }
        w.commit();
    } catch (const std::exception & e) {
        std::cerr << e.what() << std::endl;
    }
}

PgList::~PgList() {
    cn_.disconnect();
}
