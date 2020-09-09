/* 
 * File:   CreateCurationForm.cpp
 * Author: mueller
 * 
 * Created on April 17, 2015, 3:33 PM
 */

#include "TextpressoCentralGlobalDefinitions.h"
#include "CreateCurationForm.h"
#include "PgPrepopulation.h"
#include "HelpDialog.h"
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WGridLayout>
#include <Wt/WText>
#include <Wt/WLineEdit>
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <Wt/WFont>
#include <Wt/WImage>
#include <Wt/WPanel>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WMessageBox>
#include <Wt/WLengthValidator>
#include <boost/algorithm/string.hpp>
#include <Wt/WTimer>
#include <vector>
#include <Wt/WComboBox>

CreateCurationForm::CreateCurationForm(Session * session, Wt::WContainerWidget * parent)
: Wt::WContainerWidget(parent) {
    saveflag_ = false;
    session_ = session;
    session_->login().changed().connect(this, &CreateCurationForm::SessionLoginChanged);
    cfc_ = new CurationFormsConfiguration(PGCURATIONFORMS, PGCURATIONFORMSTABLENAME);

    //
    Wt::WImage * im = new Wt::WImage("resources/icons/qmark15.png");
    im->setVerticalAlignment(Wt::AlignTop);
    im->setInline(true);
    Wt::WText * explanationtext = new Wt::WText(" Click on question mark for help!");
    explanationtext->setInline(true);
    im->mouseWentOver().connect(std::bind([ = ] (){
        im->decorationStyle().setCursor(Wt::PointingHandCursor);
    }));
    im->clicked().connect(std::bind([ = ] (){
        Wt::WContainerWidget * helpcontent = new Wt::WContainerWidget();
        Wt::WText * general1 = new Wt::WText();
        general1->setText(
        "This form consist of two panels. In the right panel you can create "
        "and edit the form fields. In the left panel additional information "
        "for the form can be entered. "
        );
        helpcontent->addWidget(general1);
        helpcontent->addWidget(new Wt::WBreak());
        helpcontent->addWidget(new Wt::WBreak());
        Wt::WText * caption1 = new Wt::WText("Form Name: ");
        Wt::WText * expl1 = new Wt::WText(
        "The name identifies the form from others. The line edit has "
        "autocomplete functionality populated with names of forms contained in the "
        "database. When picking an already existing form, it will be loaded.");
        caption1->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption1);
        helpcontent->addWidget(expl1);
        helpcontent->addWidget(new Wt::WBreak());
        Wt::WText * caption2 = new Wt::WText("URL for Posting to External Database: ");
        caption2->decorationStyle().font().setWeight(Wt::WFont::Bold);
        Wt::WText * expl2 = new Wt::WText(
        "This is the URL of the server that receives the form data submitted. "
        "The server should be able to receive a HTTP Post. Data can be posted "
        "as a JSON (See <a href='http://json.org'>json.org</a>) object or as "
        "URI parameters. The URL address can be either specified directly, or "
        "the system can be instructed to take the value an incoming URL "
        "parameter to carry the address. In latter case the name of that variable "
        "must be specified instead."
        );
        helpcontent->addWidget(caption2);
        helpcontent->addWidget(expl2);
        helpcontent->addWidget(new Wt::WBreak());
        Wt::WText * caption3 = new Wt::WText("Share with: ");
        Wt::WText * expl3 = new Wt::WText(
        "If you want to share the form with other curators, enter their login "
        "name here. They will be able to submit data to the server whose URL "
        "you entered in the URL field."
        );
        caption3->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption3);
        helpcontent->addWidget(expl3);
        helpcontent->addWidget(new Wt::WBreak());
        Wt::WText * caption4 = new Wt::WText("Reset (Button): ");
        Wt::WText * expl4 = new Wt::WText("Resets the whole web page.");
        caption4->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption4);
        helpcontent->addWidget(expl4);
        helpcontent->addWidget(new Wt::WBreak());
        Wt::WText * caption5 = new Wt::WText("Save (Button): ");
        Wt::WText * expl5 = new Wt::WText(
        "Saves the form. After the form is saved, it can be used in the curation "
        "tab. It's name should be available in the 'Select Form' drop-down menu "
        "in the curation form."
        );
        caption5->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption5);
        helpcontent->addWidget(expl5);
        helpcontent->addWidget(new Wt::WBreak());
        Wt::WImage * plus = new Wt::WImage("resources/icons/green-cross.png");
        Wt::WText * caption6 = new Wt::WText("Plus Button ");
        Wt::WText * caption6a = new Wt::WText(": ");
        caption6->decorationStyle().font().setWeight(Wt::WFont::Bold);
        caption6a->decorationStyle().font().setWeight(Wt::WFont::Bold);
        Wt::WText * expl6 = new Wt::WText(
        "Clicking on this button opens another form to enter specifications of "
        "a field. It also spawns more plus buttons to the right on bottom of the "
        "current field. The geometry of the form should be preserved according to "
        "how it is seen here. The form for field specification contains more help "
        "items. Click on the question mark in the form."
        );
        helpcontent->addWidget(caption6);
        helpcontent->addWidget(plus);
        helpcontent->addWidget(caption6a);
        helpcontent->addWidget(expl6);
        caption6->setInline(true);
        plus->setInline(true);
        caption6a->setInline(true);
        helpcontent->addWidget(new Wt::WBreak());
        helpcontent->setWidth(Wt::WLength(48, Wt::WLength::FontEx));
        helpcontent->setPadding(Wt::WLength(1, Wt::WLength::FontEx));
        helpcontent->setContentAlignment(Wt::AlignJustify);
        new HelpDialog("Curation Form Explanation", false, helpcontent);
    }));
    //
    rightsidecont_ = new Wt::WContainerWidget();
    rightsidelayout_ = new Wt::WVBoxLayout();
    rightsidecont_->setLayout(rightsidelayout_);
    nuscont_ = new Wt::WContainerWidget();
    gridcont_ = new Wt::WContainerWidget();
    grid_ = new Wt::WGridLayout();
    gridcont_->setLayout(grid_);
    entrycont_ = new Wt::WContainerWidget();
    entrycont_->setMaximumSize(Wt::WLength(40, Wt::WLength::FontEx), Wt::WLength::Auto);
    rightsidelayout_->addWidget(nuscont_);
    rightsidelayout_->addWidget(gridcont_);
    Wt::WContainerWidget * formcontent = new Wt::WContainerWidget();
    Wt::WHBoxLayout * hbox = new Wt::WHBoxLayout();
    formcontent->setLayout(hbox);
    PopulateEntryForm();
    PopulateNUSCont();
    AddGreenPlus(0, 0);
    hbox->addWidget(entrycont_);
    hbox->addWidget(rightsidecont_, Wt::AlignLeft);
    addWidget(formcontent);
    addWidget(im);
    addWidget(explanationtext);
}

