/* 
 * File:   Permissions.h
 * Author: mueller
 *
 * Created on April 5, 2016, 7:54 PM
 */

#ifndef PERMISSIONS_H
#define	PERMISSIONS_H

#include <Wt/WContainerWidget>
#include "UrlParameters.h"

class Permissions : public Wt::WContainerWidget {
public:
    Permissions(UrlParameters * urlparams, Wt::WContainerWidget * parent = NULL);
    virtual ~Permissions();
private:
    Wt::WContainerWidget * parent_;
    UrlParameters * urlparams_;
    std::map<std::string, std::set<std::string>> userpermissions_;
    std::map<std::string, std::set<std::string>> useravailablelits_;
    void UpdateTableContainers(Wt::WContainerWidget * available, Wt::WContainerWidget * permitted, std::string user);
    void FromA2P(std::string user, std::string lit, Wt:: WContainerWidget* a, Wt::WContainerWidget * p);
    void FromP2A(std::string user, std::string lit, Wt:: WContainerWidget* a, Wt::WContainerWidget * p);
    void Save();
    void SetCursorHand(Wt::WWidget * w);
};

#endif	/* PERMISSIONS_H */

