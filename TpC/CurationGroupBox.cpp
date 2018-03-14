/* 
 * File:   CurationGroupBox.cpp
 * Author: mueller
 * 
 * Created on March 7, 2014, 11:05 AM
 */

#include "CurationGroupBox.h"
#include "CurationFormsConfiguration.h"
#include "TCNavWeb.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include <Wt/WBreak>
#include <Wt/WFont>
#include <Wt/WIconPair>
#include <Wt/WStackedWidget>
#include <map>
#include <boost/algorithm/string.hpp>

namespace {

    void DeleteAll(std::map<Wt::WText*, Wt::WImage*> & im,
            std::map<Wt::WText*, Wt::WText*> & ew,
            std::vector<Wt::WText*> & los,
            std::map<Wt::WText*, Wt::WText*> & ss,
            std::map<Wt::WText*, Wt::WText*> & se) {
        // im contains the start-image mappings,
        // ew contains the start-end mappings,
        // los contains all starts
        // ss contains the start-snipletstart mappings (destroy maps only)
        // se contains the start-snipletend mappings (destroy maps only)
        std::map<Wt::WText*, Wt::WText*>::iterator it;
        for (it = ew.begin(); it != ew.end(); it++) {
            delete (*it).second;
        }
        ew.clear();
        std::map<Wt::WText*, Wt::WImage*>::iterator it2;
        for (it2 = im.begin(); it2 != im.end(); it2++) {
            delete (*it2).second;
        }
        im.clear();
        ss.clear();
        se.clear();
        std::vector<Wt::WText*>::iterator it3;
        for (it3 = los.begin(); it3 != los.end(); it3++)
            delete(*it3);
        los.clear();
    }

    void RemoveAllChildren(Wt::WContainerWidget * c) {
        std::vector<Wt::WWidget*> list = c->children();
        std::vector<Wt::WWidget*>::iterator it;
        for (it = list.begin(); it != list.end(); it++)
            c->removeWidget(*it);
    }
}

// Basic operations

CurationGroupBox::CurationGroupBox(Session * session, Wt::WString title,
        Wt::WContainerWidget * parent) : Wt::WGroupBox(parent) {
    session_ = session;
    session_->login().changed().connect(this, &CurationGroupBox::SetLoginState);
    setTitle(title);
    InitializeVars();
    CreateContents();
    UpdateContents();
    TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent);
    tcnw->GetCustomizationInstance()->GetCreateCurationFormInstance()
            ->FormSavedSignal().connect(this, &CurationGroupBox::PopulateFormComboBox);
}

void CurationGroupBox::InitializeVars() {
    loggedinLoggedoutState_ = 0;
    cancelAllSnipletsState_ = false;
    container_ = NULL;
    loggedinLoggedoutStack_ = NULL;
    loggedoutExplanation_ = NULL;
    loggedinContainer_ = NULL;
    collectionStatusIconPair_ = NULL;
    collectionStatusLabel_ = NULL;
    cancelAllSnipletsImage_ = NULL;
    cancelAllSnipletsText_ = NULL;
    listOfSnipletStarts_.clear();
    snipletStarts_.clear();
    snipletEnds_.clear();
    snipletCancelImage_.clear();
    lastStart_ = NULL;
    snipletsContainer_ = NULL;
}