void CreateCurationForm::SessionLoginChanged() {
    PopulateFormNameSuggestions();
}

void CreateCurationForm::ResetRightSide() {
    ResetGrid();
    AddGreenPlus(0, 0);
    nuscont_->clear();
    PopulateNUSCont();
}

void CreateCurationForm::ResetGrid() {
    gridcont_->clear();
    grid_ = new Wt::WGridLayout();
    gridcont_->setLayout(grid_);
}

void CreateCurationForm::ResetEntry() {
    entrycont_->clear();
    PopulateEntryForm();
}

void CreateCurationForm::ResetForm() {
    ResetRightSide();
    ResetEntry();
}

void CreateCurationForm::PopulateEntryForm() {
    Wt::WVBoxLayout * vbox = new Wt::WVBoxLayout();
    entrycont_->setLayout(vbox);
    //
    formnameedit_ = new Wt::WLineEdit();
    formnameedit_->setEmptyText("(required!)");
    formnameedit_->enterPressed().connect(boost::bind(&CreateCurationForm::FormnameEditEnterPressed, this));
    // Set options for name;
    Wt::WSuggestionPopup::Options nameoptions;
    nameoptions.highlightBeginTag = "<span class=\"highlight\">";
    nameoptions.highlightEndTag = "</span>";
    nameoptions.listSeparator = ',';
    nameoptions.whitespace = " ";
    nameoptions.wordSeparators = "-., \";";
    formnamesuggestionpopup_ = new Wt::WSuggestionPopup(
            Wt::WSuggestionPopup::generateMatcherJS(nameoptions),
            Wt::WSuggestionPopup::generateReplacerJS(nameoptions));
    formnamesuggestionpopup_->forEdit(formnameedit_);
    // Populate the underlying model with suggestions:
    PopulateFormNameSuggestions();
    //
    posturlmodedropdown_ = new Wt::WComboBox();
    posturlmodedropdown_->addItem("URL address:");
    posturlmodedropdown_->addItem("Incoming URL parameter:");
    posturlmodedropdown_->setCurrentIndex(0);
    posturledit_ = new Wt::WLineEdit();
    posturledit_->enterPressed().connect(boost::bind(&CreateCurationForm::PosturlEditEnterPressed, this));
    //
    Wt::WContainerWidget * groupcont = new Wt::WContainerWidget();
    Wt::WButtonGroup * paramformatgroup = new Wt::WButtonGroup(groupcont);
    paramformatgroup->checkedChanged().connect(boost::bind(&CreateCurationForm::ParameterFormatChanged, this, paramformatgroup));
    Wt::WRadioButton * button;
    button = new Wt::WRadioButton("JSON", groupcont);
    paramformatgroup->addButton(button);
    button = new Wt::WRadioButton("URI", groupcont);
    paramformatgroup->addButton(button);
    //
    sharewithedit_ = new Wt::WLineEdit();
    sharewithedit_->enterPressed().connect(boost::bind(&CreateCurationForm::ShareWithEditEnterPressed, this));
    //
    Wt::WContainerWidget * auxcont = new Wt::WContainerWidget();
    //
    auxcont->addWidget(new Wt::WText("Form Name (press enter to populate field)"));
    auxcont->addWidget(formnameedit_);
    vbox->addWidget(auxcont);
    //
    auxcont = new Wt::WContainerWidget();
    auxcont->addWidget(new Wt::WText("URL for Posting to External Database (press enter to populate field)"));
    auxcont->addWidget(posturlmodedropdown_);
    auxcont->addWidget(posturledit_);
    vbox->addWidget(auxcont);
    //
    auxcont = new Wt::WContainerWidget();
    Wt::WText * grouptext = new Wt::WText("Format for Sending Data: ");
    grouptext->setVerticalAlignment(Wt::AlignBottom);
    grouptext->setInline(true);
    groupcont->setInline(true);
    auxcont->addWidget(grouptext);
    auxcont->addWidget(groupcont);
    vbox->addWidget(auxcont);
    //
    // for now hide the 'Share with' option
    //auxcont = new Wt::WContainerWidget();
    //auxcont->addWidget(new Wt::WText("Share with (use loginname, separate with 'Enter' for each name)"));
    //auxcont->addWidget(sharewithedit_);
    //vbox->addWidget(auxcont);
    //
    //
    auxcont = new Wt::WContainerWidget();
    Wt::WPushButton * resetbutton = new Wt::WPushButton("Reset");
    resetbutton->clicked().connect(this, &CreateCurationForm::ResetForm);
    savebutton_ = new Wt::WPushButton("Save");
    savebutton_->clicked().connect(this, &CreateCurationForm::SaveForm);
    savebutton_->disable();
    auxcont->addWidget(resetbutton);
    auxcont->addWidget(savebutton_);
    vbox->addWidget(auxcont);
}

void CreateCurationForm::PopulateFormNameSuggestions() {
    if (session_->login().state() != 0) {
        std::string username(session_->login().user().identity("loginname").toUTF8());
        for (long unsigned int i = 0; i < cfc_->GetTableSize(); i++)
            if (username.compare(cfc_->GetOwner(i)) == 0)
                formnamesuggestionpopup_->addSuggestion(Wt::WString(cfc_->GetFormName(i)));
    }
}

