/* 
 * File:   Home.h
 * Author: mueller
 *
 * Created on April 4, 2013, 11:46 AM
 */

#ifndef HOME_H
#define	HOME_H

#include <Wt/WContainerWidget>
#include "Search.h"
#include "Session.h"

class Home : public Wt::WContainerWidget {
public:
    Home(Session&, Wt::WContainerWidget * parent = NULL);
    void LoadContent(Search * search = NULL);
private:
    Wt::WContainerWidget * parent_;
    Wt::WContainerWidget * SearchBox(Search * search);
    void SetCursorHand(Wt::WWidget * w);
    Session* session_;
};

#endif	/* HOME_H */
