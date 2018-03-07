/* 
 * File:   Permissions.cpp
 * Author: mueller
 * 
 * Created on April 5, 2016, 7:54 PM
 */

#include "TextpressoCentralGlobalDefinitions.h"
#include "Permissions.h"
#include "Preference.h"
#include <pqxx/pqxx>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WTable>
#include <Wt/WPushButton>
#include <Wt/WCssDecorationStyle>
#include <boost/regex.hpp>

Permissions::Permissions(UrlParameters * urlparams, Wt::WContainerWidget * parent)
: parent_(parent), urlparams_(urlparams), Wt::WContainerWidget(parent) {
    // load all users and their permissions
    std::set<std::string> users;
    pqxx::connection cn(AUTHIDENTITIES);
    pqxx::work w(cn);
    pqxx::result r;
    std::stringstream pc;
    pc << "select identity from ";
    pc << AUTHIDENTITIESTABLENAME;
    r = w.exec(pc.str());
    for (pqxx::result::size_type i = 0; i != r.size(); i++) {
        std::string cname;
        if (r[i]["identity"].to(cname)) {
            users.insert(cname);
            Preference * litperms = new Preference(PGLITERATUREPERMISSION,
                    PGLITERATUREPERMISSIONTABLENAME, cname);
            std::set<std::string> auxset(litperms->GetPreferences());
            userpermissions_.insert(std::make_pair(cname, auxset));
            delete litperms;
        }
    }
    w.commit();
    cn.disconnect();
    // add default
    std::string cname("default");
    users.insert(cname);
    Preference * litperms = new Preference(PGLITERATUREPERMISSION,
            PGLITERATUREPERMISSIONTABLENAME, cname);
    std::set<std::string> auxset(litperms->GetPreferences());
    userpermissions_.insert(std::make_pair(cname, auxset));
    delete litperms;
    // load all available literatures
    std::set<std::string> alllits;
    std::string fname(AVAILABLELITERATUREFILE);
    std::ifstream f(fname.c_str());
    std::string in;
    while (getline(f, in)) {
        std::string newtext = "";
        boost::regex re("\\_\\d+$");
        std::string base = boost::regex_replace(in, re, newtext);
        alllits.insert(base);
    }
    f.close();
    std::set<std::string>::iterator itu, itl;
    for (itu = users.begin(); itu != users.end(); itu++)
        for (itl = alllits.begin(); itl != alllits.end(); itl++)
            if (userpermissions_[*itu].find(*itl) == userpermissions_[*itu].end())
                useravailablelits_[*itu].insert(*itl);
    // prepare table content
    Wt::WContainerWidget * tablecont = new Wt::WContainerWidget();
    Wt::WTable * table = new Wt::WTable();
    tablecont->addWidget(table);
    table->setHeaderCount(1);
    table->elementAt(0, 0)->addWidget(new Wt::WText("User"));
    table->elementAt(0, 1)->addWidget(new Wt::WText("Available literature"));
    table->elementAt(0, 2)->addWidget(new Wt::WText("Access granted to literature"));
    table->addStyleClass("table form-inline");
    table->addStyleClass("table-bordered");
    int rowcount(1);
    for (itu = users.begin(); itu != users.end(); itu++) {
        Wt::WText * ut = new Wt::WText(*itu);
        Wt::WContainerWidget * litcont = new Wt::WContainerWidget();
        Wt::WContainerWidget * permcont = new Wt::WContainerWidget();
        UpdateTableContainers(litcont, permcont, *itu);
        table->elementAt(rowcount, 0)->addWidget(ut);
        table->elementAt(rowcount, 1)->addWidget(litcont);
        table->elementAt(rowcount, 2)->addWidget(permcont);
        rowcount++;
    }
    //
    Wt::WPushButton * save = new Wt::WPushButton("Save");
    save->clicked().connect(boost::bind(&Permissions::Save, this));
    //
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WText("Click on literatures to move them between columns:"));
    addWidget(new Wt::WBreak());
    addWidget(tablecont);
    addWidget(save);
}

void Permissions::Save() {
    std::map<std::string, std::set < std::string>>::iterator it;
    for (it = userpermissions_.begin(); it != userpermissions_.end(); it++) {
        Preference * litperms = new Preference(PGLITERATUREPERMISSION,
                PGLITERATUREPERMISSIONTABLENAME, (*it).first);
        litperms->SavePreferences((*it).first, (*it).second);
        delete litperms;
    }
}

void Permissions::UpdateTableContainers(Wt::WContainerWidget * available, Wt::WContainerWidget * permitted, std::string user) {
    //
    available->clear();
    permitted->clear();
    //
    std::set<std::string>::iterator it;
    for (it = useravailablelits_[user].begin(); it != useravailablelits_[user].end(); it++) {
        Wt::WText * textaux = new Wt::WText(*it);
        textaux->clicked().connect(boost::bind(&Permissions::FromA2P, this, user, *it, available, permitted));
        textaux->mouseWentOver().connect(boost::bind(&Permissions::SetCursorHand, this, textaux));
        available->addWidget(textaux);
        available->addWidget(new Wt::WBreak());
    }
    for (it = userpermissions_[user].begin(); it != userpermissions_[user].end(); it++) {
        Wt::WText * textaux = new Wt::WText(*it);
        textaux->clicked().connect(boost::bind(&Permissions::FromP2A, this, user, *it, available, permitted));
        textaux->mouseWentOver().connect(boost::bind(&Permissions::SetCursorHand, this, textaux));
        permitted->addWidget(textaux);
        permitted->addWidget(new Wt::WBreak());
    }
}

void Permissions::FromA2P(std::string user, std::string lit, Wt::WContainerWidget * a, Wt::WContainerWidget* p) {
    useravailablelits_[user].erase(lit);
    userpermissions_[user].insert(lit);
    UpdateTableContainers(a, p, user);
}

void Permissions::FromP2A(std::string user, std::string lit, Wt::WContainerWidget * a, Wt::WContainerWidget* p) {
    useravailablelits_[user].insert(lit);
    userpermissions_[user].erase(lit);
    UpdateTableContainers(a, p, user);
}

Permissions::~Permissions() {
}

void Permissions::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}