void CreateCurationForm::PopulateNUSCont() {
    formnamelabel_ = new Wt::WText("Form name: ");
    formnamelabel_->decorationStyle().font().setWeight(Wt::WFont::Bold);
    formnamelabel_->setInline(true);
    formnametext_ = new Wt::WText();
    formnametext_->setInline(true);
    posturllabel_ = new Wt::WText("Address: ");
    posturllabel_->decorationStyle().font().setWeight(Wt::WFont::Bold);
    posturllabel_->setInline(true);
    posturltext_ = new Wt::WText();
    posturltext_->setInline(true);
    paramformatlabel_ = new Wt::WText("Format for Sending Data: ");
    paramformatlabel_->decorationStyle().font().setWeight(Wt::WFont::Bold);
    paramformatlabel_->setInline(true);
    paramformattext_ = new Wt::WText();
    paramformattext_->setInline(true);
    sharewithlabel_ = new Wt::WText("Shared with: ");
    sharewithlabel_->decorationStyle().font().setWeight(Wt::WFont::Bold);
    sharewithlabel_->setInline(true);
    sharewithtext_ = new Wt::WText();
    sharewithtext_->setInline(true);
    //
    nuscont_->addWidget(formnamelabel_);
    nuscont_->addWidget(formnametext_);
    nuscont_->addWidget(new Wt::WBreak());
    nuscont_->addWidget(posturllabel_);
    nuscont_->addWidget(posturltext_);
    nuscont_->addWidget(new Wt::WBreak());
    nuscont_->addWidget(paramformatlabel_);
    nuscont_->addWidget(paramformattext_);
    nuscont_->addWidget(new Wt::WBreak());
    //
    // disable shared with field for now.
    //nuscont_->addWidget(sharewithlabel_);
    //nuscont_->addWidget(sharewithtext_);
    //nuscont_->addWidget(new Wt::WBreak());
}

void CreateCurationForm::AddGreenPlus(int row, int col) {
    Wt::WContainerWidget * wc = new Wt::WContainerWidget();
    wc->setMinimumSize(Wt::WLength::Auto, Wt::WLength(24, Wt::WLength::FontEx));
    Wt::WImage * plus = new Wt::WImage("resources/icons/green-cross.png");
    plus->setMaximumSize(Wt::WLength(24, Wt::WLength::Pixel), Wt::WLength(24, Wt::WLength::Pixel));
    plus->mouseWentOver().connect(boost::bind(&CreateCurationForm::SetCursorHand, this, plus));
    plus->clicked().connect(boost::bind(&CreateCurationForm::GreenCrossClicked, this, row, col, wc));
    wc->addWidget(plus);
    if (FindFieldInfoToTheLeft(row, col) != NULL) {
        Wt::WImage * minus = new Wt::WImage("resources/icons/red-minus.png");
        minus->setMaximumSize(Wt::WLength(24, Wt::WLength::Pixel), Wt::WLength(24, Wt::WLength::Pixel));
        minus->mouseWentOver().connect(boost::bind(&CreateCurationForm::SetCursorHand, this, minus));
        minus->clicked().connect(boost::bind(&CreateCurationForm::RedMinusClicked, this, row, col, wc));
        wc->addWidget(minus);
    }
    //
    grid_->addWidget(wc, row, col);
}

CreateCurationForm::fieldinfo * CreateCurationForm::FindFieldInfoToTheLeft(int x, int y) {
    fieldinfo * ret = NULL;
    for (std::vector<fieldinfo*>::iterator it = fieldinfolist_.begin(); it != fieldinfolist_.end(); it++) {
        if ((*(*it)).x == x)
            if ((*(*it)).y == y - 1)
                ret = (*it);
    }
    return ret;
}

void CreateCurationForm::GreenCrossClicked(int x, int y, Wt::WContainerWidget * item) {
    item->clear();
    fieldinfo * f = new fieldinfo;
    CreateFieldInfoInterface(f, x, y, item);
    AddGreenPlus(x + 1, y);
    AddGreenPlus(x, y + 1);
}

void CreateCurationForm::RedMinusClicked(int x, int y, Wt::WContainerWidget * item) {
    fieldinfo * f = FindFieldInfoToTheLeft(x, y);
    if (f != NULL) {
        (*f).cont->clear();
        AddGreenPlus((*f).x, (*f).y);
        std::vector<fieldinfo*>::iterator it2becleared
                = std::find(fieldinfolist_.begin(), fieldinfolist_.end(), f);
        fieldinfolist_.erase(it2becleared);
        grid_->removeWidget(item);
        delete item;
        delete f;
    }
}

