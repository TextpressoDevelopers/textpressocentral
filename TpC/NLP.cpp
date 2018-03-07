/* 
 * File:   NLP.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 10:21 AM
 */

#include "NLP.h"
#include <Wt/WBreak>
#include <Wt/WText>

NLP::NLP(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    this->addWidget(new Wt::WBreak);
    this->addWidget(new Wt::WText("Train and apply machine-learning algorithms for text mining purposes. In development."));
}

NLP::~NLP() {
}

