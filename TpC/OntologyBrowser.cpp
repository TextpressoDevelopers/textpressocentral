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
#include <Wt/WCssDecorationStyle>

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
            preloaded, false, false);
    pcc->setMaximumSize(Wt::WLength(45,Wt::WLength::Percentage), h1);
    //
    Wt::WText * ortext = new Wt::WText("-- or --");
    ortext->decorationStyle().font().setWeight(Wt::WFont::Bold);
    ortext->decorationStyle().font().setSize(Wt::WFont::Large);
    ortext->setVerticalAlignment(Wt::AlignMiddle);
    //
    OntologyTermQuery * otq = new OntologyTermQuery();
    otq->setMaximumSize(Wt::WLength(45, Wt::WLength::Percentage), h1);
    //
    Wt::WHBoxLayout * qlayout = new Wt::WHBoxLayout();
    qlayout->addWidget(pcc);
    qlayout->addWidget(ortext);
    qlayout->addWidget(otq);
    Wt::WContainerWidget * querycont = new Wt::WContainerWidget();
    querycont->setLayout(qlayout);
    //
    OntologyDisplay * od = new OntologyDisplay(pcc, otq, h2);
    od->setMaximumSize(Wt::WLength(100, Wt::WLength::Percentage), h2);
    layout->addWidget(querycont);
    layout->addWidget(od);    
}

OntologyBrowser::~OntologyBrowser() {
}
