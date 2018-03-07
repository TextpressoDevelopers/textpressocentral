/**
    Project: textpressocentral
    File name: MyLostPasswordWidget.cpp

    @author valerio
    @version 1.0 6/30/17.
*/

#include "MyLostPasswordWidget.h"
#include "Wt/Auth/AuthService"

#include "Wt/WLineEdit"
#include "Wt/WMessageBox"
#include "Wt/WPushButton"
#include "MyAuthService.h"
#include "AuthSingleton.h"

using namespace Wt;
using namespace Wt::Auth;

MyLostPasswordWidget::MyLostPasswordWidget(AbstractUserDatabase& users,
                                           const AuthService& auth,
                                           WContainerWidget *parent)
        : WTemplate(tr("Wt.Auth.template.lost-password"), parent),
          users_(users),
          baseAuth_(auth)
{
    addFunction("id", &Functions::id);
    addFunction("tr", &Functions::tr);
    addFunction("block", &Functions::block);

    WLineEdit *email = new WLineEdit();
    email->setFocus(true);

    WPushButton *okButton = new WPushButton(tr("Wt.Auth.send"));
    WPushButton *cancelButton = new WPushButton(tr("Wt.WMessageBox.Cancel"));

    okButton->clicked().connect(this, &MyLostPasswordWidget::send);
    cancelButton->clicked().connect(this, &MyLostPasswordWidget::cancel);

    bindWidget("email", email);
    bindWidget("send-button", okButton);
    bindWidget("cancel-button", cancelButton);
}

void MyLostPasswordWidget::send()
{
    WFormWidget *email = resolve<WFormWidget *>("email");
    baseAuth_.lostPassword(email->valueText().toUTF8(), users_);
    cancel();
    WMessageBox* box;

    if (AuthSingleton::getInstance().isUserValid()) {
         box = new WMessageBox(tr("Wt.Auth.lost-password"),
                                                 tr("Wt.Auth.mail-sent"),
                                                 NoIcon, Ok);
    } else {
        box = new WMessageBox("Error", "We can't find the provided email address. Make sure you verified the address "
                                      "by following the link we sent you during registration. If you can't find the "
                                      "activation email, please contact textpresso team",
                              NoIcon, Ok);
    }
#ifndef WT_TARGET_JAVA
    box->buttonClicked().connect
            (boost::bind(&MyLostPasswordWidget::deleteBox, box));
#else
    box->buttonClicked().connect
    (boost::bind(&MyLostPasswordWidget::deleteBox, this, box));
#endif
    box->show();
}

void MyLostPasswordWidget::deleteBox(WMessageBox *box)
{
    delete box;
}

void MyLostPasswordWidget::cancel()
{
    delete this;
}
