/* 
 * File:   PickedLiteratureContents.cpp
 * Author: mueller
 * 
 * Created on April 15, 2015, 3:09 PM
 */

#include "PickedLiteratureContents.h"
#include "Preference.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include <Wt/WBreak>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <Wt/WCssDecorationStyle>
#include <IndexManager.h>

PickedLiteratureContents::PickedLiteratureContents(Session * session,
        std::map<std::string, bool> & literatures,
        Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    session_ = session;
    session_->login().changed().connect(boost::bind(&PickedLiteratureContents::SessionLoginChanged, this));
    //
    gridcont_ = new Wt::WContainerWidget();
    grid_ = NULL;
    setMinimumSize(Wt::WLength(15 * squaresize, Wt::WLength::FontEx), Wt::WLength::Auto);
    PopulateGrid(literatures, squaresize);
    //
    checkall_ = new Wt::WCheckBox("Check/Uncheck all literatures");
    checkall_->changed().connect(std::bind([ = ] (){
        for (auto& lit : pickedliterature_) {
            lit.second = checkall_->isChecked();
            if (cbxes_.find(lit.first) != cbxes_.end())
                    cbxes_[lit.first]->setChecked(checkall_->isChecked());
        }
    }));
    ok_ = new Wt::WPushButton("Ok");
    Wt::WString savetext("Save Preferences");
    if (session_->login().state() == 0) savetext += " (disabled -- not logged in)";
    savebox_ = new Wt::WCheckBox(savetext);
    savebox_->setChecked(false);
    if (session_->login().state() != 0)
        savebox_->enable();
    else
        savebox_->disable();
    //
    Wt::WPushButton * reload = new Wt::WPushButton("Reload Saved Configuration");
    reload->clicked().connect(this, &PickedLiteratureContents::ReloadPressed);
    addWidget(savebox_);
    addWidget(new Wt::WBreak());
    addWidget(ok_);
    ok_->setInline(true);
    addWidget(reload);
    reload->setInline(true);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(checkall_);
    addWidget(new Wt::WBreak());
    addWidget(gridcont_);
}

void PickedLiteratureContents::PopulateGrid(std::map < std::string, bool> & literatures,
        int squaresize) {
    if (grid_ != NULL) {
        delete grid_;
        gridcont_->clear();
    }
    if (literatures.empty()) {
        grid_ = NULL;
        Wt::WText * helptext = new Wt::WText();
        helptext->decorationStyle().setForegroundColor(Wt::red);
        if (session_->login().state() != 0) {
            helptext->setText("No literature is available because "
                    "permissions have not been set. "
                    "Contact Textpresso at textpresso@caltech.edu."
                    );
        } else {
            helptext->setText("No literature is available because you are not logged in.");

        }
        helptext->setWordWrap(true);
        gridcont_->addWidget(new Wt::WBreak());
        gridcont_->addWidget(helptext);
    } else {
        grid_ = new Wt::WGridLayout();
        gridcont_->setLayout(grid_);
        cbxes_.clear();
        int x(0), y(0), count(0);
        std::map < std::string, bool>::iterator it;
        for (it = literatures.begin(); it != literatures.end(); it++) {
            x = count % squaresize;
            y = count / squaresize;
            std::string newtext = "";
            boost::regex re("\\_\\d+$");
            std::string base = boost::regex_replace((*it).first, re, newtext);
            if (cbxes_.find(base) == cbxes_.end()) {
                cbxes_[base] = new Wt::WCheckBox(base);
                grid_->addWidget(cbxes_[base], y, x);
                count++;
            }
            cbxes_[base]->setChecked((*it).second);
            cbxes_[base]->changed().connect(boost::bind(&PickedLiteratureContents::ChangedCheckBox,
                    this, cbxes_[base], (*it).first));
            pickedliterature_[(*it).first] = (*it).second;
        }
    }
}

void PickedLiteratureContents::ChangedCheckBox(Wt::WCheckBox * cb, std::string s) {
    pickedliterature_[s] = cb->isChecked();
}

void PickedLiteratureContents::EnableSaveBox(bool b) {
    if (b)
        savebox_->enable();
    else
        savebox_->disable();
}

void PickedLiteratureContents::ShowSaveBox(bool b) {
    if (b)
        savebox_->show();
    else
        savebox_->hide();
}

void PickedLiteratureContents::ReloadPressed() {
    ReadLitPref();
    PopulateGrid(pickedliterature_, squaresize);
}

void PickedLiteratureContents::UpdateLiteraturePreferences(bool checkpermissions) {
    std::string username("default");
    if (session_->login().state() != 0) {
        username = session_->login().user().identity("loginname").toUTF8();
    }
    Preference * pref = new Preference(PGLITERATURE, PGLITPREFTABLENAME, username);
    Preference * permissions = new Preference(PGLITERATUREPERMISSION,
                                              PGLITERATUREPERMISSIONTABLENAME, username);
    Preference * dfpermissions = new Preference(PGLITERATUREPERMISSION,
                                                PGLITERATUREPERMISSIONTABLENAME, "default");
    for (const std::string& corpus : tpc::index::IndexManager::get_available_corpora(tpc::index::CAS_ROOT_LOCATION.c_str())) {
        // grant search rights based on individual and default permissions.
        if ((!checkpermissions || permissions->IsPreference(corpus)) || dfpermissions->IsPreference(corpus))
            if (pref->HasPreferences())
                pickedliterature_[corpus] = (pref->IsPreference(corpus)) ? true : false;

            else
                pickedliterature_[corpus] = true;
    }
    delete pref;
    delete permissions;
    delete dfpermissions;
}

void PickedLiteratureContents::ReadLitPref() {
    pickedliterature_.clear();
    UpdateLiteraturePreferences(true);
}

void PickedLiteratureContents::SessionLoginChanged() {
    ReloadPressed();
}

PickedLiteratureContents::~PickedLiteratureContents() {
}
