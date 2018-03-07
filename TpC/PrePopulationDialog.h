/* 
 * File:   PrePopulationDialog.h
 * Author: mueller
 *
 * Created on July 16, 2015, 10:11 AM
 */

#ifndef PREPOPULATIONDIALOG_H
#define	PREPOPULATIONDIALOG_H

#include "Session.h"
#include <Wt/WDialog>
#include "PickCategory.h"
#include "AutocompleteAndValidationDialog.h"

class PrePopulationDialog : public Wt::WDialog {
public:

    enum mode {
        statictext, terms, termsandsynonyms, categories,
        termsandcategories, termssynonymsandcategories, viewerinfodata
    };

    struct syndatabase {
        std::string col;
        std::string whereclause;
        std::string database;
        std::string tablename;
    };

    syndatabase GetSynDataBaseInfo() {
        return syndatabase_;
    }
    PrePopulationDialog(Session * session);
    virtual ~PrePopulationDialog();
    void SetTitle(Wt::WString name);

    bool HasPrePopInfo() {
        return (!prepopstring_.empty());
    }

    Wt::WString GetPrePopString() {
        return prepopstring_;
    }

    int GetPrePopMode() {
        return prepopmode_;
    }
    void SetPrePopString(Wt::WString s);
    void SetPrePopMode(int i);
    AutocompleteAndValidationDialog::forminfo GetSdiDialog() { return sdiinfo_; }
private:
    Session * session_;
    int prepopmode_;
    Wt::WString prepopstring_;
    Wt::WButtonGroup * modegroup_;
    Wt::WContainerWidget * prepopdetailscont_;
    Wt::WText * prepoptextlabel_;
    Wt::WText * prepoptext_;
    PickCategory * pickcat_;
    std::set<std::string> preloadedcategories_;
    Wt::WDialog * sdidialog_;
    AutocompleteAndValidationDialog::forminfo sdiinfo_;
    syndatabase syndatabase_;
    void RadioButtonChecked();
    void SynDataInfoButtonClicked();
    void SdiDialogDone(AutocompleteAndValidationDialog::forminfo fi);
    void PickCategoryDialogDone(Wt::WDialog::DialogCode code);
    void ReadPreloadedCategories();
};

#endif	/* PREPOPULATIONDIALOG_H */

