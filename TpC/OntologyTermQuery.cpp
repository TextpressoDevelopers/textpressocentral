/* 
 * File:   OntologyTermQuery.cpp
 * Author: mueller
 * 
 * Created on July 19, 2017, 10:09 PM
 */

#include "OntologyTermQuery.h"
#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>

OntologyTermQuery::OntologyTermQuery() {
    Wt::WText *caption = new Wt::WText("Search for Terms");
    caption->decorationStyle().font().setWeight(Wt::WFont::Bold);
    caption->decorationStyle().font().setSize(Wt::WFont::Large);
    Wt::WText *label = new Wt::WText("Enter: ");
    term_ = new Wt::WLineEdit();
    label->setInline(true);
    term_->setInline(true);
    addWidget(new Wt::WBreak());
    addWidget(caption);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(label);
    addWidget(term_);
    term_->enterPressed().connect(std::bind([ = ] (){
        termentered_.emit();
    }));
}

OntologyTermQuery::OntologyTermQuery(const OntologyTermQuery& orig) {
}

OntologyTermQuery::~OntologyTermQuery() {
}

