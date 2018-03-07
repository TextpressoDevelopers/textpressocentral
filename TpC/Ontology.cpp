/* 
 * File:   Ontology.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 10:20 AM
 */

#include "Ontology.h"
#include "AssistedFieldValueForm.h"
#include "LexicalVariations.h"

#include <fstream>
#include <boost/algorithm/string.hpp>

#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WContainerWidget>
#include <Wt/WPopupMenu>
#include <Wt/WText>
#include <Wt/WToolBar>
#include <Wt/WCssDecorationStyle>
#include <Wt/WImage>
#include <Wt/WApplication>
#include <Wt/WFileUpload>

namespace {

    Wt::WPushButton * createColorButton(const char * className,
            const Wt::WString & text) {
        Wt::WPushButton * button = new Wt::WPushButton();
        button->setTextFormat(Wt::XHTMLText);
        button->setText(text);
        button->addStyleClass(className);
        return button;
    }

    void MakeModelUserEditable(Wt::WString loginname, Wt::WStandardItemModel * model) {
        int iowner;
        for (int ic = 3; ic < model->columnCount(); ic++)
            if (Wt::asString(model->headerData(ic)).toUTF8().compare("owner") == 0)
                iowner = ic;
        for (int ir = 0; ir < model->rowCount(); ir++)
            for (int ic = 3; ic < model->columnCount(); ic++)
                if (model->item(ir, iowner)->text() == loginname)
                    model->item(ir, ic)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsEditable);
                else
                    model->item(ir, ic)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);

    }

}

Ontology::Ontology(Session & session, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    messagecontainer_ = new Wt::WContainerWidget();
    message_ = new Wt::WText();
    messagecontainer_->addWidget(message_);
    AuthEvent(&session);
    session.login().changed().connect(boost::bind(&Ontology::AuthEvent, this, &session));
    this->addWidget(new Wt::WBreak);
    Wt::WContainerWidget * buttoncontainer = new Wt::WContainerWidget(this);
    buttoncontainer->setInline(true);
    actioncontainer_ = new Wt::WContainerWidget();
    resultcontainer_ = new Wt::WContainerWidget();
    snapshotcount_ = 0;
    model_ = new Wt::WStandardItemModel(0, 0);
    model_->itemChanged().connect(this, &Ontology::OntItemChanged);
    itemchangedupdate_ = true;
    ot_ = new OntologyTable(model_);
    selectall_ = new Wt::WCheckBox("Select All");
    selectall_->setChecked(false);
    resultcontainer_->addWidget(selectall_);
    resultcontainer_->addWidget(ot_);
    resultcontainer_->hide();
    this->addWidget(messagecontainer_);
    this->addWidget(actioncontainer_);
    this->addWidget(resultcontainer_);
    Wt::WToolBar * toolBar = new Wt::WToolBar(buttoncontainer);
    Wt::WPushButton * ontsearchbutton = createColorButton("btn-search", "Search");
    Wt::WPushButton * ontfilterbutton = createColorButton("btn-filter", "Filter");
    Wt::WPushButton * ontinsertbutton = createColorButton("btn-insert", "Insert");
    ontdelbutton_ = createColorButton("btn-delete", "Delete");
    ontlexicalbutton_ = createColorButton("btn-lexical", "Lex. Var.");
    Wt::WPushButton * ontuploadbutton = createColorButton("btn-upload", "Upload");
    ontsnapshotbutton_ = createColorButton("btn-snapshot", "Snapshot");
    ontreversebutton_ = createColorButton("btn-reverse", "Reverse");
    ontcommitbutton_ = createColorButton("btn-commit", "Commit");
    toolBar->addButton(ontsearchbutton);
    toolBar->addButton(ontfilterbutton);
    toolBar->addButton(ontuploadbutton);
    toolBar->addSeparator();
    toolBar->addButton(ontinsertbutton);
    toolBar->addButton(ontdelbutton_);
    toolBar->addButton(ontlexicalbutton_);
    toolBar->addButton(ontsnapshotbutton_);
    toolBar->addButton(ontreversebutton_);
    toolBar->addSeparator();
    toolBar->addButton(ontcommitbutton_);

    //    ontinsertbutton_->disable();
    ontdelbutton_->disable();
    ontlexicalbutton_->disable();
    ontsnapshotbutton_->disable();
    ontreversebutton_->disable();
    ontcommitbutton_->disable();

    ontsearchbutton->clicked().connect(boost::bind(&Ontology::OntSearchClicked, this));
    ontfilterbutton->clicked().connect(boost::bind(&Ontology::OntFilterClicked, this));
    ontinsertbutton->clicked().connect(boost::bind(&Ontology::OntInsertClicked, this));
    ontdelbutton_->clicked().connect(boost::bind(&Ontology::OntDeleteClicked, this));
    ontlexicalbutton_->clicked().connect(boost::bind(&Ontology::OntLexicalClicked, this));
    ontcommitbutton_->clicked().connect(boost::bind(&Ontology::OntCommitClicked, this));
    ontuploadbutton->clicked().connect(boost::bind(&Ontology::OntUploadClicked, this));
    ontsnapshotbutton_->clicked().connect(boost::bind(&Ontology::OntSnapshotClicked, this));
    ontreversebutton_->clicked().connect(boost::bind(&Ontology::OntReverseClicked, this));
    selectall_->clicked().connect(boost::bind(&Ontology::OntSelectAllClicked, this));
}

