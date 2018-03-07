/* 
 * File:   PickCategory.h
 * Author: mueller
 *
 * Created on October 29, 2014, 10:13 AM
 */

#ifndef PICKCATEGORY_H
#define	PICKCATEGORY_H

#include "Session.h"
#include "PickCategoryContainer.h"
#include <Wt/WDialog>

class PickCategory : public Wt::WDialog {
public:
    PickCategory(Session * session, std::set<std::string> & preloaded);

    std::set<Wt::WString> GetSelected() {
        return pcc_->GetSelected();
    }

    bool CatAnded() {
        return pcc_->CatAnded();
    }
    virtual ~PickCategory();
private:
    PickCategoryContainer * pcc_;
};

#endif	/* PICKCATEGORY_H */
