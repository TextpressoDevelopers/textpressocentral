/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "AuthWidget.h"
#include "RegistrationView.h"
#include "Session.h"
#include "MyLostPasswordWidget.h"

AuthWidget::AuthWidget(Session & session)
: Wt::Auth::AuthWidget(Session::auth(), session.users(), session.login()),
session_(session) {
}

Wt::WWidget *AuthWidget::createRegistrationView(const Wt::Auth::Identity & id) {
    RegistrationView * w = new RegistrationView(session_, this);
    w->setMinimumSize(Wt::WLength(60,Wt::WLength::FontEx), Wt::WLength::Auto);
    Wt::Auth::RegistrationModel * model = createRegistrationModel();
    if (id.isValid())
        model->registerIdentified(id);
    w->setModel(model);
    return w;
}

Wt::WWidget* AuthWidget::createLostPasswordView() {
    return new MyLostPasswordWidget(session_.users(), session_.auth());
}