Wt::WContainerWidget * CreateCurationForm::CreateFieldInfoInterface(fieldinfo * f, int x, int y, Wt::WContainerWidget * cont) {
    (*f).x = x;
    (*f).y = y;
    (*f).cont = cont;
    Wt::WPanel * panel = new Wt::WPanel();
    panel->setTitleBar(false);
    Wt::WContainerWidget * centralcont = new Wt::WContainerWidget();
    panel->setCentralWidget(centralcont);
    cont->addWidget(panel);
    //
    (*f).errortext = new Wt::WText();
    Wt::WText * fnetext = new Wt::WText("Field name: ");
    (*f).fieldnameedit = new Wt::WLineEdit();
    (*f).fieldnameedit->setMaximumSize(Wt::WLength(50, Wt::WLength::FontEx), Wt::WLength(2, Wt::WLength::FontEx));
    (*f).fnvalidator = new Wt::WLengthValidator(2, 64);
    (*f).fnvalidator->setMandatory(true);
    (*f).fnvalidator->setInvalidBlankText("Field name cannot be empty.");
    (*f).fnvalidator->setInvalidTooShortText("Field name must have at least 2 characters.");
    (*f).fnvalidator->setInvalidTooLongText("Field name cannot be longer than 63 characters.");
    (*f).fieldnameedit->setValidator((*f).fnvalidator);
    (*f).fieldnameedit->changed().connect(boost::bind(&CreateCurationForm::FieldInfoValidate, this, f));
    centralcont->addWidget((*f).errortext);
    centralcont->addWidget(new Wt::WBreak());
    centralcont->addWidget(fnetext);
    centralcont->addWidget((*f).fieldnameedit);
    fnetext->setInline(true);
    (*f).fieldnameedit->setInline(true);
    //
    centralcont->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * bgroupcont = new Wt::WContainerWidget();
    (*f).ftbgrouplabel = new Wt::WText("Field type: ");
    (*f).ftbgroup = new Wt::WButtonGroup(bgroupcont);
    (*f).ftbgroup->checkedChanged().connect(boost::bind(&CreateCurationForm::FieldInfoValidate, this, f));
    Wt::WRadioButton * button;
    button = new Wt::WRadioButton("Line edit", bgroupcont);
    (*f).ftbgroup->addButton(button);
    button = new Wt::WRadioButton("Text area", bgroupcont);
    (*f).ftbgroup->addButton(button);
    button = new Wt::WRadioButton("Check box", bgroupcont);
    (*f).ftbgroup->addButton(button);
    button = new Wt::WRadioButton("Drop-down list", bgroupcont);
    (*f).ftbgroup->addButton(button);
    button = new Wt::WRadioButton("List maker", bgroupcont);
    (*f).ftbgroup->addButton(button);
    Wt::WContainerWidget * ret = new Wt::WContainerWidget();
    (*f).ftbgroup->checkedChanged().connect(boost::bind(&CreateCurationForm::FtRadioButtonChecked,
            this, f, ret));
    (*f).checkboxdefaultbgroup = NULL;
    (*f).ftbgrouplabel->setInline(true);
    bgroupcont->setInline(true);
    (*f).ftbgrouplabel->setVerticalAlignment(Wt::AlignBottom);
    centralcont->addWidget((*f).ftbgrouplabel);
    centralcont->addWidget(bgroupcont);
    centralcont->addWidget(new Wt::WBreak());
    centralcont->addWidget(ret);
    fieldinfolist_.push_back(f);
    //
    Wt::WImage * qmark = new Wt::WImage("resources/icons/qmark15.png");
    centralcont->addWidget(qmark);
    qmark->mouseWentOver().connect(std::bind([ = ] (){
        qmark->decorationStyle().setCursor(Wt::PointingHandCursor);
    }));
    qmark->clicked().connect(std::bind([ = ] (){
        Wt::WContainerWidget * helpcontent = new Wt::WContainerWidget();
        Wt::WText * general1 = new Wt::WText();
        general1->setText(
        "The field form allows you to specify a data entry field. Each field "
        "consist of a name and an data entry. Data entry can be assisted by "
        "an autocomplete or pre-population functionality. Data entries can be "
        "validated"
        );
        helpcontent->addWidget(general1);
        helpcontent->addWidget(new Wt::WBreak());
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption1 = new Wt::WText("Field Name: ");
        Wt::WText * expl1 = new Wt::WText("This is the name of the data entry field. ");
        caption1->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption1);
        helpcontent->addWidget(expl1);
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption2 = new Wt::WText("Field Type: ");
        Wt::WText * expl2 = new Wt::WText(
        "Specify the type of the form for the field. 'Line Edit' should be used "
        "when text to be entered fits in one line, 'Text Area' is suitable for longer, "
        "free text entries. 'Check-box' is a binary data entry (yes/no), "
        "and 'Drop-down' restricts the user to choose from a "
        "predefined list of data entries. 'List Maker' is similar to "
        "'Text Area' (multiple lines of free text), except that you "
        "can have each line validated or autocompleted "
        "separately, which you cannot conveniently do for 'Text Area'. "
        );
        caption2->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption2);
        helpcontent->addWidget(expl2);
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption3 = new Wt::WText("Edit Autocomplete & Validation: ");
        Wt::WText * expl3 = new Wt::WText(
        "Pushing this button opens another window where you can enter "
        "autocomplete and validation info. In both cases a column of a Postgres "
        "table can be specified from which you would like the actual suggestion and "
        "validation info to be retrieved."
        );
        caption3->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption3);
        helpcontent->addWidget(expl3);
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption4 = new Wt::WText("Prepopulate Field: ");
        Wt::WText * expl4 = new Wt::WText(
        "Opens another window where details about prepopulating a data entry can "
        "be entered. The prepopulating can be based on the contents of the text spans "
        "marked up in the paper viewer (curation tab)."
        );
        caption4->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption4);
        helpcontent->addWidget(expl4);
        helpcontent->addWidget(new Wt::WBreak());

        helpcontent->setWidth(Wt::WLength(48, Wt::WLength::FontEx));
        helpcontent->setPadding(Wt::WLength(1, Wt::WLength::FontEx));
        helpcontent->setContentAlignment(Wt::AlignJustify);
        new HelpDialog("Field Form Explanation", false, helpcontent);
    }));
    //
    return ret;
}

void CreateCurationForm::FtRadioButtonChecked(fieldinfo * f, Wt::WContainerWidget * wc) {
    wc->clear();
    if ((*f).ftbgroup->checkedId() == CurationFieldConfiguration::checkbox) {
        Wt::WContainerWidget * bgroupcont = new Wt::WContainerWidget();
        Wt::WText * ftrb = new Wt::WText("Should checkbox be checked by default: ");
        (*f).checkboxdefaultbgroup = new Wt::WButtonGroup(bgroupcont);
        Wt::WRadioButton * button;
        button = new Wt::WRadioButton("yes", bgroupcont);
        (*f).checkboxdefaultbgroup->addButton(button);
        button = new Wt::WRadioButton("no", bgroupcont);
        (*f).checkboxdefaultbgroup->addButton(button);
        (*f).checkboxdefaultbgroup->setSelectedButtonIndex(1);
        ftrb->setInline(true);
        bgroupcont->setInline(true);
        wc->addWidget(ftrb);
        wc->addWidget(bgroupcont);
    } else if ((*f).ftbgroup->checkedId() == CurationFieldConfiguration::combobox) {
        Wt::WContainerWidget * comboinfocont = new Wt::WContainerWidget();
        Wt::WText * helptext = new Wt::WText("Add items, one per line: ");
        (*f).combotextarea = new Wt::WTextArea();
        (*f).combotextarea->resize(Wt::WLength::Auto, Wt::WLength(6, Wt::WLength::FontEx));
        wc->addWidget(helptext);
        wc->addWidget((*f).combotextarea);
    } else {
        bool valdialog;
        if (((*f).ftbgroup->checkedId() == CurationFieldConfiguration::lineedit) ||
                ((*f).ftbgroup->checkedId() == CurationFieldConfiguration::listmaker))
            valdialog = true;
        else
            valdialog = false;
        (*f).avd = new AutocompleteAndValidationDialog(valdialog);
        Wt::WPushButton * avdeditbutton = new Wt::WPushButton("Edit Autocomplete & Validation");
        avdeditbutton->setStyleClass("btn-small");
        avdeditbutton->clicked().connect(std::bind([ = ] (){
            (*f).avd->SetTitle((*f).fieldnameedit->text());
            (*f).avd->show();
        }));
        (*f).ppd = new PrePopulationDialog(session_);
        Wt::WPushButton * prepopbutton = new Wt::WPushButton("Prepopulate Field");
        prepopbutton->setStyleClass("btn-small");
        prepopbutton->clicked().connect(std::bind([ = ] (){
            (*f).ppd->SetTitle((*f).fieldnameedit->text());
            (*f).ppd->show();
        }));
        avdeditbutton->setInline(true);
        prepopbutton->setInline(true);
        wc->addWidget(avdeditbutton);
        wc->addWidget(prepopbutton);
    }
}

