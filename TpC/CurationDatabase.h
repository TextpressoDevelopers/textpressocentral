/* 
 * File:   CurationDatabase.h
 * Author: mueller
 *
 * Created on March 21, 2014, 4:38 PM
 */

#ifndef CURATIONDATABASE_H
#define	CURATIONDATABASE_H

#include <Wt/WContainerWidget>
#include "Session.h"

class CurationDatabase : public Wt::WContainerWidget {
public:
    CurationDatabase(Session & session, Wt::WContainerWidget * parent = NULL);
private:

};

#endif	/* CURATIONDATABASE_H */

