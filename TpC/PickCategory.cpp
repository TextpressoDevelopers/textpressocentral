/* 
 * File:   PickCategory.cpp
 * Author: mueller
 * 
 * Created on October 29, 2014, 10:13 AM
 */

#include "PickCategory.h"
#include "PickCategoryContainer.h"

PickCategory::PickCategory(Session * session, std::set<std::string> & preloaded) {
    setWindowTitle("Pick Categories");
    setModal(false);
    setResizable(true);
    setClosable(true);
//    setMaximumSize(Wt::WLength::Auto, Wt::WLength::Auto);
    setMaximumSize(Wt::WLength::Auto, Wt::WLength(70, Wt::WLength::FontEx));
    //
    pcc_ = new PickCategoryContainer(session, preloaded, true, true);
    pcc_->setMaximumSize(Wt::WLength::Auto, Wt::WLength(45, Wt::WLength::FontEx));
    contents()->addWidget(pcc_);
    pcc_->OkClicked().connect(this, &Wt::WDialog::accept);
    //
}

PickCategory::~PickCategory() {
}
