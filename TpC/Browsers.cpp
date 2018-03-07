/* 
 * File:   Browsers.cpp
 * Author: mueller
 * 
 * Created on July 17, 2017, 11:32 AM
 */

#include "Browsers.h"
//#include "TextpressoCentralGlobalDefinitions.h"
#include <Wt/WTabWidget>
//#include <Wt/WText>
//#include <Wt/WCssDecorationStyle>
//#include <Wt/WTimer>
//#include <boost/regex.hpp>
//#include <boost/filesystem.hpp>
//#include <Wt/WImage>
//#include <Wt/WTable>
//#include <Wt/WBreak>

Browsers::Browsers(Session & session, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    clear();
    //
    ob_= new OntologyBrowser(session, parent);
    //pb_ = new PaperBrowser();
    //
    tabwidget_ = new Wt::WTabWidget(this);
    tabwidget_->addTab(ob_, "Ontology");
    //tabwidget_->addTab(pb_, "Papers");
    tabwidget_->setCurrentIndex(0);
    tabwidget_->show();
}

Browsers::~Browsers() {
}

