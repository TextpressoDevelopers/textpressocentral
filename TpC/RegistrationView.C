/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#include "RegistrationView.h"
#include "UserDetailsModel.h"

#include <Wt/WLineEdit>

RegistrationView::RegistrationView(Session & session, Wt::Auth::AuthWidget * authWidget)
: Wt::Auth::RegistrationWidget(authWidget), session_(session) {
    setTemplateText(tr("template.registration"));
    detailsModel_ = new UserDetailsModel(session_, this);
    updateView(detailsModel_);
}

Wt::WFormWidget * RegistrationView::createFormWidget(Wt::WFormModel::Field field) {
    if (field == UserDetailsModel::affiliation)
        return new Wt::WLineEdit();
    else if (field == UserDetailsModel::permissions)
        return new Wt::WLineEdit();
    else
        return (Wt::WFormWidget*) Wt::Auth::RegistrationWidget::createFormWidget(field);
}

bool RegistrationView::validate() {
    bool result = Wt::Auth::RegistrationWidget::validate();
    updateModel(detailsModel_);
    if (!detailsModel_->validate())
        result = false;
    updateView(detailsModel_);

    return result;
}

void RegistrationView::registerUserDetails(Wt::Auth::User & user) {
    detailsModel_->save(user);
}