void Ontology::OntSelectAllClicked() {
    if (model_ != NULL) {
        bool state = selectall_->isChecked();
        for (int i = 0; i < model_->rowCount(); i++)
            model_->item(i, 0)->setChecked(state);
    }
}

void Ontology::OntItemChanged(Wt::WStandardItem * si) {
    if (itemchangedupdate_)
        if (model_ != NULL)
            if (si != NULL) {
                int cf = -1;
                int rf = -1;
                for (int ic = 0; (ic < model_->columnCount()) && (cf < 0); ic++)
                    for (int ir = 0; (ir < model_->rowCount()) && (rf < 0); ir++)
                        if (model_->item(ir, ic) == si) {
                            cf = ic;
                            rf = ir;
                        }
                if (cf > 2) {
                    itemchangedupdate_ = false;
                    si->setIcon(MODIFIEDSTATUS);
                    si->setData(si->data()); // hack to trigger display of data
                    if (model_->item(rf, 1) != NULL)
                        if ((model_->item(rf, 1)->icon() != DELETEDSTATUS) &&
                                model_->item(rf, 1)->icon() != INSERTEDSTATUS)
                            model_->item(rf, 1)->setIcon(MODIFIEDSTATUS);
                    itemchangedupdate_ = true;
                }
            }
}

void Ontology::OntSearchClicked() {
    /////
    actioncontainer_->clear();
    AssistedFieldValueForm * afvf = new AssistedFieldValueForm();
    afvf->done().connect(this, &Ontology::SearchAfvfComplete);
    actioncontainer_->addWidget(afvf);
    /////
    actioncontainer_->addWidget(new Wt::WBreak());
    Wt::WText * title2 = new Wt::WText("or formulate the full where clause yourself: ");
    title2->decorationStyle().font().setWeight(Wt::WFont::Bold);
    title2->decorationStyle().setForegroundColor(Wt::WColor(0, 68, 204));
    title2->setVerticalAlignment(Wt::AlignMiddle);
    actioncontainer_->addWidget(title2);
    Wt::WLineEdit * where = new Wt::WLineEdit();
    where->setText(Wt::WString("where "));
    where->setVerticalAlignment(Wt::AlignMiddle);
    where->enterPressed().connect(boost::bind(&Ontology::OntWhereFieldEnterPressed, this, where));
    actioncontainer_->addWidget(where);
    Wt::WAnchor * a = new Wt::WAnchor("http://www.postgresql.org/docs/9.3/static/queries-table-expressions.html#QUERIES-WHERE");
    Wt::WImage * im = new Wt::WImage("resources/icons/qmark15.png");
    im->setVerticalAlignment(Wt::AlignTop);
    a->setImage(im);
    a->setTarget(Wt::TargetNewWindow);
    a->setVerticalAlignment(Wt::AlignSuper);
    actioncontainer_->addWidget(a);
    /////
}

