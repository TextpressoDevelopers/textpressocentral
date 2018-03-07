/* 
 * File:   OntologyBrowser.h
 * Author: mueller
 *
 * Created on July 17, 2017, 11:48 AM
 */

#ifndef ONTOLOGYBROWSER_H
#define	ONTOLOGYBROWSER_H

#include "Session.h"
#include <Wt/WContainerWidget>

class OntologyBrowser : public Wt::WContainerWidget {
public:
    OntologyBrowser(Session & session, Wt::WContainerWidget * parent = NULL);
    virtual ~OntologyBrowser();
private:
 Session * session_;
};

#endif	/* ONTOLOGYBROWSER_H */

