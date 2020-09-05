/* 
 * File:   Customization.cpp
 * Author: mueller
 * 
 * Created on April 15, 2015, 1:48 PM
 */

#include "Customization.h"
#include "Preference.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include "CustomizeColors.h"
#include <Wt/WTabWidget>
#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WCssDecorationStyle>
#include <Wt/WTimer>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <Wt/WImage>
#include <Wt/WTable>
#include <Wt/WBreak>

Customization::Customization(Wt::WContainerWidget * parent) {
    alreadyloaded_ = false;
}

void Customization::LoadContent(Session & session) {
    if (!alreadyloaded_) {
        session_ = &session;
        session_->login().changed().connect(boost::bind(&Customization::SessionLoginChanged, this));
        clear();
        //
        statusline_ = new Wt::WText("", this);
        //
        ReadLitPref();
        PickedLiteratureContents * plc = new PickedLiteratureContents(session_, pickedliterature_);
        plc->SignalOkClicked().connect(boost::bind(&Customization::PlcOkClicked, this, plc));
        plc->SetSaveBoxChecked(true);
        plc->ShowSaveBox(false);
        //
        ccf_ = new CreateCurationForm(session_);
        //
        Wt::WContainerWidget * helpcontainer = new Wt::WContainerWidget();
        FillContainerWithHelpText(helpcontainer);
        //
        ccol = new CustomizeColors(session_);
        ccol->signalSaveClicked().connect(boost::bind(&Customization::colorSaveClicked, this));
        //
        tabwidget_ = new Wt::WTabWidget(this);
        tabwidget_->addTab(plc, "Literature Preference");
        //tabwidget_->addTab(new Wt::WContainerWidget(), "Categories Choices");
        tabwidget_->addTab(ccf_, "Curation Form");
        tabwidget_->addTab(ccol, "Curation Colors");
        tabwidget_->addTab(helpcontainer, "Help");
        tabwidget_->setCurrentIndex(0);
        SessionLoginChanged();
        alreadyloaded_ = true;
    }
}

void Customization::AddLitPrefFromFile(std::string fname, bool checkpermissions) {
    if (session_->login().state() != 0) {
        std::string username("");
        username = session_->login().user().identity("loginname").toUTF8();
        Preference * pref = new Preference(PGLITERATURE, PGLITPREFTABLENAME, username);
        Preference * permissions = new Preference(PGLITERATUREPERMISSION,
                PGLITERATUREPERMISSIONTABLENAME, username);
        Preference * dfpermissions = new Preference(PGLITERATUREPERMISSION,
                PGLITERATUREPERMISSIONTABLENAME, "default");
        std::ifstream f(fname.c_str());
        std::string in;
        while (getline(f, in)) {
            std::string newtext = "";
            boost::regex re("\\_\\d+$");
            std::string base = boost::regex_replace(in, re, newtext);
            // grant search rights based on individual and default permissions.
            if ((!checkpermissions || permissions->IsPreference(base)) || dfpermissions->IsPreference(base))
                if (pref->HasPreferences())
                    pickedliterature_[in] = (pref->IsPreference(base)) ? true : false;
                else
                    pickedliterature_[in] = true;
        }
        f.close();
        delete pref;
        delete permissions;
        delete dfpermissions;

    }
}

void Customization::ReadLitPref() {
    pickedliterature_.clear();
    AddLitPrefFromFile("/usr/local/textpresso/luceneindex/subindex.config", true);
    if (session_->login().state() != 0) {
        std::string fname("/usr/local/textpresso/luceneindex/" +
                session_->login().user().identity("loginname").toUTF8() +
                ".subindex.config");
        if (boost::filesystem::exists(fname)) AddLitPrefFromFile(fname, false);
    }
}

void Customization::colorSaveClicked() {
    statusline_->setText("Color Preference: Configuration saved.");
    statusline_->decorationStyle().setForegroundColor(Wt::green);
    EraseStatusLine();
}

void Customization::PlcOkClicked(PickedLiteratureContents * plc) {
    std::map < std::string, bool>::iterator it;
    for (it = pickedliterature_.begin(); it != pickedliterature_.end(); it++)
        (*it).second = plc->GetCheckState((*it).first);
    if (plc->SaveBoxIsChecked()) {
        if (session_->login().state() != 0) {
            std::string username("");
            username = session_->login().user().identity("loginname").toUTF8();
            Preference * pref = new Preference(PGLITERATURE, PGLITPREFTABLENAME, username);
            std::set<std::string> lits;
            for (std::map < std::string, bool>::iterator it = pickedliterature_.begin();
                    it != pickedliterature_.end(); it++) {
                if ((*it).second) {
                    std::string newtext = "";
                    boost::regex re("\\_\\d+$");
                    std::string base = boost::regex_replace((*it).first, re, newtext);
                    lits.insert(base);
                }
            }
            pref->SavePreferences(username, lits);
            delete pref;
            statusline_->setText("Literature Preference: Configuration saved.");
            statusline_->decorationStyle().setForegroundColor(Wt::green);
        }
    }
    EraseStatusLine();
}