void Ontology::OntDeleteClicked() {
    actioncontainer_->clear();
    Wt::WText * title = new Wt::WText("Toggled Delete for checked rows.");
    title->decorationStyle().font().setWeight(Wt::WFont::Bold);
    title->decorationStyle().setForegroundColor(Wt::WColor(189, 54, 47));
    actioncontainer_->addWidget(title);
    for (int row = 0; row < model_->rowCount(); row++) {
        if (Wt::asString(model_->data(row, 0, Wt::CheckStateRole)) == "true")
            if (model_->item(row, 1)->icon() == DELETEDSTATUS)
                model_->item(row, 1)->setIcon("");
            else
                model_->item(row, 1)->setIcon(DELETEDSTATUS);
    }
}

void Ontology::OntInsertClicked() {
    actioncontainer_->clear();
    if (model_->rowCount() == 0) {
        TpOntApi * toa = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
        ot_->CreateTableWithOneEmptyLine(model_, toa);
        delete toa;
        resultcontainer_->show();
        int iowner;
        for (int ic = 3; ic < model_->columnCount(); ic++)
            if (Wt::asString(model_->headerData(ic)).toUTF8().compare("owner") == 0)
                iowner = ic;
        model_->setData(0, 0,
                boost::any(false), Wt::CheckStateRole);
        model_->item(0, 1)->setIcon(INSERTEDSTATUS);
        model_->item(0, 2)->setData(boost::any(), Wt::EditRole);
        if (!loginname_.empty()) {
            model_->setData(0, iowner, loginname_);
        } else
            model_->setData(0, iowner, Wt::WString("textpresso"));
        ontdelbutton_->enable();
        ontlexicalbutton_->enable();
        ontsnapshotbutton_->enable();
        ontcommitbutton_->enable();
    } else {
        Wt::WText * title = new Wt::WText("Inserted rows before checked rows.");
        title->decorationStyle().font().setWeight(Wt::WFont::Bold);
        title->decorationStyle().setForegroundColor(Wt::WColor(81, 163, 81));
        actioncontainer_->addWidget(title);
        std::vector<int> inserts;
        for (int row = 0; row < model_->rowCount(); row++)
            if (Wt::asString(model_->data(row, 0, Wt::CheckStateRole)) == "true")
                inserts.push_back(row);
        // find index of owner column
        int iowner;
        for (int ic = 3; ic < model_->columnCount(); ic++)
            if (Wt::asString(model_->headerData(ic)).toUTF8().compare("owner") == 0)
                iowner = ic;
        while (!inserts.empty()) {
            int row = inserts.back();
            inserts.pop_back();
            std::vector<Wt::WStandardItem*> rs;
            for (int ic = 0; ic < model_->columnCount(); ic++) {
                Wt::WStandardItem * si = new Wt::WStandardItem(*model_->item(row, ic));
                rs.push_back(si);
            }
            model_->insertRow(row, rs);
            model_->setData(row, 0,
                    boost::any(false), Wt::CheckStateRole);
            model_->item(row, 1)->setIcon(INSERTEDSTATUS);
            model_->item(row, 2)->setData(boost::any(), Wt::EditRole);
            if (!loginname_.empty()) {
                model_->setData(row, iowner, loginname_);
            } else
                model_->setData(row, iowner, Wt::WString("textpresso"));
        }
    }
    MakeModelUserEditable(loginname_, model_);
}

void Ontology::OntLexicalClicked() {
    int iterm = -1;
    int ilexical = -1;
    for (int i = 0; i < model_->columnCount(); i++) {
        std::string header = Wt::asString(model_->headerData(i)).toUTF8();
        if (header.compare("term") == 0) iterm = i;
        if (header.compare("lexicalvariations") == 0) ilexical = i;
    }
    std::vector<std::string> inp;
    for (int i = 0; i < model_->rowCount(); i++)
        inp.push_back(Wt::asString(model_->data(i, iterm)).toUTF8());
    LexicalVariations * lv = new LexicalVariations(inp);
    for (int i = 0; i < model_->rowCount(); i++)
        if (Wt::asString(model_->data(i, 0, Wt::CheckStateRole)) == "true")
            model_->setData(i, ilexical, boost::any(lv->GetForms(i)));
}

