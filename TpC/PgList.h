/* 
 * File:   PgList.h
 * Author: mueller
 *
 * Created on March 30, 2015, 4:36 PM
 */

#ifndef PGLIST_H
#define	PGLIST_H

#include <set>
#include <pqxx/pqxx>

class PgList {
public:
    PgList(std::string dbname, std::string tablename);
    virtual ~PgList();
    void CreateListTable();
    void SaveList(std::set<std::string> & preferences);
    void AddItem(std::string item);
    std::string GetRandomItem();
    bool IsInList(std::string s);
    bool HasList();
    std::set<std::string> GetList() { return list_; }
private:
   // database connection
    pqxx::connection cn_;
    std::string tablename_;
    std::set<std::string> list_;
    void LoadListFromDb();
};

#endif	/* PGLIST_H */
