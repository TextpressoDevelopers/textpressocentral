/* 
 * File:   Browsers.h
 * Author: mueller
 *
 * Created on July 17, 2017, 11:32 AM
 */

#ifndef BROWSERS_H
#define	BROWSERS_H

#include "OntologyBrowser.h"
#include "PaperBrowser.h"
#include <Wt/WContainerWidget>

class Browsers : public Wt::WContainerWidget {
public:
    Browsers(Session & session, Wt::WContainerWidget * parent = NULL);
    virtual ~Browsers();
private:
    OntologyBrowser * ob_;
    PaperBrowser * pb_;
    Wt::WTabWidget * tabwidget_;
};

#endif	/* BROWSERS_H */