void CurationGroupBox::CreateContents() {
    container_ = new Wt::WContainerWidget();
    Wt::WLength x(Wt::WLength(24, Wt::WLength::FontEx));
    Wt::WLength y(Wt::WLength(36, Wt::WLength::FontEx));
    container_->setWidth(x);
    //container_->setHeight(y);
    addWidget(container_);
    //
    loggedinLoggedoutStack_ = new Wt::WStackedWidget();
    loggedoutExplanation_ = new Wt::WText("You need to be logged in to use this feature.");
    loggedoutExplanation_->decorationStyle().setForegroundColor(Wt::darkRed);
    loggedoutExplanation_->decorationStyle().font().setSize(Wt::WFont::Small);
    loggedinContainer_ = new Wt::WContainerWidget();
    loggedinLoggedoutStack_->insertWidget(0, loggedoutExplanation_);
    loggedinLoggedoutStack_->insertWidget(1, loggedinContainer_);
    container_->addWidget(loggedinLoggedoutStack_);
    // How to use viewer help text
    howToUseViewerExplanation_ = new Wt::WText;
    Wt::WString aux = "Click on the first, then on the last word of a sentence "
            "or paragraph to select it for curation. Repeat until selection "
            " process is finished, then select the form you want to use "
            " and hit the 'Use form in curation panel!' button below.";
    howToUseViewerExplanation_->setText(aux);
    howToUseViewerExplanation_->decorationStyle().setForegroundColor(Wt::darkGreen);
    howToUseViewerExplanation_->decorationStyle().font().setSize(Wt::WFont::Small);
    loggedinContainer_->addWidget(howToUseViewerExplanation_);
    loggedinContainer_->addWidget(new Wt::WBreak());
    //
    collectionStatusIconPair_ = new Wt::WIconPair("resources/icons/pencil_weak.png",
            "resources/icons/pencil_add.png", false);
    collectionStatusIconPair_->setToolTip("Collecting text indicator.");
    collectionStatusIconPair_->setInline(true);
    collectionStatusLabel_ = new Wt::WText("Collection status: ");
    collectionStatusLabel_->setInline(true);
    loggedinContainer_->addWidget(collectionStatusLabel_);
    loggedinContainer_->addWidget(collectionStatusIconPair_);
    loggedinContainer_->addWidget(new Wt::WBreak());
    //
    cancelAllSnipletsImage_ = new Wt::WImage("resources/icons/cancel.png");
    cancelAllSnipletsImage_->setInline(true);
    cancelAllSnipletsImage_->clicked().connect(boost::bind(&CurationGroupBox::CancelAllSnipletsClicked, this, false));
    cancelAllSnipletsImage_->mouseWentOver().connect(boost::bind(&CurationGroupBox::SetCursorHand, this, cancelAllSnipletsImage_));
    cancelAllSnipletsText_ = new Wt::WText(" Delete all entries.");
    cancelAllSnipletsText_->setInline(true);
    cancelAllSnipletsText_->clicked().connect(boost::bind(&CurationGroupBox::CancelAllSnipletsClicked, this, true));
    cancelAllSnipletsText_->mouseWentOver().connect(boost::bind(&CurationGroupBox::SetCursorHand, this, cancelAllSnipletsText_));
    cancelAllSnipletsText_->decorationStyle().setForegroundColor(Wt::darkRed);
    loggedinContainer_->addWidget(cancelAllSnipletsImage_);
    loggedinContainer_->addWidget(cancelAllSnipletsText_);
    loggedinContainer_->addWidget(new Wt::WBreak);
    //
    snipletsContainer_ = new Wt::WContainerWidget();
    loggedinContainer_->addWidget(snipletsContainer_);
    //
    formnameComboBox_ = new Wt::WComboBox();
    formnameComboBox_->setWidth(x);
    PopulateFormComboBox();
    curateButton_ = new Wt::WPushButton("Use form in curation panel!");
    curateButton_->setWidth(x);
    curateButton_->setToolTip("Open panel for annotation.");
    curateButton_->setInline(true);
    loggedinContainer_->addWidget(new Wt::WBreak());
    loggedinContainer_->addWidget(new Wt::WText("Select Form:"));
    loggedinContainer_->addWidget(formnameComboBox_);
    loggedinContainer_->addWidget(curateButton_);
}

void CurationGroupBox::UpdateContents() {
    loggedinLoggedoutStack_->setCurrentIndex(loggedinLoggedoutState_);
    if (lastStart_ != NULL)
        collectionStatusIconPair_->setState(1);
    else
        collectionStatusIconPair_->setState(0);
    if (cancelAllSnipletsState_)
        ShowCancelAllSniplets();
    else
        HideCancelAllSniplets();
    RemoveAllChildren(snipletsContainer_);
    std::vector<Wt::WText*>::iterator it;
    for (it = listOfSnipletStarts_.begin(); it != listOfSnipletStarts_.end(); it++) {
        std::map<Wt::WText*, Wt::WImage*>::iterator im;
        im = snipletCancelImage_.find(*it);
        if (im != snipletCancelImage_.end()) {
            (*im).second->setInline(true);
            snipletsContainer_->addWidget((*im).second);
        }
        (*it)->setInline(true);
        snipletsContainer_->addWidget(*it);
        //
        std::map<Wt::WText*, Wt::WText*>::iterator it2;
        it2 = endWtext_.find(*it);
        //
        if (it2 != endWtext_.end()) {
            (*it2).second->setInline(true);
            snipletsContainer_->addWidget((*it2).second);
        }
        snipletsContainer_->addWidget(new Wt::WBreak());
    }
}

void CurationGroupBox::PopulateFormComboBox() {
    formnameComboBox_->clear();
    Wt::WString username("");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname");
    if (!username.empty()) {
        std::set<Wt::WString> names; // for sorting
        CurationFormsConfiguration cfc(PGCURATIONFORMS, PGCURATIONFORMSTABLENAME);
        for (long unsigned int i = 0; i < cfc.GetTableSize(); i++) {
            // still have to take into account privileges
            std::string privileges(cfc.GetPrivileges(i));
            std::vector<std::string> splits;
            boost::split(splits, privileges, boost::is_any_of(","));
            bool hasprivs(std::find(splits.begin(), splits.end(), username) != splits.end());
            //
            if (username.toUTF8().compare(cfc.GetOwner(i)) == 0 || hasprivs)
                names.insert(Wt::WString(cfc.GetFormName(i)));
        }
        for (std::set<Wt::WString>::iterator it = names.begin(); it != names.end(); it++)
            formnameComboBox_->addItem(*it);
        if (formnameComboBox_->count() == 0) {
            formnameComboBox_->disable();
        } else {
            formnameComboBox_->enable();
        }
    }
}

CurationGroupBox::~CurationGroupBox() {
    DeleteAll(snipletCancelImage_, endWtext_, listOfSnipletStarts_, snipletStarts_, snipletEnds_);
    delete loggedoutExplanation_;
    delete collectionStatusIconPair_;
    delete collectionStatusLabel_;
    delete loggedinContainer_;
    delete loggedinLoggedoutStack_;
    delete container_;
}

