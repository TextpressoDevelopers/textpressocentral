/* 
 * File:   TpOntologyBrowser.h
 * Author: mueller
 *
 * Created on September 8, 2014, 2:08 PM
 */

#ifndef TPONTOLOGYBROWSER_H
#define	TPONTOLOGYBROWSER_H

#include <Wt/WTree>

#include "../TextpressoCentralGlobalDefinitions.h"
#include <pqxx/pqxx>

#include <map>

class TpOntologyBrowser : public Wt::WTree {
    typedef std::multimap<std::string, std::string> mmsstype;
    typedef std::pair<mmsstype::iterator, mmsstype::iterator> mmssrange;

//    struct nano {
//        std::string name;
//        Wt::WTreeNode* node;
//    };

public:
    TpOntologyBrowser();

    void SetIncludeChildrenFlag(bool x) {
        includechildren_ = x;
    }

    bool GetIncludeChildrenFlag() {
        return includechildren_;
    }
    
    std::set<std::string> GetCategorySet() { return categorynames_; }
    
    std::set<Wt::WString> GetSelected();
    ~TpOntologyBrowser();
private:
    // database connection
    pqxx::connection cn_;
    mmsstype pcrelations_;
    std::set<std::string> categorynames_;
    bool includechildren_;
    void NodeSelected(Wt::WTreeNode * x);
    void LoadNextChildrenAdvanced(Wt::WTreeNode * x, bool grandchildren);
    void LoadNextChildren(Wt::WTreeNode * x);
    void Read2Cols(std::string t, std::string c1, std::string c2);
};

#endif	/* TPONTOLOGYBROWSER_H */

