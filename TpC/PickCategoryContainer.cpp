/* 
 * File:   PickCategoryContainer.cpp
 * Author: mueller
 * 
 * Created on July 16, 2017, 9:32 AM
 */


#include "PickCategoryContainer.h"
#include "Preference.h"

#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <Wt/WSuggestionPopup>
#include <Wt/WGroupBox>
#include <Wt/WRadioButton>
#include <Wt/WHBoxLayout>
#include <Wt/WScrollArea>
#include <Wt/WCssDecorationStyle>
#include <Wt/WTimer>
#include <Wt/WLabel>

PickCategoryContainer::PickCategoryContainer(Session * session,
        std::set<std::string> & preloaded, bool showmatchcriteriachoice) {
    showmatchcriteriachoice_ = showmatchcriteriachoice;
    preloaded_ = preloaded;
    //
    Wt::WHBoxLayout * boxlayout = new Wt::WHBoxLayout();
    setLayout(boxlayout);
    //
    Wt::WPushButton * changeprefs = new Wt::WPushButton("Change tree preferences");
    changeprefs->clicked().connect(boost::bind(&PickCategoryContainer::ChangePreferences, this,
            session));
    changeprefs->setStyleClass("btn-mini");
    changeprefs->setFloatSide(Wt::Right);
    //
    includechildren_ = new Wt::WCheckBox("include children of selected categories");
    includechildren_->setFloatSide(Wt::Left);
    includechildren_->setChecked();
    includechildren_->changed().connect(this, &PickCategoryContainer::DisplaySelectedCategories);
    //
    selectedcatdisplay_ = new Wt::WContainerWidget();
    selectedcatdisplay_->setMaximumSize(Wt::WLength(48, Wt::WLength::FontEx),
            Wt::WLength(64, Wt::WLength::FontEx));
    Wt::WContainerWidget * inputdisplay = new Wt::WContainerWidget();
    inputdisplay->setMinimumSize(Wt::WLength(80, Wt::WLength::FontEx), // max width
            Wt::WLength(48, Wt::WLength::FontEx)); // max height
    //
    tcb_ = new TpCategoryBrowser(preloaded_);
    tcb_->itemSelectionChanged().connect(this, &PickCategoryContainer::ItemSelectionChanged);
    //
    searchbox_ = new Wt::WLineEdit();
    searchbox_->setEmptyText("Type in category");
    searchbox_->resize(Wt::WLength(64, Wt::WLength::FontEx), Wt::WLength(2, Wt::WLength::FontEx));
    searchbox_->enterPressed().connect(this, &PickCategoryContainer::EnterPressed);
    //
    Wt::WSuggestionPopup::Options popupOption;
    popupOption.highlightBeginTag = "<span class=\"highlight\">";
    popupOption.highlightEndTag = "</span>";
    popupOption.wordSeparators = "-., \";";
    Wt::WSuggestionPopup * sp = new Wt::WSuggestionPopup(
            Wt::WSuggestionPopup::generateMatcherJS(popupOption),
            Wt::WSuggestionPopup::generateReplacerJS(popupOption));
    // Populate the underlying model with suggestions:
    std::set<std::string> aux = tcb_->GetCategorySet();
    std::set<std::string>::iterator it;
    for (it = aux.begin(); it != aux.end(); it++) {
        catsetfrompg_.insert(*it);
        sp->addSuggestion(*it);
    }
    inputdisplay->addWidget(includechildren_);
    inputdisplay->addWidget(new Wt::WBreak());
    inputdisplay->addWidget(new Wt::WBreak());
    sp->forEdit(searchbox_);
    sp->setMaximumSize(Wt::WLength(60, Wt::WLength::FontEx), Wt::WLength(30, Wt::WLength::FontEx));
    sp->setFilterLength(4);
    inputdisplay->addWidget(searchbox_);
    searchbox_->setInline(true);
    Wt::WPushButton * addbutton = new Wt::WPushButton("Add!");
    addbutton->setStyleClass("btn-mini");
    addbutton->setVerticalAlignment(Wt::AlignTop);
    addbutton->clicked().connect(this, &PickCategoryContainer::EnterPressed);
    addbutton->setInline(true);
    inputdisplay->addWidget(addbutton);
    inputdisplay->addWidget(new Wt::WBreak());
    inputdisplay->addWidget(new Wt::WText("or pick from tree below:"));
    inputdisplay->addWidget(new Wt::WBreak());
    inputdisplay->addWidget(changeprefs);
    inputdisplay->addWidget(new Wt::WBreak());
    inputdisplay->addWidget(tcb_);
    inputdisplay->addWidget(new Wt::WBreak());
    Wt::WScrollArea * cs = new Wt::WScrollArea();
    cs->setScrollBarPolicy(Wt::WScrollArea::ScrollBarAsNeeded);
    Wt::WLength heightfx = Wt::WLength(36, Wt::WLength::FontEx);
    inputdisplay->setHeight(heightfx);
    cs->setWidget(inputdisplay);
    boxlayout->addWidget(selectedcatdisplay_);
    boxlayout->addWidget(cs);
    //
}

void PickCategoryContainer::EnterPressed() {
    if (!searchbox_->text().empty()) {
        if (catsetfrompg_.find((searchbox_->text()).toUTF8()) != catsetfrompg_.end()) {
            catsfromlineedit_.insert(searchbox_->text());
            searchbox_->setText("");
            DisplaySelectedCategories();
        } else {
            Wt::WText * warning = new Wt::WText("This category does not exist.");
            warning->decorationStyle().setBackgroundColor(Wt::yellow);
            selectedcatdisplay_->clear();
            selectedcatdisplay_->addWidget(new Wt::WBreak());
            selectedcatdisplay_->addWidget(warning);
            selectedcatdisplay_->addWidget(new Wt::WBreak());
            Wt::WTimer * timer = new Wt::WTimer();
            timer->setInterval(2000);
            timer->setSingleShot(true);
            timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
                timer->stop();
                delete timer;
                DisplaySelectedCategories();
            }, timer));
            timer->start();
        }
    }
}