bool CreateCurationForm::FieldInfoValidate(fieldinfo * f) {
    bool ret;
    Wt::WString errorstring("");
    Wt::WBorder bx;
    bx.setStyle(Wt::WBorder::Solid);
    Wt::WValidator::Result result = (*f).fnvalidator->validate((*f).fieldnameedit->text());
    if (result.state() == Wt::WValidator::Valid) {
        bx.setColor(Wt::black);
        bx.setWidth(Wt::WBorder::Thin);
        (*f).fieldnameedit->decorationStyle().setBorder(bx);
    } else {
        bx.setColor(Wt::WColor(185, 74, 72));
        bx.setWidth(Wt::WBorder::Medium);
        (*f).fieldnameedit->decorationStyle().setBorder(bx);
        errorstring += result.message();
    }
    if ((*f).ftbgroup->checkedId() != -1) {
        (*f).ftbgrouplabel->decorationStyle().setForegroundColor(Wt::black);
    } else {
        (*f).ftbgrouplabel->decorationStyle().setForegroundColor(Wt::WColor(185, 74, 72));
        if (!errorstring.empty()) errorstring += " ";
        errorstring += "Field type must be specified.";
    }
    if (formnametext_->text().empty()) errorstring += " Form must have a name (above).";
    if (posturltext_->text().empty()) errorstring += " Form must have a post URL (above).";
    (*f).errortext->setText(errorstring);
    if (errorstring.empty()) {
        ret = true;
        (*f).errortext->setStyleClass("");
    } else {
        ret = false;
        (*f).errortext->setStyleClass("alert-danger");
    }
    SetSaveFlag(ret);
    return ret;
}

