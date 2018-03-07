/* 
 * File:   PrePopulationDialog.cpp
 * Author: mueller
 * 
 * Created on July 16, 2015, 10:11 AM
 */

#include "PrePopulationDialog.h"
#include "Preference.h"
#include "HelpDialog.h"
#include <Wt/WPushButton>
#include <Wt/WVBoxLayout>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WBreak>
#include <Wt/WLineEdit>
#include <Wt/WFont>
#include <Wt/WCssDecorationStyle>
#include <Wt/WImage>

PrePopulationDialog::PrePopulationDialog(Session * session) {
    session_ = session;
    setModal(false);
    setResizable(true);
    setClosable(true);
    //
    Wt::WPushButton * ok = new Wt::WPushButton("Ok");
    ok->setStyleClass("btn-small");
    ok->clicked().connect(this, &Wt::WDialog::accept);
    //
    Wt::WContainerWidget * bgroupcont = new Wt::WContainerWidget();
    Wt::WText * modelabel = new Wt::WText("Mode of Population: ");
    modegroup_ = new Wt::WButtonGroup(bgroupcont);
    Wt::WRadioButton * button;
    button = new Wt::WRadioButton("Static text", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    button = new Wt::WRadioButton("Terms found in text spans and matched by underlying categories.", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    button = new Wt::WRadioButton("Terms and their synonyms. Terms found in text spans and matched by underlying categories.", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    button = new Wt::WRadioButton("Matched underlying categories of terms found in text spans.", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    button = new Wt::WRadioButton("Terms and their matched underlying categories found in text spans.", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    button = new Wt::WRadioButton("Terms, their synonyms and matched underlying categories. Terms found in text spans.", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    button = new Wt::WRadioButton("Data from paper info", bgroupcont);
    modegroup_->addButton(button);
    bgroupcont->addWidget(new Wt::WBreak());
    modegroup_->checkedChanged().connect(boost::bind(&PrePopulationDialog::RadioButtonChecked, this));
    //
    prepoptext_ = new Wt::WText();
    bgroupcont->addWidget(new Wt::WBreak());
    prepoptextlabel_ = new Wt::WText("Current prepopulation data: ");
    prepoptext_->decorationStyle().font().setWeight(Wt::WFont::Bold);
    prepoptextlabel_->decorationStyle().font().setWeight(Wt::WFont::Bold);
    prepoptext_->hide();
    prepoptextlabel_->hide();
    bgroupcont->addWidget(prepoptextlabel_);
    bgroupcont->addWidget(prepoptext_);
    //
    SetPrePopString("");
    //
    prepopdetailscont_ = new Wt::WContainerWidget();
    prepopdetailscont_->setContentAlignment(Wt::AlignMiddle);
    //
    Wt::WPushButton * syndatainfobutton = new Wt::WPushButton("Enter Synonym Database Info");
    syndatainfobutton->setStyleClass("btn-small");
    syndatainfobutton->clicked().connect(boost::bind(&PrePopulationDialog::SynDataInfoButtonClicked, this));
    //
    Wt::WImage * qmark = new Wt::WImage("resources/icons/qmark15.png");
    qmark->setFloatSide(Wt::Right);
    contents()->addWidget(qmark);
    qmark->mouseWentOver().connect(std::bind([ = ] (){
        qmark->decorationStyle().setCursor(Wt::PointingHandCursor);
    }));
    qmark->clicked().connect(std::bind([ = ] (){
        Wt::WContainerWidget * helpcontent = new Wt::WContainerWidget();
        Wt::WText * general1 = new Wt::WText();
        general1->setText(
        "The main purpose of prepopulating data entries is to ease the transfer "
        "of text from a paper into the form. The entries is found by matching "
        "underlying categories in text spans marked by the user. For example, "
        "if you ask the entry field to be filled with a human disease (category), "
        "and the text span contains the term chickenpox, then the website fills "
        "in this term automatically. Another purpose of prepopulating data entries "
        "is to fill it with a static text that does not change often."
        );
        helpcontent->addWidget(general1);
        helpcontent->addWidget(new Wt::WBreak());
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption1 = new Wt::WText("Mode of Population: ");
        Wt::WText * expl1 = new Wt::WText(
        "Pick which combination of term, synonyms, category or static text you "
        "would like the website to fill in. Information from the paper can "
        "also be used. Choose 'Data from paper info' for that."
        );
        caption1->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption1);
        helpcontent->addWidget(expl1);
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption2 = new Wt::WText("Enter Synonym Database Info (button): ");
        Wt::WText * expl2 = new Wt::WText(
        "If you would like to have synonyms of terms in the data entry field, "
        "specify the column, tablename and database info in the window that pops "
        "up. The synonym information is read in from one column in a Postgres "
        "table. Each table cell contains all synonyms for a term (including the term itself) "
        "separated by a '|'."
        );
        caption2->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption2);
        helpcontent->addWidget(expl2);
        helpcontent->addWidget(new Wt::WBreak());


        helpcontent->setWidth(Wt::WLength(48, Wt::WLength::FontEx));
        helpcontent->setPadding(Wt::WLength(1, Wt::WLength::FontEx));
        helpcontent->setContentAlignment(Wt::AlignJustify);
        new HelpDialog("Prepopulation Help", false, helpcontent);
    }));

    contents()->addWidget(modelabel);
    contents()->addWidget(new Wt::WBreak());
    contents()->addWidget(bgroupcont);
    contents()->addWidget(new Wt::WBreak());
    contents()->addWidget(prepopdetailscont_);
    contents()->addWidget(new Wt::WBreak());
    contents()->addWidget(syndatainfobutton);
    contents()->addWidget(new Wt::WBreak());
    contents()->addWidget(new Wt::WBreak());
    contents()->addWidget(ok);
    //
    ReadPreloadedCategories();
    //
    sdidialog_ = new Wt::WDialog("Synonym Database", this);
    sdidialog_->setModal(false);
    sdidialog_->setResizable(true);
    sdidialog_->setClosable(true);
    AutocompleteAndValidationDialog * dummyaav = new AutocompleteAndValidationDialog(false);
    Wt::WContainerWidget * infobox = dummyaav->AutocompleteAndValidationDialog::InfoBox(sdiinfo_, "Enter the following info");
    Wt::WPushButton * sdiok = new Wt::WPushButton("Ok");
    sdiok->setStyleClass("btn-small");
    sdiok->clicked().connect(boost::bind(&PrePopulationDialog::SdiDialogDone, this, sdiinfo_));
    sdidialog_->contents()->addWidget(infobox);
    sdidialog_->contents()->addWidget(sdiok);
    sdidialog_->hide();
}

PrePopulationDialog::~PrePopulationDialog() {
}

void PrePopulationDialog::SetTitle(Wt::WString name) {
    Wt::WString s("Prepopulation of Field ");
    if (!name.empty()) s += name;
    setWindowTitle(s);
}

void PrePopulationDialog::SetPrePopString(Wt::WString s) {
    prepopstring_ = s;
    prepoptext_->setText(prepopstring_);
    if (!prepopstring_.empty()) {
        prepoptextlabel_->show();
        prepoptext_->show();
    } else {
        prepoptextlabel_->hide();
        prepoptext_->hide();
    }
}

void PrePopulationDialog::SetPrePopMode(int i) {
    prepopmode_ = i;
    modegroup_->setSelectedButtonIndex(prepopmode_);
    RadioButtonChecked();
}

void PrePopulationDialog::RadioButtonChecked() {
    prepopdetailscont_->clear();
    prepopdetailscont_->setContentAlignment(Wt::AlignMiddle);
    Wt::WText * explanation = new Wt::WText();
    explanation->setInline(true);
    prepopdetailscont_->addWidget(explanation);
    //
    Wt::WLineEdit * linp = new Wt::WLineEdit(prepopstring_);
    linp->changed().connect(std::bind([ = ] (){
        SetPrePopString(linp->text());
    }));
    linp->setInline(true);
    //
    Wt::WPushButton * pcatbutton = new Wt::WPushButton("Pick!");
    pcatbutton->clicked().connect(std::bind([ = ] (){
        pickcat_ = new PickCategory(session_, preloadedcategories_);
        pickcat_->finished().connect(this, &PrePopulationDialog::PickCategoryDialogDone);
        pickcat_->show();
    }));
    //
    Wt::WContainerWidget * bgroupvicont = new Wt::WContainerWidget();
    Wt::WButtonGroup * bgroupvi = new Wt::WButtonGroup(bgroupvicont);
    Wt::WRadioButton * button;
    button = new Wt::WRadioButton("Terms", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Positions", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Add. Annotations", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Filename", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Paper Id", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Annotator", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Created", bgroupvicont);
    bgroupvi->addButton(button);
    button = new Wt::WRadioButton("Version", bgroupvicont);
    bgroupvi->addButton(button);
    bgroupvi->checkedChanged().connect(std::bind([ = ] () {
        SetPrePopString(bgroupvi->checkedButton()->text());
    }));
    //
    switch (modegroup_->checkedId()) {
        case statictext:
            explanation->setText("Edit static text: ");
            prepopdetailscont_->addWidget(linp);
            break;
        case viewerinfodata:
            explanation->setText("Select field name: ");
            prepopdetailscont_->addWidget(bgroupvicont);
            break;
        default:
            explanation->setText("Launch Dialog for Category Selection: ");
            prepopdetailscont_->addWidget(pcatbutton);
            break;
    }
    prepopmode_ = modegroup_->checkedId();
}

void PrePopulationDialog::SynDataInfoButtonClicked() {
    sdidialog_->show();
}

void PrePopulationDialog::SdiDialogDone(AutocompleteAndValidationDialog::forminfo fi) {
    syndatabase_.col = fi.columnname->text().toUTF8();
    syndatabase_.tablename = fi.tablename->text().toUTF8();
    std::string where = fi.whereclause->text().toUTF8();
    boost::replace_all(where, "'", "''");
    syndatabase_.whereclause = where;
    if (!fi.dbname->text().empty()) { // fill database only if dbname is present
        std::string db("dbname=" + fi.dbname->text().toUTF8());
        if (!fi.hostname->text().empty())
            db += " host=" + fi.hostname->text().toUTF8();
        if (!fi.port->text().empty())
            db += " port=" + fi.port->text().toUTF8();
        if (!fi.username->text().empty())
            db += " user=" + fi.username->text().toUTF8();
        if (!fi.password->text().empty())
            db += " password=" + fi.password->text().toUTF8();
        syndatabase_.database = db;
    }
    sdidialog_->hide();
}

void PrePopulationDialog::PickCategoryDialogDone(Wt::WDialog::DialogCode code) {
    if (code == Wt::WDialog::Accepted) {
        std::set<Wt::WString> list(pickcat_->GetSelected());
        Wt::WString c = (pickcat_->CatAnded()) ? "&" : "|";
        std::set<Wt::WString>::iterator it = list.begin();
        Wt::WString aux(*it);
        while (++it != list.end()) aux += c + *it;
        SetPrePopString(aux);
    }
    delete pickcat_;
}

void PrePopulationDialog::ReadPreloadedCategories() {
    std::string username("default");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname").toUTF8();
    TpCategoryBrowser * tcp = new TpCategoryBrowser();
    std::set<std::string> plist = tcp->GetAllDirectChildrensName("root");
    delete tcp;
    Preference * pref = new Preference(PGPRELOADEDCATEGORIES, PGPRELOADEDCATTABLENAME, username);
    preloadedcategories_.clear();
    std::set<std::string>::iterator it;
    for (it = plist.begin(); it != plist.end(); it++) {
        if (pref->HasPreferences())
            if (pref->IsPreference(*it)) preloadedcategories_.insert(*it);
    }
    delete pref;
}

