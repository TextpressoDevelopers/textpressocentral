/* 
 * File:   Login.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 11:46 AM
 */

#include "Login.h"
#include "AuthWidget.h"

Login::Login(AuthWidget * aw, Wt::WContainerWidget * parent) {
    this->addWidget(aw);
}

Login::~Login() {
}