void CreateCurationForm::SaveForm() {
    std::string fn(formnametext_->text().toUTF8());
    std::string ow(session_->login().user().identity("loginname").toUTF8());
    std::string pr(sharewithtext_->text().toUTF8());
    std::string po;
    po = (urleditisaddress_) ? (posturltext_->text().toUTF8()) : "$" + posturltext_->text().toUTF8() + "$";
    std::string fo(paramformattext_->text().toUTF8());
    if (fieldinfolistloaded_.size() == fieldinfolist_.size()) {
        for (long unsigned int i = 0; i < fieldinfolist_.size(); i++)
            if (FieldInfoIsNotSame(fieldinfolist_[i], fieldinfolistloaded_[i])) {
                saveflag_ = true;
                continue;
            }
    } else {
        saveflag_ = true;
    }
    ValidateAllFields();
    if (saveflag_) {
        if (!(fieldinfolist_.empty() || fn.empty())) {
            CurationFieldConfiguration cfic(PGCURATIONFIELDS, PGCURATIONFIELDSTABLENAME, fn);
            for (long unsigned int i = 0; i < fieldinfolist_.size(); i++) {
                std::string fieldname = (*fieldinfolist_[i]).fieldnameedit->text().toUTF8();
                CurationFieldConfiguration::fieldtype ft
                        = (CurationFieldConfiguration::fieldtype)
                        (*fieldinfolist_[i]).ftbgroup->checkedId();
                int x = (*fieldinfolist_[i]).x;
                int y = (*fieldinfolist_[i]).y;
                bool b;
                if ((*fieldinfolist_[i]).checkboxdefaultbgroup != NULL)
                    b = ((*fieldinfolist_[i]).checkboxdefaultbgroup->checkedId() == 0);
                else
                    b = false;
                CurationFieldConfiguration::pginfo s = {}, v = {};
                int prepopid(-1);
                if ((ft == CurationFieldConfiguration::lineedit) ||
                        (ft == CurationFieldConfiguration::textarea) ||
                        (ft == CurationFieldConfiguration::listmaker)) {
                    AutocompleteAndValidationDialog::forminfo sinp;
                    sinp = (*fieldinfolist_[i]).avd->GetAutocompleteInfo();
                    s.col = sinp.columnname->text().toUTF8();
                    s.tablename = sinp.tablename->text().toUTF8();
                    std::string where = sinp.whereclause->text().toUTF8();
                    boost::replace_all(where, "'", "''");
                    s.whereclause = where;
                    std::string db("");
                    if (!sinp.dbname->text().empty())
                        db += "dbname=" + sinp.dbname->text().toUTF8();
                    if (!db.empty()) db += " ";
                    if (!sinp.hostname->text().empty())
                        db += "host=" + sinp.hostname->text().toUTF8();
                    if (!db.empty()) db += " ";
                    if (!sinp.port->text().empty())
                        db += "port=" + sinp.port->text().toUTF8();
                    if (!db.empty()) db += " ";
                    if (!sinp.username->text().empty())
                        db += "user=" + sinp.username->text().toUTF8();
                    if (!db.empty()) db += " ";
                    if (!sinp.password->text().empty())
                        db += "password=" + sinp.password->text().toUTF8();
                    s.database = db;
                    if ((*fieldinfolist_[i]).avd->HasValidationInfo()) {
                        AutocompleteAndValidationDialog::forminfo vinp;
                        vinp = (*fieldinfolist_[i]).avd->GetValidationInfo();
                        v.col = vinp.columnname->text().toUTF8();
                        v.tablename = vinp.tablename->text().toUTF8();
                        where = vinp.whereclause->text().toUTF8();
                        boost::replace_all(where, "'", "''");
                        v.whereclause = where;
                        std::string db("");
                        if (!vinp.dbname->text().empty())
                            db += "dbname=" + vinp.dbname->text().toUTF8();
                        if (!db.empty()) db += " ";
                        if (!vinp.hostname->text().empty())
                            db += "host=" + vinp.hostname->text().toUTF8();
                        if (!db.empty()) db += " ";
                        if (!vinp.port->text().empty())
                            db += "port=" + vinp.port->text().toUTF8();
                        if (!db.empty()) db += " ";
                        if (!vinp.username->text().empty())
                            db += "user=" + vinp.username->text().toUTF8();
                        if (!db.empty()) db += " ";
                        if (!vinp.password->text().empty())
                            db += "password=" + vinp.password->text().toUTF8();
                        v.database = db;
                    }
                    if ((*fieldinfolist_[i]).ppd->HasPrePopInfo()) {
                        PgPrepopulation * ppp = new PgPrepopulation(PGPREPOPULATION, PGPREPOPULATIONTABLENAME);
                        prepopid = ppp->GetNewPrePopId();
                        int mode = (*fieldinfolist_[i]).ppd->GetPrePopMode();
                        std::string data = (*fieldinfolist_[i]).ppd->GetPrePopString().toUTF8();
                        std::string database = (*fieldinfolist_[i]).ppd->GetSynDataBaseInfo().database;
                        std::string tablename = (*fieldinfolist_[i]).ppd->GetSynDataBaseInfo().tablename;
                        std::string colname = (*fieldinfolist_[i]).ppd->GetSynDataBaseInfo().col;
                        std::string whereclause = (*fieldinfolist_[i]).ppd->GetSynDataBaseInfo().whereclause;
                        ppp->SaveTableData(prepopid, mode, data,
                                database, tablename, colname, whereclause);
                    }
                }
                std::string combochoices("");
                if (ft == CurationFieldConfiguration::combobox) {
                    std::vector<std::string> splits;
                    std::string inp((*fieldinfolist_[i]).combotextarea->text().toUTF8());
                    boost::split(splits, inp, boost::is_any_of("\t\n"));
                    if (!splits.empty()) {
                        for (int i = 0; i < splits.size(); i++) {
                            if (i > 0) combochoices += "|";
                            combochoices += splits[i];
                        }
                    }
                }
                cfic.SaveField(fieldname, ft, x, y, b, s, v, combochoices, prepopid);
            }
            ResetRightSide();


            Wt::WMessageBox * savednote = new Wt::WMessageBox("Saved!", "Your form "
                    "has been saved. If you want edit it later on, type its name ("
                    + Wt::WString(fn) +
                    ") in the form name box and hit 'Enter.'", Wt::Information, Wt::Ok);
            savednote->buttonClicked().connect(std::bind([ = ] (){
                delete savednote;
            }));
            savednote->show();



        }
        if (!fn.empty()) {
            cfc_->SaveForm(fn, ow, pr, po, fo);
            ResetEntry();
            saveflag_ = false;
            savebutton_->disable();
            formsaved_.emit();
            fieldinfolist_.clear();
            fieldinfolistloaded_.clear();
        }
    } else if (fn.empty()) {
        if (formnametext_->text().empty()) {
            formnametext_->setText("Form name needs to be defined!");
            formnametext_->decorationStyle().setForegroundColor(Wt::red);
        }
        Wt::WTimer * timer = new Wt::WTimer();
        timer->setInterval(3000);
        timer->setSingleShot(true);
        timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
            timer->stop();
            delete timer;
            formnametext_->setText("");
            formnametext_->decorationStyle().setForegroundColor(Wt::black);
        }, timer));
        timer->start();
    } else if (fieldinfolist_.empty()) {
        Wt::WText * warning = new Wt::WText("At least one field needs to be defined.");
        warning->decorationStyle().setForegroundColor(Wt::red);
        ResetGrid();
        grid_->addWidget(warning, 0, 0);
        Wt::WTimer * timer = new Wt::WTimer();
        timer->setInterval(3000);
        timer->setSingleShot(true);
        timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
            timer->stop();
            delete timer;
            ResetGrid();
            AddGreenPlus(0, 0);
        }, timer));
        timer->start();
    }
}

