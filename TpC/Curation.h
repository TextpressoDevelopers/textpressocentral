/* 
 * File:   Curation.h
 * Author: mueller
 *
 * Created on April 4, 2013, 10:21 AM
 */

#ifndef CURATION_H
#define	CURATION_H

#include <Wt/WContainerWidget>
#include <Wt/WComboBox>
#include "Session.h"
#include "UrlParameters.h"
#include "TCNavWeb.h"

struct PaperAddress;

class Curation : public Wt::WContainerWidget {
public:
    Curation(Session & session, UrlParameters * urlparams, Wt::WContainerWidget * parent = NULL);
    virtual ~Curation();
    void LoadContent();
private:
    Session * session_;
    Wt::WContainerWidget * parent_;
    Wt::WContainerWidget * combocontainer_;
    Wt::WContainerWidget * viewercontainer_;
    UrlParameters * urlparams_;
    void SetPaperInViewer();
    void SetCursorHand(Wt::WWidget * w);

    PaperAddress* newpaper;
};

#endif	/* CURATION_H */
