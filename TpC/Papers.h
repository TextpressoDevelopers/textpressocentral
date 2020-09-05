/* 
 * File:   Papers.h
 * Author: mueller
 *
 * Created on April 4, 2013, 10:22 AM
 */

#ifndef PAPERS_H
#define	PAPERS_H

#include "Session.h"
#include <Wt/WContainerWidget>
#include <Wt/WTabWidget>

class Papers : public Wt::WContainerWidget {
public:
    Papers(Wt::WContainerWidget * parent = NULL);
    void LoadContent(Session & session);
    virtual ~Papers();
private:
    bool alreadyloaded_;
    Session * session_;
    Wt::WText * statusline_;
    Wt::WTabWidget * tabwidget_;
    void FillContainerWithHelpText(Wt::WContainerWidget * p);
    void SessionLoginChanged();
};

#endif	/* PAPERS_H */

