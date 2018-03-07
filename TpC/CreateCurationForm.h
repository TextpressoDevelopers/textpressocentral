/* 
 * File:   CreateCurationForm.h
 * Author: mueller
 *
 * Created on April 17, 2015, 3:33 PM
 */

#ifndef CREATECURATIONFORM_H
#define	CREATECURATIONFORM_H

#include "Session.h"
#include "CurationFieldConfiguration.h"
#include "CurationFormsConfiguration.h"
#include "AutocompleteAndValidationDialog.h"
#include "PrePopulationDialog.h"
#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WSuggestionPopup>
#include <Wt/WTextArea>

class CreateCurationForm : public Wt::WContainerWidget {
public:
    CreateCurationForm(Session * session, Wt::WContainerWidget * parent = NULL);

    inline Wt::Signal<void> & FormSavedSignal() {
        return formsaved_;
    }
    virtual ~CreateCurationForm();
private:

    struct fieldinfo {
        int x;
        int y;
        Wt::WContainerWidget * cont;
        Wt::WLineEdit * fieldnameedit;
        Wt::WLengthValidator * fnvalidator;
        Wt::WText * errortext;
        Wt::WButtonGroup * ftbgroup;
        Wt::WText * ftbgrouplabel;
        Wt::WButtonGroup * checkboxdefaultbgroup;
        AutocompleteAndValidationDialog * avd;
        Wt::WTextArea * combotextarea;
        PrePopulationDialog * ppd;
    };
    bool saveflag_;
    std::vector<fieldinfo*> fieldinfolist_;
    std::vector<fieldinfo*> fieldinfolistloaded_;
    Session * session_;
    CurationFormsConfiguration * cfc_;
    //std::string formname_;
    //std::string posturl_;
    std::string parameterformat_;
    std::vector<std::string> sharedwith_;
    Wt::WContainerWidget * rightsidecont_;
    Wt::WVBoxLayout * rightsidelayout_;
    Wt::WContainerWidget * nuscont_;
    Wt::WContainerWidget * gridcont_;
    Wt::WGridLayout * grid_;
    Wt::WContainerWidget * entrycont_;
    Wt::WLineEdit * formnameedit_;
    Wt::WSuggestionPopup * formnamesuggestionpopup_;
    Wt::WComboBox * posturlmodedropdown_;
    Wt::WLineEdit * posturledit_;
    Wt::WLineEdit * sharewithedit_;
    Wt::WText * formnamelabel_;
    Wt::WText * formnametext_;
    Wt::WText * posturllabel_;
    bool urleditisaddress_;
    Wt::WText * posturltext_;
    Wt::WText * paramformatlabel_;
    Wt::WText * paramformattext_;
    Wt::WText * sharewithlabel_;
    Wt::WText * sharewithtext_;
    Wt::WPushButton * savebutton_;
    Wt::Signal<void> formsaved_;
    void SessionLoginChanged();
    void PopulateEntryForm();
    void PopulateNUSCont();
    void PopulateFormNameSuggestions();
    void UpdateShareWithText();
    void FormnameEditEnterPressed();
    void PosturlEditEnterPressed();
    void ShareWithEditEnterPressed();
    void AddGreenPlus(int row, int col);
    void GreenCrossClicked(int x, int y, Wt::WContainerWidget * item);
    void RedMinusClicked(int x, int y, Wt::WContainerWidget * item);
    fieldinfo * FindFieldInfoToTheLeft(int x, int y);
    Wt::WContainerWidget * CreateFieldInfoInterface(fieldinfo * f, int x, int y, Wt::WContainerWidget * cont);
    void FtRadioButtonChecked(fieldinfo * f, Wt::WContainerWidget * wc);
    void ParameterFormatChanged (Wt::WButtonGroup * bg);
    void SaveForm();
    void ResetRightSide();
    void ResetGrid();
    void ResetEntry();
    void ResetForm();
    void LoadFields(std::string formname);
    void SetCursorHand(Wt::WWidget * w);
    bool FieldInfoIsNotSame(fieldinfo * f1, fieldinfo * f2);
    bool FieldInfoValidate(fieldinfo * f);
    void SetSaveFlag(bool b);
    bool ValidateAllFields();
};

#endif	/* CREATECURATIONFORM_H */
