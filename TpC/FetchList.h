/* 
 * File:   FetchList.h
 * Author: mueller
 *
 * Created on March 22, 2016, 2:28 PM
 */

#ifndef FETCHLIST_H
#define	FETCHLIST_H

#include <set>
#include <pqxx/pqxx>
#include <Wt/Http/Client>

class FetchList {
public:
    FetchList(std::string location, std::string tablename,
            std::string columnname, std::string whereclause);
    virtual ~FetchList();
    bool IsInList(std::string s);
    bool HasList();
    std::set<std::string> GetList() { return list_;}
private:
 // database connection
    Wt::Http::Client * client_;
    pqxx::connection * cn_;
    std::string url_;
    std::string tablename_;
    std::string columnname_;
    std::string whereclause_;
    std::set<std::string> list_;
    bool ispg_;
    void LoadList(std::string s);
    void HandleHttpResponse(boost::system::error_code err, const Wt::Http::Message & response);
};

#endif	/* FETCHLIST_H */