void CreateCurationForm::LoadFields(std::string formname) {
    fieldinfolist_.clear();
    CurationFieldConfiguration cfic(PGCURATIONFIELDS,
            PGCURATIONFIELDSTABLENAME, formname);
    ResetGrid();
    for (long unsigned int i = 0; i < cfic.NumberOfFields(); i++) {
        std::string fieldname(cfic.GetFieldName(i));
        CurationFieldConfiguration::fieldtype fieldtype(cfic.GetFieldType(i));
        int x(cfic.GetX(i));
        int y(cfic.GetY(i));
        bool checkboxdefault(cfic.GetCheckboxDefault(i));
        Wt::WContainerWidget * wc = new Wt::WContainerWidget();
        wc->setMinimumSize(Wt::WLength(50, Wt::WLength::FontEx), Wt::WLength(15, Wt::WLength::FontEx));
        fieldinfo * f = new fieldinfo;
        // (*f).x and (*f).y are set in CreateFieldInfoInterface.
        Wt::WContainerWidget * aux = CreateFieldInfoInterface(f, x, y, wc);
        (*f).fieldnameedit->setText(Wt::WString(fieldname));
        (*f).ftbgroup->setSelectedButtonIndex(fieldtype);
        FtRadioButtonChecked(f, aux);
        if (fieldtype == CurationFieldConfiguration::checkbox) {
            if (checkboxdefault)
                (*f).checkboxdefaultbgroup->setSelectedButtonIndex(0);
            else
                (*f).checkboxdefaultbgroup->setSelectedButtonIndex(1);
        } else if (fieldtype == CurationFieldConfiguration::combobox) {
            std::string aux(cfic.GetComboChoice(i));
            boost::replace_all(aux, "|", "\n");
            aux = "\n" + aux; // need to add extra empty line in case the first choice is an empty line already
            // this is an oddity in setText method below.
            (*f).combotextarea->setText(Wt::WString(aux));
        } else {
            CurationFieldConfiguration::pginfo s = cfic.GetSuggestionBoxInfo(i);
            Wt::WLineEdit * laux = (*f).avd->GetAutocompleteInfo().columnname;
            laux->setText(Wt::WString(s.col));
            laux = (*f).avd->GetAutocompleteInfo().tablename;
            laux->setText(Wt::WString(s.tablename));
            laux = (*f).avd->GetAutocompleteInfo().whereclause;
            laux->setText(Wt::WString(s.whereclause));
            std::vector<std::string> splits;
            boost::split(splits, s.database, boost::is_any_of(" "));
            for (std::vector<std::string>::iterator it = splits.begin();
                    it != splits.end(); it++) {
                if ((*it).find("dbname=") != std::string::npos) {
                    laux = (*f).avd->GetAutocompleteInfo().dbname;
                    laux->setText(Wt::WString((*it).substr(7, (*it).size() - 7)));
                }
                if ((*it).find("host=") != std::string::npos) {
                    laux = (*f).avd->GetAutocompleteInfo().hostname;
                    laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                    laux = (*f).avd->GetAutoCompleteHostName();
                    laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                }
                if ((*it).find("port=") != std::string::npos) {
                    laux = (*f).avd->GetAutocompleteInfo().port;
                    laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                }
                if ((*it).find("user=") != std::string::npos) {
                    laux = (*f).avd->GetAutocompleteInfo().username;
                    laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                }
                if ((*it).find("password=") != std::string::npos) {
                    laux = (*f).avd->GetAutocompleteInfo().password;
                    laux->setText(Wt::WString((*it).substr(9, (*it).size() - 9)));
                }
            }
            if ((*f).avd->HasValidationInfo()) {
                CurationFieldConfiguration::pginfo v = cfic.GetValidationInfo(i);
                laux = (*f).avd->GetValidationInfo().columnname;
                laux->setText(Wt::WString(v.col));
                laux = (*f).avd->GetValidationInfo().tablename;
                laux->setText(Wt::WString(v.tablename));
                laux = (*f).avd->GetValidationInfo().whereclause;
                laux->setText(Wt::WString(v.whereclause));
                boost::split(splits, v.database, boost::is_any_of(" "));
                for (std::vector<std::string>::iterator it = splits.begin();
                        it != splits.end(); it++) {
                    if ((*it).find("dbname=") != std::string::npos) {
                        laux = (*f).avd->GetValidationInfo().dbname;
                        laux->setText(Wt::WString((*it).substr(7, (*it).size() - 7)));
                    }
                    if ((*it).find("host=") != std::string::npos) {
                        laux = (*f).avd->GetValidationInfo().hostname;
                        laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                        laux = (*f).avd->GetValidationHostName();
                        laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                    }
                    if ((*it).find("port=") != std::string::npos) {
                        laux = (*f).avd->GetValidationInfo().port;
                        laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                    }
                    if ((*it).find("user=") != std::string::npos) {
                        laux = (*f).avd->GetValidationInfo().username;
                        laux->setText(Wt::WString((*it).substr(5, (*it).size() - 5)));
                    }
                    if ((*it).find("password=") != std::string::npos) {
                        laux = (*f).avd->GetValidationInfo().password;
                        laux->setText(Wt::WString((*it).substr(9, (*it).size() - 9)));
                    }
                }
            }
            int prepopid(cfic.GetPrePopId(i));
            if (prepopid + 1 > 0) {
                PgPrepopulation * ppp = new PgPrepopulation(PGPREPOPULATION, PGPREPOPULATIONTABLENAME);
                int mode;
                std::string data;
                std::string syndatabase;
                std::string syntablename;
                std::string syncolname;
                std::string synwhereclause;
                ppp->ReadTableData(prepopid, mode, data,
                        syndatabase, syntablename, syncolname, synwhereclause);
                (*f).ppd->SetPrePopString(Wt::WString(data));
                (*f).ppd->SetPrePopMode(mode);
                (*f).ppd->GetSynDataBaseInfo().col = syncolname;
                (*f).ppd->GetSynDataBaseInfo().tablename = syntablename;
                (*f).ppd->GetSynDataBaseInfo().whereclause = synwhereclause;
                (*f).ppd->GetSynDataBaseInfo().database = syndatabase;
                Wt::WLineEdit * laux = (*f).ppd->GetSdiDialog().columnname;
                laux->setText(Wt::WString(syncolname));
                laux = (*f).ppd->GetSdiDialog().tablename;
                laux->setText(Wt::WString(syntablename));
                laux = (*f).ppd->GetSdiDialog().whereclause;
                laux->setText(Wt::WString(synwhereclause));
                std::vector<std::string> splits;
                boost::split(splits, syndatabase, boost::is_any_of(" "));
                for (std::vector<std::string>::iterator it = splits.begin();
                        it != splits.end(); it++) {
                    if ((*it).find("dbname=") != std::string::npos) {
                        std::vector<std::string> splits2;
                        boost::split(splits2, *it, boost::is_any_of("="));
                        laux = (*f).ppd->GetSdiDialog().dbname;
                        laux->setText(Wt::WString(splits2[1]));
                    }
                    if ((*it).find("host=") != std::string::npos) {
                        std::vector<std::string> splits2;
                        boost::split(splits2, *it, boost::is_any_of("="));
                        laux = (*f).ppd->GetSdiDialog().hostname;
                        laux->setText(Wt::WString(splits2[1]));
                    }
                    if ((*it).find("port=") != std::string::npos) {
                        std::vector<std::string> splits2;
                        boost::split(splits2, *it, boost::is_any_of("="));
                        laux = (*f).ppd->GetSdiDialog().port;
                        laux->setText(Wt::WString(splits2[1]));
                    }
                    if ((*it).find("user=") != std::string::npos) {
                        std::vector<std::string> splits2;
                        boost::split(splits2, *it, boost::is_any_of("="));
                        laux = (*f).ppd->GetSdiDialog().username;
                        laux->setText(Wt::WString(splits2[1]));
                    }
                    if ((*it).find("password=") != std::string::npos) {
                        std::vector<std::string> splits2;
                        boost::split(splits2, *it, boost::is_any_of("="));
                        laux = (*f).ppd->GetSdiDialog().password;
                        laux->setText(Wt::WString(splits2[1]));
                    }
                }
                delete ppp;
            }
        }
        grid_->addWidget(wc, x, y);
        AddGreenPlus(x + 1, y);
        AddGreenPlus(x, y + 1);
    }
    fieldinfolistloaded_.clear();
    for (int i = 0; i < fieldinfolist_.size(); i++) {
        fieldinfo * f = new fieldinfo;
        (*f) = (*fieldinfolist_[i]);
        fieldinfolistloaded_.push_back(f);
    }
    if (cfic.NumberOfFields() == 0) AddGreenPlus(0, 0);
}