void Ontology::OntFilterClicked() {
    actioncontainer_->clear();
    AssistedFieldValueForm * afvf = new AssistedFieldValueForm(model_);
    afvf->done().connect(this, &Ontology::FilterAfvfComplete);
    actioncontainer_->addWidget(afvf);
}

void Ontology::OntCommitClicked() {
    // require all field in model to be non-empty
    bool error = false;
    itemchangedupdate_ = false;
    for (int i = 0; i < model_->rowCount(); i++) {
        std::string s = Wt::asString(model_->data(i, 1, Wt::DecorationRole)).toUTF8();
        if (s.compare(DELETEDSTATUS) != 0) {
            for (int j = 3; j < model_->columnCount(); j++)
                if (Wt::asString(model_->data(i, j)).empty()) {
                    error = true;
                    model_->item(i, j)->setIcon(ATTENTION);
                    model_->item(i, j)->setData(boost::any(), Wt::EditRole);
                }
        }
    }
    itemchangedupdate_ = true;
    if (error) {
        Wt::WText * text = new Wt::WText("Some fields are empty!");
        text->decorationStyle().font().setWeight(Wt::WFont::Bold);
        text->decorationStyle().setForegroundColor(Wt::WColor(200, 0, 0));
        actioncontainer_->clear();
        actioncontainer_->addWidget(text);
        return;
    }
    // work on deletions first
    TpOntApi * toad = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    for (int i = 0; i < model_->rowCount(); i++) {
        std::string s = Wt::asString(model_->data(i, 1, Wt::DecorationRole)).toUTF8();
        if (s.compare(DELETEDSTATUS) == 0) {
            TpOntEntry * t = new TpOntEntry();
            double iid = Wt::asNumber(model_->data(i, 2));
            t->SetIid(int(iid));
            toad->PushResult(t);
        }
    }
    toad->DeleteInDb();
    delete toad;
    // work on modifications and insertion next
    TpOntApi * toam = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    TpOntApi * toai = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    for (int i = 0; i < model_->rowCount(); i++) {
        std::string s = Wt::asString(model_->data(i, 1, Wt::DecorationRole)).toUTF8();
        if ((s.compare(MODIFIEDSTATUS) == 0) || (s.compare(INSERTEDSTATUS) == 0)) {
            TpOntEntry * t = new TpOntEntry();
            double iid = Wt::asNumber(model_->data(i, 2));
            t->SetIid(int(iid));
            for (int j = 3; j < model_->columnCount(); j++) {
                std::string aux = Wt::asString(model_->data(i, j)).toUTF8();
                // need to make sure that column headers in model_ are mapped
                // to column names in PGONTOLOGY correctly.
                std::string mheader = Wt::asString(model_->headerData(j)).toUTF8();
                t->PopulateColumn(mheader, aux);
            }
            if (s.compare(MODIFIEDSTATUS) == 0)
                toam->PushResult(t);
            else
                toai->PushResult(t);
        }
    }
    toam->UpdateDb();
    toai->WriteDb();
    delete toam;
    delete toai;
    actioncontainer_->clear();
    Wt::WText * text = new Wt::WText("Changes committed.");
    text->decorationStyle().font().setWeight(Wt::WFont::Bold);
    text->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 0));
    actioncontainer_->addWidget(text);
    model_->clear();
    resultcontainer_->hide();
    //    ontinsertbutton_->disable();
    ontdelbutton_->disable();
    ontsnapshotbutton_->disable();
    ontlexicalbutton_->disable();
    ontcommitbutton_->disable();
}

