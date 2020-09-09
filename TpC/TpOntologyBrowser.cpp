/* 
 * File:   TpOntologyBrowser.cpp
 * Author: mueller
 * 
 * Created on September 8, 2014, 2:08 PM
 */


#include "TpOntologyBrowser.h"
#include "PgList.h"

#include <Wt/WTreeNode>
#include <Wt/WText>
#include <iostream>
#include <vector>

TpOntologyBrowser::TpOntologyBrowser() : cn_(PGONTOLOGY) {
    includechildren_ = false;
    std::cerr << "TpOntologyBrowser: Connected to " << cn_.dbname() << std::endl;
    setSelectionMode(Wt::ExtendedSelection);
    // find root
    Read2Cols(PCRELATIONSTABLENAME, "parent", "child");
    std::map<std::string, std::string>::iterator itroot = pcrelations_.begin();
    bool changed = true;
    while (changed) {
        changed = false;
        std::map<std::string, std::string>::iterator it;
        for (it = pcrelations_.begin(); it != pcrelations_.end(); it++) {
            if ((*it).second == (*itroot).first) {// current root is a child;
                itroot = it;
                changed = true;
            }
        }
    }
    std::string rootname = (*itroot).first;
    Wt::WTreeNode * root = new Wt::WTreeNode(rootname);
    setTreeRoot(root);
    root->disable();
    root->label()->setTextFormat(Wt::PlainText);
    root->setLoadPolicy(Wt::WTreeNode::LazyLoading);
    // take care of of first generation children
    LoadNextChildren(root);
    root->expand();
}

void TpOntologyBrowser::NodeSelected(Wt::WTreeNode * x) {
    if (includechildren_) {
        std::vector<Wt::WTreeNode*> current_children(x->childNodes());
        std::vector<Wt::WTreeNode*>::iterator it;
        for (it = current_children.begin(); it != current_children.end(); it++)
            select(*it, isSelected(x));
    }
}

void TpOntologyBrowser::LoadNextChildrenAdvanced(Wt::WTreeNode * x, bool grandchildren) {
    std::vector<Wt::WTreeNode*> children(x->childNodes());
    while (!children.empty()) {
        x->removeChildNode(children.back());
        delete children.back();
        children.pop_back();
    }
    std::string xname = x->label()->text().toUTF8();
    mmssrange range = pcrelations_.equal_range(xname);
    for (mmsstype::iterator it = range.first; it != range.second; it++) {
        std::string childname = (*it).second;
        Wt::WTreeNode * childnode = new Wt::WTreeNode(Wt::WString(childname));
        childnode->selected().connect(boost::bind(&TpOntologyBrowser::NodeSelected, this, childnode));
        childnode->expanded().connect(boost::bind(&TpOntologyBrowser::LoadNextChildrenAdvanced, this, childnode, true));
        x->addChildNode(childnode);
        if (grandchildren) {
            std::vector<Wt::WTreeNode*> children(x->childNodes());
            while (!children.empty()) {
                LoadNextChildrenAdvanced(children.back(), false);
                children.pop_back();
            }
        }
    }
}

void TpOntologyBrowser::LoadNextChildren(Wt::WTreeNode * x) {
    std::vector<Wt::WTreeNode*> children(x->childNodes());
    while (!children.empty()) {
        x->removeChildNode(children.back());
        delete children.back();
        children.pop_back();
    }
    std::string xname = x->label()->text().toUTF8();
    mmssrange range = pcrelations_.equal_range(xname);
    std::set<std::string> childnames;
    for (mmsstype::iterator it = range.first; it != range.second; it++)
        childnames.insert((*it).second);
    for (std::set<std::string>::iterator it = childnames.begin(); it != childnames.end(); it++) {
        Wt::WTreeNode * childnode = new Wt::WTreeNode(Wt::WString(*it));
        childnode->selected().connect(boost::bind(&TpOntologyBrowser::NodeSelected, this, childnode));
        childnode->expanded().connect(boost::bind(&TpOntologyBrowser::LoadNextChildren, this, childnode));
        childnode->label()->setWordWrap(false);
        x->addChildNode(childnode);
        mmssrange childrange = pcrelations_.equal_range(*it);
        if (childrange.first != childrange.second) {
            //create a dummy here to make it expandable, will be replaced later.
            Wt::WTreeNode * auxchildnode = new Wt::WTreeNode("");
            auxchildnode->expanded().connect(boost::bind(&TpOntologyBrowser::LoadNextChildren, this, auxchildnode));
            childnode->addChildNode(auxchildnode);
        }
    }
}

std::set<Wt::WString> TpOntologyBrowser::GetSelected() {
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

TpOntologyBrowser::~TpOntologyBrowser() {
    cn_.disconnect();
}

void TpOntologyBrowser::Read2Cols(std::string tablename,
        std::string col1name, std::string col2name) {
    try {
        pqxx::work w(cn_);
        PgList ontmembers(PGONTOLOGY, ONTOLOGYMEMBERSTABLENAME);
        for (auto tbn : ontmembers.GetList()) {
            pqxx::result r;
            std::stringstream pc;
            pc << "select " << col1name << "," << col2name << " from \"";
            pc << tablename << "_" << tbn << "\"";
            r = w.exec(pc.str());
            std::set<std::string> seen;
            for (pqxx::result::size_type i = 0; i != r.size(); i++) {
                std::string col1;
                std::string col2;
                if (r[i][col1name].to(col1) && r[i][col2name].to(col2)) {
                    std::string auxstring = col1 + "+*_" + col2;
                    if (seen.find(auxstring) == seen.end()) {
                        seen.insert(auxstring);
                        pcrelations_.insert(std::make_pair(col1, col2));
                        categorynames_.insert(col1);
                        categorynames_.insert(col2);
                    }
                }
            }
        }
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.
        std::cerr << e.what() << std::endl;
    }
}
