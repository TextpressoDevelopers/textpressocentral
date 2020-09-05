/* 
 * File:   Curation.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 10:21 AM
 */

#include "Curation.h"
#include "Viewer.h"
#include "TCNavWeb.h"

Curation::Curation(Session & session, UrlParameters * urlparams, Wt::WContainerWidget * parent) : parent_(parent),
urlparams_(urlparams), session_(&session), Wt::WContainerWidget(parent) {
    combocontainer_ = new Wt::WContainerWidget();
    combocontainer_->setContentAlignment(Wt::AlignRight);
    viewercontainer_ = new Wt::WContainerWidget();
    newpaper = NULL;
    addWidget(combocontainer_);
    addWidget(viewercontainer_);
}

Curation::~Curation() {
}

void Curation::LoadContent() {
    TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent_);
    if (!tcnw->SinglePaperItemIsEmpty()) {
        TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent_);
        PaperAddress * newpaper = tcnw->GetSinglePaperItem();
        combocontainer_->clear();
        combocontainer_->addWidget(new Wt::WBreak());
        combocontainer_->addWidget(new Wt::WText("Title: " + (*newpaper).title));
        combocontainer_->addWidget(new Wt::WBreak());
        combocontainer_->addWidget(new Wt::WText("Author: " + (*newpaper).author));
        combocontainer_->addWidget(new Wt::WBreak());
        combocontainer_->addWidget(new Wt::WText("Journal: " + (*newpaper).journal));
        combocontainer_->addWidget(new Wt::WBreak());
        combocontainer_->addWidget(new Wt::WText("Year: " + (*newpaper).year));
        combocontainer_->addWidget(new Wt::WBreak());
        SetPaperInViewer();
    } else {
        viewercontainer_->clear();
        Wt::WText * emptyscreen1 = new Wt::WText();
        emptyscreen1->setText("If you see this text, then you haven't successfully loaded a paper. ");
        Wt::WAnchor * search = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/search"), "Search");
        Wt::WText * emptyscreen2 = new Wt::WText();
        emptyscreen2->setText(" papers first and pick those of interest "
                "in the 'Select' column of the result table. ");
        emptyscreen1->decorationStyle().setForegroundColor(Wt::red);
        search->decorationStyle().setForegroundColor(Wt::magenta);
        search->decorationStyle().setTextDecoration(Wt::WCssDecorationStyle::Underline);
        emptyscreen2->decorationStyle().setForegroundColor(Wt::red);
        viewercontainer_->addWidget(emptyscreen1);
        viewercontainer_->addWidget(search);
        viewercontainer_->addWidget(emptyscreen2);
    }
}

void Curation::SetPaperInViewer() {
    TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent_);
    if (newpaper == NULL || tcnw->paperHasChanged()) {
        tcnw->setNoChange();
        newpaper = tcnw->GetSinglePaperItem();
        viewercontainer_->clear();
        Viewer *viewer = new Viewer(urlparams_, session_, newpaper, parent_);
        viewercontainer_->addWidget(viewer);
    }
}

void Curation::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}