void Ontology::OntUploadClicked() {
    actioncontainer_->clear();

    Wt::WAnchor * a = new Wt::WAnchor("/resources/uploadtemplate.tsv");
    a->setText("Tab-delimited upload template.");
    a->setTarget(Wt::TargetNewWindow);
    actioncontainer_->addWidget(a);
    actioncontainer_->addWidget(new Wt::WBreak);

    Wt::WFileUpload * upload = new Wt::WFileUpload(actioncontainer_);
    upload->setFileTextSize(40);

    // Provide a button
    Wt::WPushButton * uploadButton = new Wt::WPushButton("Upload", actioncontainer_);

    // Upload when the button is clicked.
    uploadButton->clicked().connect(upload, &Wt::WFileUpload::upload);
    uploadButton->clicked().connect(uploadButton, &Wt::WPushButton::disable);

    // Upload automatically when the user entered a file.
    //upload->changed().connect(upload, &Wt::WFileUpload::upload);
    //upload->changed().connect(uploadButton, &Wt::WPushButton::disable);

    // React to a succesfull upload.
    upload->uploaded().connect(boost::bind(&Ontology::ProcessUploadedFile, this, upload));

    // React to a fileupload problem.
    upload->fileTooLarge().connect(this, &Ontology::UploadfileTooLarge);
}

void Ontology::OntSnapshotClicked() {
    actioncontainer_->clear();
    snapshotcount_ = (snapshotcount_ < 0) ? 0 : snapshotcount_;
    //    resultcontainer_->clear();
    std::stringstream snapshotid;
    snapshotid << "/tmp/";
    snapshotid << Wt::WApplication::instance()->sessionId();
    snapshotid << ".snapshot.";
    snapshotid << snapshotcount_++;
    snapshotid << ".model";
    OntSaveModel(snapshotid.str().c_str());
    actioncontainer_->addWidget(new Wt::WText("Snapshot added as "));
    actioncontainer_->addWidget(new Wt::WText(snapshotid.str()));
    ontreversebutton_->enable();
}

void Ontology::OntReverseClicked() {
    actioncontainer_->clear();
    std::stringstream snapshotid;
    snapshotid << "/tmp/";
    snapshotid << Wt::WApplication::instance()->sessionId();
    snapshotid << ".snapshot.";
    snapshotid << --snapshotcount_;
    snapshotid << ".model";
    if (snapshotcount_ == 0)
        ontreversebutton_->disable();
    OntLoadModel(snapshotid.str().c_str());
    actioncontainer_->addWidget(new Wt::WText("Reversed to "));
    actioncontainer_->addWidget(new Wt::WText(snapshotid.str()));
}

void Ontology::OntResetClicked() {
    actioncontainer_->clear();
    resultcontainer_->clear();
}

void Ontology::OntWhereFieldEnterPressed(Wt::WLineEdit * where) {
    Wt::WString w = where->text();
    actioncontainer_->clear();
    Wt::WString s;
    s = "SEARCH: where clause: " + w;
    Wt::WText * stext = new Wt::WText(s);
    stext->decorationStyle().font().setWeight(Wt::WFont::Bold);
    stext->decorationStyle().setForegroundColor(Wt::WColor(0, 68, 204));
    actioncontainer_->addWidget(stext);
    itemchangedupdate_ = false;
    TpOntApi * toa = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    toa->SearchDbWithWhereClause(w.toUTF8());
    model_->clear();
    if (toa->GetResultListSize() != 0) {
        resultcontainer_->show();
        //        ontinsertbutton_->enable();
        ontdelbutton_->enable();
        ontlexicalbutton_->enable();
        ontsnapshotbutton_->enable();
        ontcommitbutton_->enable();
        ot_->PopulateModelFromPg(model_, toa);
        MakeModelUserEditable(loginname_, model_);
    } else
        resultcontainer_->hide();
    delete toa;
    itemchangedupdate_ = true;
}

