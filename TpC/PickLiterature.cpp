/* 
 * File:   PickLiterature.cpp
 * Author: mueller
 * 
 * Created on March 9, 2015, 2:03 PM
 */

#include "PickLiterature.h"
#include <Wt/WVBoxLayout>
#include <Wt/WBreak>

PickLiterature::PickLiterature(Session * session, std::map<std::string, bool> & pickedlit) {
    setWindowTitle("Pick Literature");
    setModal(false);
    setResizable(true);
    setWidth(Wt::WLength("60%"));
    setClosable(true);
    Wt::WVBoxLayout * vertbox = new Wt::WVBoxLayout();
    contents()->setLayout(vertbox);
    plc_ = new PickedLiteratureContents(session, pickedlit);
    plc_->SignalOkClicked().connect(this, &Wt::WDialog::accept);
    vertbox->addWidget(plc_);
    vertbox->addWidget(new Wt::WBreak());
    contents()->setMinimumSize(Wt::WLength::Auto, Wt::WLength(40, Wt::WLength::FontEx));
}

PickLiterature::~PickLiterature() {
}
