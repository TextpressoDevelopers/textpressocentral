/* 
 * File:   Workflow.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 10:22 AM
 */

#include "Workflow.h"
#include <Wt/WBreak>
#include <Wt/WText>

Workflow::Workflow(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    this->addWidget(new Wt::WBreak);
    this->addWidget(new Wt::WText("When automated pipelines are enabled, "));
    this->addWidget(new Wt::WText("create and manage them here."));
}

Workflow::~Workflow() {
}