void Ontology::SearchAfvfComplete(Wt::WString fieldname, Wt::WString value) {
    actioncontainer_->clear();
    Wt::WString s;
    s = "SEARCH: column: " + fieldname + ", term: " + value;
    Wt::WText * stext = new Wt::WText(s);
    stext->decorationStyle().font().setWeight(Wt::WFont::Bold);
    stext->decorationStyle().setForegroundColor(Wt::WColor(0, 68, 204));
    actioncontainer_->addWidget(stext);
    itemchangedupdate_ = false;
    TpOntApi * toa = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    toa->SearchDbString(fieldname.toUTF8(), value.toUTF8());
    model_->clear();
    std::cerr << toa->GetResultListSize() << " results returned." << std::endl;
    if (toa->GetResultListSize() != 0) {
        resultcontainer_->show();
        //        ontinsertbutton_->enable();
        ontdelbutton_->enable();
        ontlexicalbutton_->enable();
        ontsnapshotbutton_->enable();
        ontcommitbutton_->enable();
        ot_->PopulateModelFromPg(model_, toa);
        MakeModelUserEditable(loginname_, model_);
    } else
        resultcontainer_->hide();
    delete toa;
    itemchangedupdate_ = true;
}

void Ontology::FilterAfvfComplete(Wt::WString fieldname, Wt::WString value) {
    actioncontainer_->clear();
    Wt::WString s;
    s = "Filter: column: " + fieldname + ", term: " + value;
    Wt::WText * stext = new Wt::WText(s);
    stext->decorationStyle().font().setWeight(Wt::WFont::Bold);
    stext->decorationStyle().setForegroundColor(Wt::WColor(248, 148, 6));
    actioncontainer_->addWidget(stext);
    itemchangedupdate_ = false;
    int affectedcol = -1;
    for (int i = 0; (i < model_->columnCount()) && (affectedcol < 0); i++) {
        std::string aux = Wt::asString(model_->headerData(i)).toUTF8();
        if (fieldname.toUTF8().compare(aux) == 0) affectedcol = i;
    }
    std::cerr << "Affected col = " << affectedcol << std::endl;
    std::string keyword = value.toUTF8();
    Wt::WStandardItemModel * newmodel = new Wt::WStandardItemModel(0, 0);
    for (int i = 0; i < model_->rowCount(); i++) {
        std::string aux = Wt::asString(model_->data(i, affectedcol, Wt::DisplayRole)).toUTF8();
        if (keyword.compare(aux) != 0) {
            std::vector<Wt::WStandardItem*> tbins;
            for (int j = 0; j < model_->columnCount(); j++) {
                Wt::WStandardItem * siaux = new Wt::WStandardItem(*model_->item(i, j));
                tbins.push_back(siaux);
            }
            newmodel->appendRow(tbins);
        }
    }
    model_->removeRows(0, model_->rowCount());
    for (int i = 0; i < newmodel->rowCount(); i++) {
        std::vector<Wt::WStandardItem*> tbins;
        for (int j = 0; j < newmodel->columnCount(); j++)
            tbins.push_back(newmodel->item(i, j));
        model_->appendRow(tbins);
    }
    itemchangedupdate_ = true;
}

void Ontology::OntLoadModel(const char * filename) {
    itemchangedupdate_ = false;
    std::ifstream f(filename);
    std::string in;
    getline(f, in);
    std::cerr << in << std::endl;
    std::vector<std::string> splits;
    boost::split(splits, in, boost::is_any_of("\t"));
    // splits contains header data
    model_->clear();
    int col = splits.size();
    model_->insertColumns(0, col);
    while (!splits.empty()) {
        model_->setHeaderData(--col, boost::any(splits.back()));
        splits.pop_back();
    }
    int row = 0;
    while (getline(f, in)) {
        std::vector<std::string> splits;
        boost::split(splits, in, boost::is_any_of("\t"));
        if (!splits.empty()) model_->insertRow(model_->rowCount());
        int col = model_->columnCount();
        while (!splits.empty()) {
            int role;
            if (col > 2)
                role = Wt::EditRole;
            else if (col == 2)
                role = Wt::DecorationRole;
            else if (col == 1)
                // this is wrong.
                role == Wt::CheckStateRole;
            model_->setData(row, --col, boost::any(splits.back()), role);
            if (col == 0) {
                model_->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
                model_->item(row, col)->setChecked(splits.back().compare("true") == 0);
            } else if (col > 2)
                model_->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsEditable);
            splits.pop_back();
        }
        row++;
    }
    f.close();
    ot_->SizeColumns(model_);
    itemchangedupdate_ = true;
}

