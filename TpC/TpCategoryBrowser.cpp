/* 
 * File:   TpCategoryBrowser.cpp
 * Author: mueller
 * 
 * Created on March 3, 2015, 11:19 AM
 */

#include "TpCategoryBrowser.h"
#include "PgList.h"
#include "../TextpressoCentralGlobalDefinitions.h"
#include <Wt/WText>
#include <boost/algorithm/string.hpp>

TpCategoryBrowser::TpCategoryBrowser(std::set<std::string> preloaded) : cn_(PGONTOLOGY) {
    setSelectionMode(Wt::ExtendedSelection);
    std::string rootname("root");
    Wt::WTreeNode * root = new Wt::WTreeNode(rootname);
    setTreeRoot(root);
    root->disable();
    root->label()->setTextFormat(Wt::PlainText);
    root->setLoadPolicy(Wt::WTreeNode::LazyLoading);
    // take care of of first generation children
    SetUpRootChildren(preloaded);
    root->expand();
}

void TpCategoryBrowser::SetUpRootChildren(std::set<std::string> preloaded) {
    LoadNextChildren(treeRoot(), preloaded);
}

void TpCategoryBrowser::DeleteRootChildren() {
    std::vector<Wt::WObject*> aux(treeRoot()->children());
    while (!aux.empty()) {
        delete aux.back();
        aux.pop_back();
    }
}

void TpCategoryBrowser::LoadNextChildren(Wt::WTreeNode * x,
        std::set<std::string> preloaded) {
    std::vector<Wt::WTreeNode*> children(x->childNodes());
    while (!children.empty()) {
        x->removeChildNode(children.back());
        delete children.back();
        children.pop_back();
    }
    std::string xname = x->label()->text().toUTF8();
    std::set<std::string> childrennames = GetAllDirectChildrensName(xname);
    if (!preloaded.empty()) {
        std::set<std::string> aux(childrennames);
        std::set<std::string>::iterator it;
        for (it = aux.begin(); it != aux.end(); it++) {
            std::set<std::string>::iterator itp = preloaded.find(*it);
            if (itp == preloaded.end())
                childrennames.erase(*it);
        }
    }
    for (std::set<std::string>::iterator it = childrennames.begin(); it != childrennames.end(); it++) {
        int noc(getNumberChildren(*it));
        Wt::WTreeNode * childnode = new Wt::WTreeNode(Wt::WString(*it));
        if (noc > 500) childnode->disable();
        childnode->expanded().connect(boost::bind(&TpCategoryBrowser::LoadNextChildren, this, childnode, std::set<std::string>()));
        childnode->label()->setWordWrap(false);
        x->addChildNode(childnode);
        if (noc > 0)
            if (noc < 500) {
                //create a dummy here to make it expandable, will be replaced later.
                Wt::WTreeNode * auxchildnode = new Wt::WTreeNode("");
                auxchildnode->expanded().connect(boost::bind(&TpCategoryBrowser::LoadNextChildren, this, auxchildnode, std::set<std::string>()));
                childnode->addChildNode(auxchildnode);
            }
    }
}

std::set<Wt::WString> TpCategoryBrowser::GetSelected() {
    std::set<Wt::WString> ret;
    std::vector<Wt::WTreeNode*> currentnodes(treeRoot()->childNodes());
    while (!currentnodes.empty()) {
        std::vector<Wt::WTreeNode*>::iterator it, it2;
        std::vector<Wt::WTreeNode*> nextnodes;
        nextnodes.clear();
        for (it = currentnodes.begin(); it != currentnodes.end(); it++) {
            std::vector<Wt::WTreeNode*> aux((*it)->childNodes());
            for (it2 = aux.begin(); it2 != aux.end(); it2++)
                nextnodes.push_back(*it2);
            if (isSelected(*it)) ret.insert((*it)->label()->text());
        }
        currentnodes.clear();
        while (!nextnodes.empty()) {
            currentnodes.push_back(nextnodes.back());
            nextnodes.pop_back();
        }
    }
    return ret;
}

int TpCategoryBrowser::getNumberChildren(std::string x) {
    int ret(0);
    try {
        pqxx::work w(cn_);
        PgList ontmembers(PGONTOLOGY, ONTOLOGYMEMBERSTABLENAME);
        for (auto tbn : ontmembers.GetList()) {
            pqxx::result r;
            std::stringstream pc;
            pc << "select count(child) from \"";
            pc << PCRELATIONSTABLENAME << "_" << tbn << "\" ";
            pc << "where parent='" << x << "'";
            r = w.exec(pc.str());
            ret += r[0]["count"].as<int>();
        }
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return ret;
}

std::set<std::string> TpCategoryBrowser::GetAllDirectChildrensName(std::string x) {
    std::set<std::string> ret;
    ret.clear();
    try {
        pqxx::work w(cn_);
        PgList ontmembers(PGONTOLOGY, ONTOLOGYMEMBERSTABLENAME);
        for (auto tbn : ontmembers.GetList()) {
            pqxx::result r;
            std::stringstream pc;
            pc << "select child from \"";
            pc << PCRELATIONSTABLENAME << "_" << tbn << "\" ";
            pc << "where parent='" << x << "'";
            r = w.exec(pc.str());
            for (pqxx::result::size_type i = 0; i != r.size(); i++) {
                std::string cname;
                if (r[i]["child"].to(cname)) {
                    ret.insert(cname);
                    cat2ont_.insert(std::make_pair(cname, tbn));
                }
            }
        }
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return ret;
}

std::set<std::string> TpCategoryBrowser::GetAllChildrensName(std::string x) {
    std::set<std::string> ret;
    ret.clear();
    for (auto cn : GetAllDirectChildrensName(x)) {
        ret.insert(cn);
        for (auto cn2 : GetAllChildrensName(cn)) ret.insert(cn2);
    }
    return ret;
}

int TpCategoryBrowser::getTotalNumSubCategories(std::string categoryName) {
    std::set<std::string> directChildrenNames = GetAllDirectChildrensName(categoryName);
    int totalNumChildren = 0;
    if (directChildrenNames.size() == 0) {
        return 0;
    } else {
        for (std::string childrenName : directChildrenNames) {
            // count children
            totalNumChildren += getTotalNumSubCategories(childrenName);
            // count actual node
            totalNumChildren += 1;
        }
    }
    return totalNumChildren;
}

std::set<std::string> TpCategoryBrowser::GetCategorySet() {
    std::set<std::string> ret;
    try {
        pqxx::work w(cn_);
        PgList ontmembers(PGONTOLOGY, ONTOLOGYMEMBERSTABLENAME);
        for (auto tbn : ontmembers.GetList()) {
            pqxx::result r;
            std::stringstream pc;
            pc << "select parent,child from \"";
            pc << PCRELATIONSTABLENAME << "_" << tbn << "\"";
            r = w.exec(pc.str());
            for (pqxx::result::size_type i = 0; i != r.size(); i++) {
                std::string cname;
                if (r[i]["child"].to(cname))
                    if (!cname.empty())
                        ret.insert(cname);
                if (r[i]["parent"].to(cname))
                    if (!cname.empty())
                        ret.insert(cname);
            }
        }
        w.commit();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    return ret;
}

TpCategoryBrowser::~TpCategoryBrowser() {
    cn_.disconnect();
}
