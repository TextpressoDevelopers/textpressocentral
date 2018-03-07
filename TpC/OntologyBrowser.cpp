/* 
 * File:   OntologyBrowser.cpp
 * Author: mueller
 * 
 * Created on July 17, 2017, 11:48 AM
 */

#include "OntologyBrowser.h"
#include "OntologyDisplay.h"
#include "OntologyTermQuery.h"
#include "PickCategoryContainer.h"

#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>

OntologyBrowser::OntologyBrowser(Session & session, Wt::WContainerWidget * parent) {
    session_ = &session;
    //
    Wt::WLength h1(Wt::WLength(20,Wt::WLength::FontEm));
    Wt::WLength h2(Wt::WLength(40,Wt::WLength::FontEm));
    //
    Wt::WVBoxLayout * layout = new Wt::WVBoxLayout();
    setLayout(layout);
    //
    std::set<std::string> preloaded;
    PickCategoryContainer * pcc = new PickCategoryContainer(session_,
            preloaded, false);
    pcc->setMaximumSize(Wt::WLength::Auto, h1);
    OntologyTermQuery * otq = new OntologyTermQuery();
    otq->setMaximumSize(Wt::WLength::Auto, h1);
    Wt::WHBoxLayout * qlayout = new Wt::WHBoxLayout();
    qlayout->addWidget(pcc);
    qlayout->addWidget(otq);
    Wt::WContainerWidget * querycont = new Wt::WContainerWidget();
    querycont->setLayout(qlayout);
    //
    OntologyDisplay * od = new OntologyDisplay(pcc, otq, h2);
    od->setMaximumSize(Wt::WLength::Auto, h2);
    layout->addWidget(querycont);
    layout->addWidget(od);
    
}

OntologyBrowser::~OntologyBrowser() {
}