void CreateCurationForm::UpdateShareWithText() {
    if (sharedwith_.size() > 0) {
        std::vector<std::string>::iterator it, it2(sharedwith_.begin());
        std::string s(*it2);
        for (it = ++it2; it != sharedwith_.end(); it++) s += "," + *it;
        sharewithtext_->setText(Wt::WString(s));
    } else {

        sharewithtext_->setText("");
    }
}

void CreateCurationForm::PosturlEditEnterPressed() {
    urleditisaddress_ = (posturlmodedropdown_->currentIndex() == 0);
    posturllabel_->setText((urleditisaddress_) ? "Address: " : "Parameter name: ");
    posturltext_->setText(posturledit_->text());
    posturlmodedropdown_->setCurrentIndex(0);
    posturledit_->setText("");
    ValidateAllFields();
}

void CreateCurationForm::ParameterFormatChanged(Wt::WButtonGroup * bg) {
    paramformattext_->setText((bg->checkedButton()->text()));
}

void CreateCurationForm::FormnameEditEnterPressed() {
    formnametext_->setText(formnameedit_->text());
    formnametext_->decorationStyle().setForegroundColor(Wt::black);
    sharedwith_.clear();
    std::string username(session_->login().user().identity("loginname").toUTF8());
    bool foundasown = false;
    for (long unsigned int i = 0; i < cfc_->GetTableSize(); i++)
        if (username.compare(cfc_->GetOwner(i)) == 0)
            if (formnametext_->text().toUTF8().compare(cfc_->GetFormName(i)) == 0) {
                std::string poaux(cfc_->GetPostUrl(i));
                urleditisaddress_ = true;
                if (poaux.length() > 1)
                    if (poaux.find("$") == 0)
                        if (poaux.rfind("$") == poaux.length() - 1) {
                            urleditisaddress_ = false;
                            poaux = poaux.substr(1, poaux.length() - 2);
                        }
                posturllabel_->setText((urleditisaddress_) ? "Address: " : "Parameter name: ");
                posturltext_->setText(Wt::WString(poaux));
                paramformattext_->setText(cfc_->GetFormatName(cfc_->GetParameterFormat(i)));
                std::string saux(cfc_->GetPrivileges(i));
                std::vector<std::string> splits;
                boost::split(splits, saux, boost::is_any_of(","));
                while (!splits.empty()) {
                    sharedwith_.push_back(splits.back());
                    splits.pop_back();
                }
                foundasown = true;
                continue;
            }
    if (!foundasown) {
        if (cfc_->FoundFormName(formnametext_->text().toUTF8())) {
            formnametext_->setText("(Name already in use. Please try again.)");
            formnametext_->decorationStyle().setForegroundColor(Wt::red);
        } else {
            formnameedit_->setText("");
        }
    } else
        LoadFields(formnametext_->text().toUTF8());
    UpdateShareWithText();
    if (formnametext_->text().empty()) {
        formnameedit_->setEmptyText("(required!)");
    } else {

        formnameedit_->setEmptyText("");
    }
    ValidateAllFields();
}

void CreateCurationForm::ShareWithEditEnterPressed() {
    if (sharewithedit_->text().empty())
        sharedwith_.clear();
    else
        sharedwith_.push_back(sharewithedit_->text().toUTF8());
    sharewithedit_->setText("");
    UpdateShareWithText();
    if (sharewithtext_->text().empty()) {
        sharewithlabel_->hide();
        sharewithtext_->hide();
    } else {

        sharewithlabel_->show();
        sharewithtext_->show();
    }
}

void CreateCurationForm::SetCursorHand(Wt::WWidget * w) {

    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}

bool CreateCurationForm::FieldInfoIsNotSame(fieldinfo * f1, fieldinfo * f2) {
    if ((*f1).x != (*f2).x) return true;
    if ((*f1).y != (*f2).y) return true;
    std::string fn1((*f1).fieldnameedit->text().toUTF8());
    std::string fn2((*f2).fieldnameedit->text().toUTF8());
    if (fn1.compare(fn2) != 0) return true;
    int i1 = (*f1).ftbgroup->checkedId();
    int i2 = (*f2).ftbgroup->checkedId();
    if (i1 != i2) return true;
    if ((*f1).ftbgroup->checkedId() == 2) {
        i1 = (*f1).checkboxdefaultbgroup->checkedId();
        i2 = (*f2).checkboxdefaultbgroup->checkedId();

        if (i1 != i2) return true;
    }
    return false;
}

void CreateCurationForm::SetSaveFlag(bool b) {
    saveflag_ = b;
    if (saveflag_)
        savebutton_->enable();

    else
        savebutton_->disable();
}

bool CreateCurationForm::ValidateAllFields() {
    bool ret(true);
    if (fieldinfolist_.size() > 0) {
        std::vector<fieldinfo*>::iterator it;
        for (it = fieldinfolist_.begin(); it != fieldinfolist_.end(); it++)
            ret = (FieldInfoValidate(*it) && ret);
    } else
        ret = false;
    SetSaveFlag(ret);

    return ret;
}

CreateCurationForm::~CreateCurationForm() {
}
