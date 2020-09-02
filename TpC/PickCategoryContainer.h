/* 
 * File:   PickCategoryContainer.h
 * Author: mueller
 *
 * Created on July 16, 2017, 9:32 AM
 */

#ifndef PICKCATEGORYCONTAINER_H
#define	PICKCATEGORYCONTAINER_H

#include "TpOntologyBrowser.h"
#include "TpCategoryBrowser.h"
#include "CategoryPreferences.h"
#include "Session.h"

#include <Wt/WContainerWidget>
#include <Wt/WCheckBox>
#include <Wt/WLineEdit>
#include <Wt/WButtonGroup>


class PickCategoryContainer : public Wt::WContainerWidget {
public:
    PickCategoryContainer(Session * session, 
            std::set<std::string> & preloaded, bool showmatchcriteriachoice,
            bool expandtree);
    std::set<Wt::WString> GetSelected(bool explct = false);
    bool CatAnded() { return (groupAndorOr_->checkedId() > 0); }
    TpCategoryBrowser * GetTCB () { return tcb_; }
    Wt::Signal<void> & OkClicked() { return okclicked_; }
    virtual ~PickCategoryContainer();
private:
    Session * session_;
    bool showmatchcriteriachoice_;
    std::set<Wt::WString> catsfromlineedit_;
    std::set<std::string> preloaded_;
    std::set<std::string> catsetfrompg_;
    Wt::WContainerWidget * selectedcatdisplay_;
    TpCategoryBrowser * tcb_;
    CategoryPreferences * cp_;
    Wt::WButtonGroup * groupAndorOr_;
    Wt::WLineEdit * searchbox_;
    Wt::WCheckBox * includechildren_;
    Wt::Signal<void> okclicked_;
    void DisplaySelectedCategories();
    void PrepareInputDisplay(Session * session,Wt::WScrollArea * cs);
    void EnterPressed();
    void ItemSelectionChanged();
    void ResetSelectedCategories();
    void ChangePreferences(Session * session);
    void CategoryPreferencesDialogDone(Wt::WDialog::DialogCode code);
};

#endif	/* PICKCATEGORYCONTAINER_H */