void Customization::EraseStatusLine() {
    Wt::WTimer * timer = new Wt::WTimer();
    timer->setInterval(2000);
    timer->setSingleShot(true);
    timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
        timer->stop();
        delete timer;
        statusline_->setText("");
    }, timer));
    timer->start();
}

void Customization::SessionLoginChanged() {
    if (session_->login().state() == 0) {
        statusline_->setText("You must be logged in to use this feature.");
        statusline_->decorationStyle().setBackgroundColor(Wt::yellow);
        tabwidget_->hide();
    } else {
        statusline_->setText("");
        statusline_->decorationStyle().setBackgroundColor(Wt::white);
        tabwidget_->show();
    }
}

Customization::~Customization() {
}

void Customization::FillContainerWithHelpText(Wt::WContainerWidget * p) {
    //
    Wt::WText * t0 = new Wt::WText(
            "This page contains details on customizing Textpresso Central. "
            "Currently, literatures to be searched can be pre-selected, and "
            "curation forms configured."
            );
    t0->decorationStyle().font().setSize(Wt::WFont::Large);
    t0->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t1 = new Wt::WText(
            "Literature Preference"
            );
    t1->decorationStyle().font().setSize(Wt::WFont::Large);
    t1->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t1->decorationStyle().font().setWeight(Wt::WFont::Bold);
    //
    Wt::WText * t11 = new Wt::WText(
            "The 'Literature Preference' tab displays all literatures that are "
            "available for searching. Use the check boxes to select literatures "
            "and push the 'Ok' button to save preferences. Saved preferences can "
            "be reloaded with the 'Reload Saved Configuration' button."
            );
    t11->decorationStyle().font().setSize(Wt::WFont::Large);
    t11->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t2 = new Wt::WText(
            "Curation Form"
            );
    t2->decorationStyle().font().setSize(Wt::WFont::Large);
    t2->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t2->decorationStyle().font().setWeight(Wt::WFont::Bold);
    //
    Wt::WText * t21 = new Wt::WText(
            "Textpresso Central allows making annotations from a paper viewed "
            "in the 'Curation' tab with the help of a curation form which is "
            "configured in the 'Curation Form' sub-tab of the 'Customization' "
            "tab. The empty configuration form looks like this:"
            );
    t21->decorationStyle().font().setSize(Wt::WFont::Large);
    t21->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i22 = new Wt::WImage("resources/other_images/emptycurationform.png");
    //
    Wt::WText * t23 = new Wt::WText(
            "The fields in the left column specify global parameters of the form, "
            "while on the right side the data of the entry fields are repeated. "
            "In addition clicking on the green plus sign allows adding a data "
            "entry field of the curation form itself. The global fields "
            "are:"
            );
    t23->decorationStyle().font().setSize(Wt::WFont::Large);
    t23->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t24 = new Wt::WText("Form Name");
    t24->decorationStyle().font().setSize(Wt::WFont::Large);
    t24->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t24->decorationStyle().font().setStyle(Wt::WFont::Italic);
    //
    Wt::WText * t25 = new Wt::WText(": Identifying name of the form. A curator "
            "can work with as many curation forms as required and send "
            "annotations to as many databases. When viewing a paper a "
            "curation form can be retrieved at any time via its name."
            );
    t25->decorationStyle().font().setSize(Wt::WFont::Large);
    t25->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t26 = new Wt::WText("URL for Posting to External Database");
    t26->decorationStyle().font().setSize(Wt::WFont::Large);
    t26->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t26->decorationStyle().font().setStyle(Wt::WFont::Italic);
    //
    Wt::WText * t27 = new Wt::WText(": This field specifies the URL of the "
            "external database to which annotations should be submitted, "
            "The drop-down menu gives two choices. One can either specify a "
            "absolute URL address, and the address is entered in the box "
            "underneath the drop-down menu. Or one can refer the URL address "
            "in an incoming URL parameter. This means that "
            "one can specify the kick-out URL for a form in an incoming "
            "parameter of Textpresso Central's URL address. For example "
            "when Textpresso Central is reached via the URL "
            "http://tpc.textpresso.org/cgi-bin/tc/tpc/search"
            "?mydatabaseurl=http://mydatabase.com, searches, annotations are "
            "performed, and the curation form is filled out, then the URL "
            "specified in parameter 'mydatabaseurl' is used to submit the data, "
            "in this case http://mydatabase.com. When the option 'Incoming "
            "URL parameter' is used, then the parameter name need to be "
            "specified underneath the drop-down menu."
            );
    t27->decorationStyle().font().setSize(Wt::WFont::Large);
    t27->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t28 = new Wt::WText("Format for Sending Data");
    t28->decorationStyle().font().setSize(Wt::WFont::Large);
    t28->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t28->decorationStyle().font().setStyle(Wt::WFont::Italic);
    //
    Wt::WText * t29 = new Wt::WText(": Data to the external database can "
            "be sent via JSON or as URI parameters in the URL of the external "
            "database. Refer to the external database guidelines which format "
            "to use. The curation form itself then gives the option to "
            "choose the HTTP method to be POST or GET."
            );
    t29->decorationStyle().font().setSize(Wt::WFont::Large);
    t29->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t210 = new Wt::WText("Clicking on the green plus sign opens "
            "up a data configuration field like this: "
            );
    t210->decorationStyle().font().setSize(Wt::WFont::Large);
    t210->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i211 = new Wt::WImage("resources/other_images/datafield.png");
    //
    Wt::WText * t212 = new Wt::WText("Each field needs to have a name and a type "
            "of entry. Field names should coincide with the name the external "
            "database accepts. After choosing an entry type more options will "
            "appear depending on the choice. Many if not all aspects of the "
            "data configuration field are explained in popup windows that can "
            "be opened by clicking on the question marks in purple circles. "
            );
    t212->decorationStyle().font().setSize(Wt::WFont::Large);
    t212->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t213 = new Wt::WText("All fields in the curation form will be "
            "arranged on a square grid. To introduce more fields, click on "
            "additional green plus signs below or to the right of an existing "
            "data configuration field. To delete a field, click on the red "
            "minus sign to the right of it."
            );
    t213->decorationStyle().font().setSize(Wt::WFont::Large);
    t213->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t3 = new Wt::WText(
            "Curation Colors"
            );
    t3->decorationStyle().font().setSize(Wt::WFont::Large);
    t3->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t3->decorationStyle().font().setWeight(Wt::WFont::Bold);
    //
    Wt::WText * t31 = new Wt::WText(
            "Highlighting colors for 'Search' and 'Curation' pages are fully customizable. In the former page, for "
            "example, results are highlighted according to the keywords and categories specified for the "
            "search, as can be seen in the figure below.");
    t31->decorationStyle().font().setSize(Wt::WFont::Large);
    t31->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i31 = new Wt::WImage("resources/other_images/keyword_and_cat_search.jpg");
    i31->setMaximumSize(Wt::WLength(1300), Wt::WLength(800));
    //
    Wt::WText * t32 = new Wt::WText(
            "The first keyword entered in the search form is highlighted with the first color in the 'Keywords' "
            "color palette, and so on for other subsequent keywords entered in the form. Similarly, the colors for "
            "categories correspond to the 'Categories' color palette, as can be seen in the color scheme below:"
            );
    //
    t32->decorationStyle().font().setSize(Wt::WFont::Large);
    t32->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i32 = new Wt::WImage("resources/other_images/colorscheme.jpg");
    i32->setMaximumSize(Wt::WLength(800), Wt::WLength(250));
    //
    Wt::WText * t33 = new Wt::WText(
            "In the 'Curation' page, keywords are highlighted according to the 'Keywords' palette, and labels "
            "according to the 'Labels' palette."
            );
    //
    t33->decorationStyle().font().setSize(Wt::WFont::Large);
    t33->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i33 = new Wt::WImage("resources/other_images/keyword_label_viewer.jpg");
    i33->setMaximumSize(Wt::WLength(1300), Wt::WLength(600));
    //
    Wt::WText * t34 = new Wt::WText(
            "To change the color scheme in the palettes, click on the colors in the 'Curation Colors' page and select "
            "the new values form the color picker. Don't forget to save the changes before leaving the page!"
            );
    //
    t34->decorationStyle().font().setSize(Wt::WFont::Large);
    t34->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    p->addWidget(t0);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t1);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t11);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t2);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t21);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(i22);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t23);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t24);
    p->addWidget(t25);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t26);
    p->addWidget(t27);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t28);
    p->addWidget(t29);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t210);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(i211);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t212);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t213);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t3);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t31);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(i31);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t32);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(i32);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t33);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(i33);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t34);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
}
