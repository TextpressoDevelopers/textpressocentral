/* 
 * File:   AnnotationGroupBox.cpp
 * Author: mueller
 * 
 * Created on March 7, 2014, 9:35 AM
 */

#include "AnnotationGroupBox.h"
#include <Wt/WCssDecorationStyle>

AnnotationGroupBox::AnnotationGroupBox(Wt::WString title,
        Wt::WContainerWidget * parent) : Wt::WGroupBox(parent) {
    setTitle(title);
    container_ = new Wt::WContainerWidget();
    Wt::WLength x(Wt::WLength(24, Wt::WLength::FontEx));
    Wt::WLength y(Wt::WLength(10, Wt::WLength::FontEx));
    container_->setWidth(x);
    //container_->setHeight(y);
    addWidget(container_);
    anntext_ = new Wt::WText();
    anntext_->decorationStyle().setBackgroundColor(NOTIFYCOLOR);
    container_->addWidget(anntext_);
}

AnnotationGroupBox::~AnnotationGroupBox() {
    delete anntext_;
    delete container_;
}

