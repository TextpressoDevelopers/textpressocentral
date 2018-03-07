/* 
 * File:   PgColumn.h
 * Author: mueller
 *
 * Created on June 4, 2015, 3:55 PM
 */

#ifndef PGCOLUMN_H
#define	PGCOLUMN_H

#include <set>
#include <pqxx/pqxx>

class PgColumn {
public:
    PgColumn(std::string database, std::string tablename,
            std::string columnname, std::string whereclause);
    virtual ~PgColumn();
    bool IsInList(std::string s);
    bool HasList();
    std::set<std::string> GetList() { return list_;}
private:
    // database connection
    pqxx::connection cn_;
    std::string tablename_;
    std::string columnname_;
    std::string whereclause_;
    std::set<std::string> list_;
    void LoadListFromDb();
};

#endif	/* PGCOLUMN_H */
