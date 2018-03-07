/* 
 * File:   AutocompleteAndValidationDialog.h
 * Author: mueller
 *
 * Created on June 8, 2015, 11:46 AM
 */

#ifndef AUTOCOMPLETEANDVALIDATIONDIALOG_H
#define	AUTOCOMPLETEANDVALIDATIONDIALOG_H

#include <Wt/WDialog>
#include <Wt/WLineEdit>
#include <Wt/WString>

class AutocompleteAndValidationDialog : public Wt::WDialog {
public:

    struct forminfo {
        Wt::WLineEdit * dbname;
        Wt::WLineEdit * tablename;
        Wt::WLineEdit * columnname;
        Wt::WLineEdit * whereclause;
        Wt::WLineEdit * hostname;
        Wt::WLineEdit * username;
        Wt::WLineEdit * password;
        Wt::WLineEdit * port;
    };

    AutocompleteAndValidationDialog(bool validationdialog);

    forminfo GetAutocompleteInfo() {
        return autocomplete_;
    }

    forminfo GetValidationInfo() {
        return validation_;
    }
    
    Wt::WLineEdit * GetAutoCompleteHostName() {
        return ahostname_;
    }

    Wt::WLineEdit * GetValidationHostName() {
        return vhostname_;
    }

    bool HasValidationInfo() {
        return validationdialog_;
    }
    void SetTitle(Wt::WString s);
    Wt::WContainerWidget * InfoBox(forminfo & fi, Wt::WString s);
    virtual ~AutocompleteAndValidationDialog();
private:
    bool validationdialog_;
    Wt::WCheckBox * v_is_same1_;
    Wt::WCheckBox * v_is_same2_;
    forminfo autocomplete_;
    forminfo validation_;
    Wt::WLineEdit * ahostname_;
    Wt::WLineEdit * vhostname_;
    std::map<std::string, std::string> idmap_;
    std::vector<Wt::WCheckBox*> checkboxlist_;
    void PpacDialogDone(Wt::WDialog::DialogCode code);
    void PopulateInfoBox();
    void DeepCopyInfo(forminfo source, forminfo & dest);
    void CheckToCopy();
    void AutocompleteURLChanged();
    void ValidationURLChanged();
};

#endif	/* AUTOCOMPLETEANDVALIDATIONDIALOG_H */

