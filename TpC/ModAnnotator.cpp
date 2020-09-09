/* 
 * File:   ModAnnotator.cpp
 * Author: mueller
 * 
 * Created on September 18, 2013, 2:44 PM
 */

#include "ModAnnotator.h"
#include <Wt/WBreak>
#include <Wt/WText>

ModAnnotator::ModAnnotator(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    this->addWidget(new Wt::WBreak);
    this->addWidget(new Wt::WText("GO annotation can be performed here. Data transfer from Textpresso is done via a webservice."));
}

ModAnnotator::~ModAnnotator() {
}

