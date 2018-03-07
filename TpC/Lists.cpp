/* 
 * File:   Lists.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 10:23 AM
 */

#include "Lists.h"
#include <Wt/WBreak>
#include <Wt/WText>

Lists::Lists(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    this->addWidget(new Wt::WBreak);
    this->addWidget(new Wt::WText("User defined lists of words, sentences and documents "));
    this->addWidget(new Wt::WText("can be manipulated here. "));
    this->addWidget(new Wt::WText("Lists will be used for NLP and Ontologies. "));
}

Lists::~Lists() {
}

