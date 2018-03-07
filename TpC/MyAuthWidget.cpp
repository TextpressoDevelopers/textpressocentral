/*
 * File:   MyAuthWidget.cpp
 * Author: mueller
 * 
 * Created on April 12, 2016, 7:17 PM
 */

#include "MyAuthWidget.h"
#include "Session.h"
#include "MyLostPasswordWidget.h"

MyAuthWidget::MyAuthWidget(Session & session) : Wt::Auth::AuthWidget(Session::auth(), session.users(), session.login()),
session_(session) {
}

void MyAuthWidget::createLoggedInView() {
    loggedinview_.emit();
}

Wt::WWidget* MyAuthWidget::createLostPasswordView() {
    return new MyLostPasswordWidget(session_.users(), session_.auth());
}