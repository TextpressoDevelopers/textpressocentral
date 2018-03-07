/* 
 * File:   AutocompleteAndValidationDialog.cpp
 * Author: mueller
 * 
 * Created on June 8, 2015, 11:46 AM
 */

#include "AutocompleteAndValidationDialog.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include "HelpDialog.h"
#include <boost/algorithm/string.hpp>
#include <pqxx/pqxx>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WText>
#include <Wt/WCheckBox>
#include <Wt/WGridLayout>
#include <Wt/WPushButton>
#include <Wt/WGroupBox>
#include <Wt/WLabel>
#include <Wt/WImage>
#include <Wt/WCssDecorationStyle>
#include <Wt/WBreak>
#include <Wt/WButtonGroup>
#include <Wt/WRadioButton>
#include <Wt/WStackedWidget>

AutocompleteAndValidationDialog::AutocompleteAndValidationDialog(bool validationdialog) {
    validationdialog_ = validationdialog;
    setModal(true);
    setResizable(true);
    setClosable(true);
    Wt::WVBoxLayout * vbox = new Wt::WVBoxLayout();
    vbox->setSpacing(36);
    contents()->setLayout(vbox);
    setMaximumSize(Wt::WLength::Auto, Wt::WLength(64, Wt::WLength::FontEx));
    //
    Wt::WImage * qmark = new Wt::WImage("resources/icons/qmark15.png");
    Wt::WContainerWidget * qmarkcont = new Wt::WContainerWidget();
    qmark->setFloatSide(Wt::Right);
    qmarkcont->addWidget(qmark);
    vbox->addWidget(qmarkcont);
    qmark->mouseWentOver().connect(std::bind([ = ] (){
        qmark->decorationStyle().setCursor(Wt::PointingHandCursor);
    }));
    qmark->clicked().connect(std::bind([ = ] (){
        Wt::WContainerWidget * helpcontent = new Wt::WContainerWidget();
        Wt::WText * general1 = new Wt::WText();
        general1->setText(
        "Most of the input in this window specifies the exact location of the "
        "Postgres table and column from which the suggestions for autocompletions "
        "should be drawn. All entries of a table column are used."
        );
        helpcontent->addWidget(general1);
        helpcontent->addWidget(new Wt::WBreak());
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption1 = new Wt::WText(
        "Use autocomplete info as validation info "
        "(check box): "
        );
        Wt::WText * expl1 = new Wt::WText(
        "If you want to use the same information for autocomplete and validation, "
        "click this box."
        );
        caption1->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption1);
        helpcontent->addWidget(expl1);
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption2 = new Wt::WText("Populate Autocomplete Form (button): ");
        Wt::WText * expl2 = new Wt::WText(
        "Textpresso categories can be used for autocomplete and validation purposes. "
        "If you want to do so, press this button. The form will be prepopulated according "
        "to your subsequent choices. "
        );
        caption2->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption2);
        helpcontent->addWidget(expl2);
        helpcontent->addWidget(new Wt::WBreak());

        Wt::WText * caption3 = new Wt::WText("All other fields: ");
        Wt::WText * expl3 = new Wt::WText(
        "All other fields determine where the Postgres table and column is located. "
        "You can include one where clause to specify the query to your needs. "
        );
        caption3->decorationStyle().font().setWeight(Wt::WFont::Bold);
        helpcontent->addWidget(caption3);
        helpcontent->addWidget(expl3);
        helpcontent->addWidget(new Wt::WBreak());

        helpcontent->setWidth(Wt::WLength(48, Wt::WLength::FontEx));
        helpcontent->setPadding(Wt::WLength(1, Wt::WLength::FontEx));
        helpcontent->setContentAlignment(Wt::AlignJustify);
        new HelpDialog("Autocomplete and Validation Help", false, helpcontent);
    }));
    // Radio button group for choosing postgres or web service
    Wt::WContainerWidget * radiogroupcontainer = new Wt::WContainerWidget();
    radiogroupcontainer->addWidget(new Wt::WText("Choice of source: "));
    radiogroupcontainer->addWidget(new Wt::WBreak());
    radiogroupcontainer->decorationStyle().font().setWeight(Wt::WFont::Bold);
    Wt::WButtonGroup * radiogroup = new Wt::WButtonGroup(radiogroupcontainer);
    Wt::WRadioButton * button;
    button = new Wt::WRadioButton("Postgres table", radiogroupcontainer);
    radiogroup->addButton(button);
    button = new Wt::WRadioButton("Web service", radiogroupcontainer);
    radiogroup->addButton(button);
    vbox->addWidget(radiogroupcontainer);
    //
    //
    Wt::WContainerWidget * postgresdisplay = new Wt::WContainerWidget();
    Wt::WContainerWidget * querydialogs = new Wt::WContainerWidget();
    Wt::WHBoxLayout * hbox = new Wt::WHBoxLayout();
    hbox->setSpacing(100);
    querydialogs->setLayout(hbox);
    if (validationdialog_) {
        v_is_same1_ = new Wt::WCheckBox("Use autocomplete info as validation info.");
        postgresdisplay->addWidget(v_is_same1_);
    }
    Wt::WContainerWidget * spib = InfoBox(autocomplete_, "Autocomplete Info");
    hbox->addWidget(spib);
    if (validationdialog_) {
        Wt::WContainerWidget * vpib = InfoBox(validation_, "Validation Info");
        v_is_same1_->clicked().connect(std::bind([ = ] (){
            if (v_is_same1_->isChecked()) {
                vpib->disable();
                        DeepCopyInfo(autocomplete_, validation_);
            } else
                vpib->enable();
            }));
        hbox->addWidget(vpib);
    }
    //
    Wt::WPushButton * populateform = new Wt::WPushButton("Populate Autocomplete Form!");
    populateform->setStyleClass("btn-small");
    populateform->setMaximumSize(Wt::WLength(30, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    populateform->setMinimumSize(Wt::WLength(30, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    populateform->clicked().connect(this, &AutocompleteAndValidationDialog::PopulateInfoBox);
    //
    postgresdisplay->addWidget(new Wt::WBreak());
    postgresdisplay->addWidget(populateform);
    postgresdisplay->addWidget(querydialogs);
    postgresdisplay->setMinimumSize(Wt::WLength(80, Wt::WLength::FontEx), Wt::WLength::Auto);
    //
    Wt::WContainerWidget * webservicedisplay = new Wt::WContainerWidget();
    //
    Wt::WLabel * lahostname = new Wt::WLabel("Autocomplete URL: ");
    ahostname_ = new Wt::WLineEdit();
    ahostname_->setEmptyText("(Type complete URL of web service.)");
    ahostname_->setWidth(Wt::WLength(80, Wt::WLength::FontEx));
    ahostname_->changed().connect(boost::bind(&AutocompleteAndValidationDialog::AutocompleteURLChanged, this));
    webservicedisplay->addWidget(lahostname);
    webservicedisplay->addWidget(ahostname_);
    //
    webservicedisplay->addWidget(new Wt::WBreak());
    //
    Wt::WLabel * lvhostname = new Wt::WLabel("Validation URL: ");
    vhostname_ = new Wt::WLineEdit();
    vhostname_->setEmptyText("(Type complete URL of web service.)");
    vhostname_->setWidth(Wt::WLength(80, Wt::WLength::FontEx));
    vhostname_->changed().connect(boost::bind(&AutocompleteAndValidationDialog::ValidationURLChanged, this));
    webservicedisplay->addWidget(lvhostname);
    webservicedisplay->addWidget(vhostname_);
    //
    if (validationdialog_) {
        v_is_same2_ = new Wt::WCheckBox("Use autocomplete info as validation info.");
        v_is_same2_->clicked().connect(std::bind([ = ] (){
            if (v_is_same2_->isChecked()) {
                vhostname_->disable();
                        DeepCopyInfo(autocomplete_, validation_);
            } else {
                vhostname_->enable();
            }
        }));
        webservicedisplay->addWidget(new Wt::WBreak());
        webservicedisplay->addWidget(v_is_same2_);
        webservicedisplay->addWidget(new Wt::WBreak());
    }
    //
    Wt::WPushButton * okp = new Wt::WPushButton("Ok");
    okp->setStyleClass("btn-small");
    okp->setMaximumSize(Wt::WLength(6, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    okp->setMinimumSize(Wt::WLength(6, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    okp->clicked().connect(this, &Wt::WDialog::accept);
    Wt::WPushButton * okw = new Wt::WPushButton("Ok");
    okw->setStyleClass("btn-small");
    okw->setMaximumSize(Wt::WLength(6, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    okw->setMinimumSize(Wt::WLength(6, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    okw->clicked().connect(this, &Wt::WDialog::accept);
    postgresdisplay->addWidget(okp);
    webservicedisplay->addWidget(okw);
    //
    Wt::WStackedWidget * stack = new Wt::WStackedWidget();
    stack->decorationStyle().setBackgroundColor(Wt::WColor(216, 218, 246));
    stack->setPadding(Wt::WLength(5, Wt::WLength::Pixel));
    stack->addWidget(postgresdisplay);
    stack->addWidget(webservicedisplay);
    stack->addWidget(new Wt::WText("Please make a selection above!"));
    stack->setCurrentIndex(2);
    radiogroup->checkedChanged().connect(std::bind([ = ](){
        stack->setCurrentIndex(radiogroup->selectedButtonIndex());
        resize(stack->width(), stack->height());
        contents()->resize(stack->width(), stack->height());
    }));
    vbox->addWidget(stack);
}

AutocompleteAndValidationDialog::~AutocompleteAndValidationDialog() {
}

void AutocompleteAndValidationDialog::SetTitle(Wt::WString name) {
    Wt::WString s("Autocomplete and Validation");
    if (!name.empty()) s += " for " + name;
    setWindowTitle(s);
}

void AutocompleteAndValidationDialog::DeepCopyInfo(forminfo source, forminfo & dest) {
    dest.dbname->setText(source.dbname->text());
    dest.tablename->setText(source.tablename->text());
    dest.columnname->setText(source.columnname->text());
    dest.whereclause->setText(source.whereclause->text());
    dest.hostname->setText(source.hostname->text());
    dest.port->setText(source.port->text());
    dest.username->setText(source.username->text());
    dest.password->setText(source.password->text());
}

void AutocompleteAndValidationDialog::PopulateInfoBox() {
    //
    idmap_.clear();
    try {
        pqxx::connection cn(PGLISTOFONTOLOGIES);
        pqxx::work w(cn);
        pqxx::result r;
        std::stringstream pc;
        pc << "select idprefix,name from ";
        pc << PGLISTOFONTOLOGIESTABLENAME;
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string idprefix;
            std::string name;
            if (r[i]["idprefix"].to(idprefix))
                if (r[i]["name"].to(name)) {
                    idmap_[name] = idprefix;
                }
        }
        w.commit();
        cn.disconnect();
    } catch (const std::exception &e) {
        std::cerr << e.what() << std::endl;
    }
    //
    Wt::WDialog * ppac = new Wt::WDialog("Populate Autocomplete Form");
    ppac->setModal(false);
    ppac->setResizable(true);
    ppac->setClosable(true);
    ppac->finished().connect(this, &AutocompleteAndValidationDialog::PpacDialogDone);
    Wt::WVBoxLayout * dialogvlayout = new Wt::WVBoxLayout();
    ppac->contents()->setLayout(dialogvlayout);
    //
    Wt::WString explanation = "Please select the ontologies whose terms "
            "you want to include in the autocomplete list.";
    dialogvlayout->addWidget(new Wt::WText(explanation));
    //
    Wt::WContainerWidget * gridcont = new Wt::WContainerWidget();
    Wt::WGridLayout * grid = new Wt::WGridLayout();
    gridcont->setLayout(grid);
    int x(0), y(0), count(0);
    int squaresize(4);
    checkboxlist_.clear();
    std::map < std::string, std::string>::iterator it;
    for (it = idmap_.begin(); it != idmap_.end(); it++) {
        x = count % squaresize;
        y = count / squaresize;
        Wt::WCheckBox * aux = new Wt::WCheckBox((*it).first);
        checkboxlist_.push_back(aux);
        aux->setChecked(false);
        grid->addWidget(aux, y + 1, x);
        count++;
    }
    gridcont->setMinimumSize(Wt::WLength(squaresize * 20, Wt::WLength::FontEx),
            Wt::WLength(5 * y, Wt::WLength::FontEx));
    dialogvlayout->addWidget(gridcont);
    //
    Wt::WPushButton * ok = new Wt::WPushButton("Ok");
    ok->setMaximumSize(Wt::WLength(10, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));
    ok->clicked().connect(ppac, &Wt::WDialog::accept);
    dialogvlayout->addWidget(ok);
    //
    ppac->show();
}

void AutocompleteAndValidationDialog::PpacDialogDone(Wt::WDialog::DialogCode code) {
    if (code == Wt::WDialog::Accepted) {
        autocomplete_.columnname->setText("term");
        std::string aux(PGONTOLOGY);
        boost::replace_first(aux, "dbname=", "");
        autocomplete_.dbname->setText(Wt::WString(aux));
        autocomplete_.hostname->setText("");
        autocomplete_.password->setText("");
        autocomplete_.port->setText("");
        autocomplete_.tablename->setText(PGONTOLOGYTABLENAME);
        std::string condition("eid ~ '^(");
        bool oneitemchecked = false;
        std::vector<Wt::WCheckBox*>::iterator it;
        for (it = checkboxlist_.begin(); it != checkboxlist_.end(); it++) {
            if ((*it)->isChecked()) {
                std::string name((*it)->text().toUTF8());
                if (oneitemchecked) condition += "|";
                condition += idmap_[name];
                oneitemchecked = true;
            }
        }
        condition += ")'";
        if (oneitemchecked)
            autocomplete_.whereclause->setText(condition);
        else
            autocomplete_.whereclause->setText("");
        CheckToCopy();
    }
}

Wt::WContainerWidget * AutocompleteAndValidationDialog::InfoBox(forminfo & fi, Wt::WString s) {
    Wt::WContainerWidget * ret = new Wt::WContainerWidget();
    //
    Wt::WGroupBox * groupBox = new Wt::WGroupBox(s);
    groupBox->addStyleClass("centered-example");
    //
    Wt::WLabel * ldbname = new Wt::WLabel("Database Name", groupBox);
    fi.dbname = new Wt::WLineEdit(groupBox);
    fi.dbname->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    ldbname->setBuddy(fi.dbname);
    //
    Wt::WLabel * ltablename = new Wt::WLabel("Table Name", groupBox);
    fi.tablename = new Wt::WLineEdit(groupBox);
    fi.tablename->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    ltablename->setBuddy(fi.tablename);
    //
    Wt::WLabel * lcolumnname = new Wt::WLabel("Column Name", groupBox);
    fi.columnname = new Wt::WLineEdit(groupBox);
    fi.columnname->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    lcolumnname->setBuddy(fi.columnname);
    //
    Wt::WLabel * lwhereclause = new Wt::WLabel("Where Clause", groupBox);
    fi.whereclause = new Wt::WLineEdit(groupBox);
    fi.whereclause->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    lwhereclause->setBuddy(fi.whereclause);
    //
    Wt::WLabel * lhostname = new Wt::WLabel("Host Name", groupBox);
    fi.hostname = new Wt::WLineEdit(groupBox);
    fi.hostname->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    lhostname->setBuddy(fi.hostname);
    //
    Wt::WLabel * lport = new Wt::WLabel("Port", groupBox);
    fi.port = new Wt::WLineEdit(groupBox);
    fi.port->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    lport->setBuddy(fi.port);
    //
    Wt::WLabel * lusername = new Wt::WLabel("User Name", groupBox);
    fi.username = new Wt::WLineEdit(groupBox);
    fi.username->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    lusername->setBuddy(fi.username);
    //
    Wt::WLabel * lpassword = new Wt::WLabel("Password", groupBox);
    fi.password = new Wt::WLineEdit(groupBox);
    fi.password->changed().connect(this, &AutocompleteAndValidationDialog::CheckToCopy);
    fi.password->setEchoMode(Wt::WLineEdit::Password);
    lpassword->setBuddy(fi.password);
    //
    groupBox->addWidget(fi.dbname);
    groupBox->addWidget(fi.tablename);
    groupBox->addWidget(fi.hostname);
    groupBox->addWidget(fi.port);
    groupBox->addWidget(fi.username);
    groupBox->addWidget(fi.password);
    //
    ret->addWidget(groupBox);

    return ret;
}

void AutocompleteAndValidationDialog::CheckToCopy() {
    if (validationdialog_)
        if ((v_is_same1_->isChecked()) || (v_is_same2_->isChecked()))
            DeepCopyInfo(autocomplete_, validation_);
}

void AutocompleteAndValidationDialog::AutocompleteURLChanged() {
    autocomplete_.hostname->setText(ahostname_->text());
    if (v_is_same2_->isChecked())
        vhostname_->setText(ahostname_->text());
    CheckToCopy();
}

void AutocompleteAndValidationDialog::ValidationURLChanged() {
    validation_.hostname->setText(vhostname_->text());
}
