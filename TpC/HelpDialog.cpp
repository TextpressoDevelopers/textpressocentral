/* 
 * File:   HelpDialog.cpp
 * Author: mueller
 * 
 * Created on September 22, 2015, 10:21 AM
 */

#include "HelpDialog.h"
#include <Wt/WBreak>
#include <Wt/WPushButton>

HelpDialog::HelpDialog(Wt::WString title, bool modal, Wt::WContainerWidget * contents) {
    setWindowTitle(title);
    setModal(modal);
    setClosable(true);
    this->contents()->addWidget(contents);
    this->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", this->contents());
    ok->clicked().connect(this, &Wt::WDialog::accept);
    finished().connect(this, &HelpDialog::HelpDialogDone);
    show();
}

void HelpDialog::HelpDialogDone() {
    delete this;
}

HelpDialog::HelpDialog(const HelpDialog& orig) {
}

HelpDialog::~HelpDialog() {
}

