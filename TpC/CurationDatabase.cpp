/* 
 * File:   CurationDatabase.cpp
 * Author: mueller
 * 
 * Created on March 21, 2014, 4:38 PM
 */

#include "CurationDatabase.h"
#include "PgTableEditor.h"
#include "TextpressoCentralGlobalDefinitions.h"

CurationDatabase::CurationDatabase(Session & session, Wt::WContainerWidget * parent) {
    addWidget(new PgTableEditor(session, PGCURATION, PGCURATIONTABLENAME));
}