void CurationGroupBox::SetLoginState() {
    loggedinLoggedoutState_ = (session_->login().state() != 0);
    UpdateContents();
    PopulateFormComboBox();
}

// Sniplet related operations

void CurationGroupBox::SubmitNewWtextEntry(Wt::WText* t) {
    if (!howToUseViewerExplanation_->text().empty()) howToUseViewerExplanation_->setText("");
    if (lastStart_ == NULL) {
        Wt::WText * start = new Wt::WText(" " + t->text());
        listOfSnipletStarts_.push_back(start);
        snipletStarts_.insert(std::make_pair(start, t));
        start->setToolTip("Scroll into view.");
        start->clicked().connect(boost::bind(&CurationGroupBox::SendScrollIntoViewSignal, this, t));
        start->mouseWentOver().connect(boost::bind(&CurationGroupBox::SetCursorHand, this, start));
        Wt::WImage * cancelsnipletimage = new Wt::WImage("resources/icons/pencil_delete.png");
        std::map<Wt::WText*, Wt::WImage*>::iterator itim;
        for (itim = snipletCancelImage_.begin(); itim != snipletCancelImage_.end(); itim++) {
            (*itim).second->setImageRef("resources/icons/pencil_weak.png");
            (*itim).second->setDisabled(true);
        }
        snipletCancelImage_.insert(std::make_pair(start, cancelsnipletimage));
        cancelsnipletimage->setToolTip("Remove selected text.");
        cancelsnipletimage->clicked().connect(boost::bind(&CurationGroupBox::RemoveCurationEntry, this, start));
        cancelsnipletimage->mouseWentOver().connect(boost::bind(&CurationGroupBox::SetCursorHand, this, cancelsnipletimage));
        lastStart_ = start;
    } else {
        Wt::WText * end = new Wt::WText();
        snipletEnds_.insert(std::make_pair(lastStart_, t));
        endWtext_.insert(std::make_pair(lastStart_, end));
        end->setToolTip("Scroll into view.");
        end->setText(" ... " + t->text());
        end->clicked().connect(boost::bind(&CurationGroupBox::SendScrollIntoViewSignal, this, t));
        end->mouseWentOver().connect(boost::bind(&CurationGroupBox::SetCursorHand, this, end));
        std::map<Wt::WText*, Wt::WImage*>::iterator itim;
        for (itim = snipletCancelImage_.begin(); itim != snipletCancelImage_.end(); itim++) {
            (*itim).second->setImageRef("resources/icons/pencil_delete.png");
            (*itim).second->setDisabled(false);
        }
        lastStart_ = NULL;
    }
    if (listOfSnipletStarts_.size() > 1) cancelAllSnipletsState_ = true;
    UpdateContents();
}

void CurationGroupBox::RemoveCurationEntry(Wt::WText * start) {
    Wt::WText * snipletStart = NULL;
    std::map<Wt::WText*, Wt::WText*>::iterator it;
    it = snipletStarts_.find(start);
    if (it != snipletStarts_.end()) {
        snipletStart = (*it).second;
        snipletStarts_.erase(it);
    }
    Wt::WText * snipletEnd = NULL;
    it = snipletEnds_.find(start);
    if (it != snipletEnds_.end()) {
        snipletEnd = (*it).second;
        snipletEnds_.erase(it);
    }
    cancelSingleSnipletSignal_.emit(snipletStart, snipletEnd);
    std::map<Wt::WText*, Wt::WImage*>::iterator it2 = snipletCancelImage_.find(start);
    if (it2 != snipletCancelImage_.end()) {
        delete (*it2).second;
        snipletCancelImage_.erase(it2);
    }
    it = endWtext_.find(start);
    if (it != endWtext_.end()) {
        delete(*it).second;
        endWtext_.erase(it);
    }
    std::vector<Wt::WText*>::iterator it3;
    it3 = std::find(listOfSnipletStarts_.begin(), listOfSnipletStarts_.end(), start);
    if (it3 != listOfSnipletStarts_.end())
        listOfSnipletStarts_.erase(it3);
    delete start;
    lastStart_ = NULL;
    if (listOfSnipletStarts_.size() < 2) cancelAllSnipletsState_ = false;
    UpdateContents();
}

void CurationGroupBox::CancelAllSnipletsClicked(bool imageortext) {
    // image: imageortext = false
    // text: imageortext = true
    cancelAllSnipletsSignal_.emit(imageortext);
    CancelAllSniplets();
}

void CurationGroupBox::CancelAllSniplets() {
    cancelAllSnipletsState_ = false;
    // now delete all mappings, WTexts and WImages in this interface;
    DeleteAll(snipletCancelImage_, endWtext_, listOfSnipletStarts_, snipletStarts_, snipletEnds_);
    lastStart_ = NULL;
    UpdateContents();
}

void CurationGroupBox::SendScrollIntoViewSignal(Wt::WText * t) {
    scrollIntoViewSignal_.emit(t);
}
