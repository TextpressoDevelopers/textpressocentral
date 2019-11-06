/* 
 * File:   TpCategoryBrowser.h
 * Author: mueller
 *
 * Created on March 3, 2015, 11:19 AM
 */

#ifndef TPCATEGORYBROWSER_H
#define	TPCATEGORYBROWSER_H

#include <Wt/WTree>
#include <Wt/WTreeNode>
#include <pqxx/pqxx>

class TpCategoryBrowser : public Wt::WTree {
    typedef std::multimap<std::string, std::string> mmsstype;
    typedef std::pair<mmsstype::iterator, mmsstype::iterator> mmssrange;

//    struct nano {
//        std::string name;
//        Wt::WTreeNode* node;
//    };

public:    
    TpCategoryBrowser(std::set<std::string> preloaded = std::set<std::string>());
    void DeleteRootChildren();
    void SetUpRootChildren(std::set<std::string> preloaded = std::set<std::string>());
    std::set<std::string> GetCategorySet();
    std::set<std::string> GetAllChildrensName(std::string x);
    std::set<std::string> GetAllDirectChildrensName(std::string x);
    std::set<Wt::WString> GetSelected();
    int getTotalNumSubCategories(std::string categoryName);
    int getNumberChildren(std::string x);
    mmsstype GetCat2Ont() { return cat2ont_; }
    ~TpCategoryBrowser();
private:
    // database connection
    mmsstype cat2ont_;
    pqxx::connection cn_;
    void LoadNextChildren(Wt::WTreeNode * x,
            std::set<std::string> preloaded = std::set<std::string>());
};

#endif	/* TPCATEGORYBROWSER_H */