void Ontology::OntSaveModel(const char * filename) {
    std::ofstream f(filename);
    f << Wt::asString(model_->headerData(0));
    for (int col = 1; col < model_->columnCount(); col++)
        f << "\t" << Wt::asString(model_->headerData(col));
    f << std::endl;
    for (int row = 0; row < model_->rowCount(); row++) {
        f << Wt::asString(model_->data(row, 0, Wt::CheckStateRole));
        f << "\t";
        f << Wt::asString(model_->data(row, 1, Wt::DecorationRole));
        for (int col = 2; col < model_->columnCount(); col++) {
            f << "\t";
            f << Wt::asString(model_->data(row, col));
        }
        f << std::endl;
    }
    f.close();
}

void Ontology::UploadfileTooLarge() {
    Wt::WText * warning = new Wt::WText("File is too large!");
    warning->decorationStyle().setForegroundColor(Wt::WColor(255, 0, 0));
    actioncontainer_->clear();
    actioncontainer_->addWidget(warning);
}

void Ontology::ProcessUploadedFile(Wt::WFileUpload * upload) {
    Wt::WString clfn(upload->clientFileName());
    std::string spfn(upload->spoolFileName());
    upload->stealSpooledFile();

    itemchangedupdate_ = false;
    std::ifstream f(spfn.c_str());
    std::string in;
    getline(f, in);
    std::cerr << in << std::endl;
    std::vector<std::string> splits;
    boost::split(splits, in, boost::is_any_of("\t"));
    // splits contains header data
    model_->clear();
    int col = splits.size() + 3;
    model_->insertColumns(0, col);
    while (!splits.empty()) {
        model_->setHeaderData(--col, boost::any(splits.back()));
        splits.pop_back();
    }
    model_->setHeaderData(2, boost::any(std::string("iid")));
    model_->setHeaderData(1, boost::any(std::string("status")));
    model_->setHeaderData(0, boost::any(std::string("select")));
    // find owner column
    int iowner;
    for (int ic = 3; ic < model_->columnCount(); ic++)
        if (Wt::asString(model_->headerData(ic)).toUTF8().compare("owner") == 0)
            iowner = ic;
    int row = 0;
    while (getline(f, in)) {
        std::vector<std::string> splits;
        boost::split(splits, in, boost::is_any_of("\t"));
        if (!splits.empty()) model_->insertRow(model_->rowCount());
        int col = model_->columnCount();
        while (!splits.empty()) {
            model_->setData(row, --col, boost::any(splits.back()), Wt::EditRole);
            model_->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsEditable);
            splits.pop_back();
        }
        model_->setData(row, 2, boost::any());
        //        model_->setData(row, 2, boost::any(-1));
        model_->setData(row, 1, boost::any(std::string(INSERTEDSTATUS)), Wt::DecorationRole);
        model_->setData(row, 0, boost::any(false), Wt::CheckStateRole);
        model_->item(row, 0)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
        if (!loginname_.empty()) {
            model_->setData(row, iowner, loginname_);
        } else
            model_->setData(row, iowner, Wt::WString("textpresso"));
        row++;
    }
    f.close();
    ot_->SizeColumns(model_);
    itemchangedupdate_ = false;
    resultcontainer_->show();
    //    ontinsertbutton_->enable();
    ontdelbutton_->enable();
    ontlexicalbutton_->enable();
    ontsnapshotbutton_->enable();
    ontcommitbutton_->enable();
    actioncontainer_->clear();
    actioncontainer_->addWidget(new Wt::WText(clfn));
    actioncontainer_->addWidget(new Wt::WText(" uploaded."));
}

void Ontology::AuthEvent(Session * session) {
    if (session->login().state() == 0) {
        std::stringstream s;
        s << "You need to login so ownership of all new entries you create ";
        s << "will be assigned to you. Otherwise they will be owned by Textpresso.";
        message_->setText(s.str());
        message_->decorationStyle().setForegroundColor(Wt::darkRed);
        loginname_ = Wt::WString::Empty;
    } else {
        message_->setText("");
        loginname_ = session->login().user().identity("loginname");
    }
}