void PickCategoryContainer::ItemSelectionChanged() {
    DisplaySelectedCategories();
}

void PickCategoryContainer::ResetSelectedCategories() {
    catsfromlineedit_.clear();
    searchbox_->setText("");
    tcb_->clearSelection();
    DisplaySelectedCategories();
}

void PickCategoryContainer::ChangePreferences(Session * session) {
    session_ = session;
    cp_ = new CategoryPreferences(session_, preloaded_);
    cp_->finished().connect(this, &PickCategoryContainer::CategoryPreferencesDialogDone);
    cp_->show();
}

void PickCategoryContainer::CategoryPreferencesDialogDone(Wt::WDialog::DialogCode code) {
    if (code == Wt::WDialog::Accepted) {
        preloaded_.clear();
        TpCategoryBrowser * tcp = new TpCategoryBrowser();
        std::set<std::string> plist = tcp->GetAllDirectChildrensName("root");
        delete tcp;
        std::set<std::string>::iterator it;
        for (it = plist.begin(); it != plist.end(); it++)
            if (cp_->GetCheckState(*it)) preloaded_.insert(*it);
        tcb_->DeleteRootChildren();
        tcb_->SetUpRootChildren(preloaded_);
    }
    if (cp_->SaveBoxIsChecked()) {
        if (session_->login().state() != 0) {
            std::string username = session_->login().user().identity("loginname").toUTF8();
            Preference * pref = new Preference(PGPRELOADEDCATEGORIES, PGPRELOADEDCATTABLENAME, username);
            pref->SavePreferences(username, preloaded_);
            delete pref;
        }
    }
    delete cp_;
}

std::set<Wt::WString> PickCategoryContainer::GetSelected(bool explct) {
    std::set<Wt::WString> aux(tcb_->GetSelected());
    std::set<Wt::WString>::iterator it;
    for (it = catsfromlineedit_.begin(); it != catsfromlineedit_.end(); it++)
        aux.insert(*it);
    std::set<Wt::WString> ret;
    for (it = aux.begin(); it != aux.end(); it++)
        if (includechildren_->isChecked())
            if (explct) {
                ret.insert(*it);
                TpCategoryBrowser * tcp = new TpCategoryBrowser();
                std::set<std::string> plist = tcp->GetAllChildrensName((*it).toUTF8());
                std::set<std::string>::iterator itc;
                for (itc = plist.begin(); itc != plist.end(); itc++)
                    ret.insert(Wt::WString(*itc));
                delete tcp;
            } else
                ret.insert("pAaCh" + (*it));
        else
            ret.insert(*it);
    return ret;
}

void PickCategoryContainer::DisplaySelectedCategories() {
    selectedcatdisplay_->clear();
    std::set<Wt::WString> list(GetSelected());
    if (!list.empty()) {
        setWidth(Wt::WLength(100, Wt::WLength::FontEx));
        selectedcatdisplay_->setWidth(Wt::WLength(30, Wt::WLength::FontEx));
        Wt::WGroupBox * groupBox = new Wt::WGroupBox("Selected Categories");
        for (std::set<Wt::WString>::iterator it = list.begin();
                it != list.end(); it++) {
            std::string aux = (*it).toUTF8();
            if (aux.find("pAaCh") != std::string::npos) {
                boost::replace_first(aux, "pAaCh", "");
                int numChildren = tcb_->getTotalNumSubCategories(aux);
                aux += " (incl. " + std::to_string(numChildren) + " children)";
            }
            groupBox->addWidget(new Wt::WText(aux));
            groupBox->addWidget(new Wt::WBreak());
        }
        selectedcatdisplay_->addWidget(groupBox);
        selectedcatdisplay_->addWidget(new Wt::WBreak());
        groupAndorOr_ = new Wt::WButtonGroup();
        Wt::WRadioButton * buttonOr = new Wt::WRadioButton("At least one category");
        groupAndorOr_->addButton(buttonOr);
        Wt::WRadioButton * buttonAnd = new Wt::WRadioButton("All categories");
        groupAndorOr_->addButton(buttonAnd);
        groupAndorOr_->setSelectedButtonIndex(1); // Select the first button by default.
        if (list.size() > 1 && showmatchcriteriachoice_) {
            selectedcatdisplay_->addWidget(new Wt::WText("Match criteria for categories in query: "));
            selectedcatdisplay_->addWidget(new Wt::WBreak());
            selectedcatdisplay_->addWidget(buttonOr);
            selectedcatdisplay_->addWidget(new Wt::WBreak());
            selectedcatdisplay_->addWidget(buttonAnd);
            selectedcatdisplay_->addWidget(new Wt::WBreak());
        }
        selectedcatdisplay_->addWidget(new Wt::WBreak());
        Wt::WPushButton * oktop = new Wt::WPushButton("Ok");
        oktop->clicked().connect(std::bind([ = ] (){
            okclicked_.emit();
        }));
        Wt::WPushButton * resetbutton = new Wt::WPushButton("Reset!");
        oktop->setInline(true);
        resetbutton->clicked().connect(this, &PickCategoryContainer::ResetSelectedCategories);
        resetbutton->setInline(true);
        selectedcatdisplay_->addWidget(oktop);
        selectedcatdisplay_->addWidget(resetbutton);
    }
    resize(Wt::WLength::Auto, Wt::WLength::Auto);
}

PickCategoryContainer::~PickCategoryContainer() {
}
