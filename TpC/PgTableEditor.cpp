/* 
 * File:   PgTableEditor.cpp
 * Author: mueller
 * 
 * Created on March 21, 2014, 2:36 PM
 */

#include "PgTableEditor.h"
#include "TpCurApi.h"

#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
#include <Wt/WTableView>
#include <Wt/WTextArea>
#include <Wt/WCssDecorationStyle>
#include <Wt/WColor>
#include <Wt/WApplication>
#include <Wt/WCheckBox>

#include <boost/algorithm/string.hpp>

namespace {

    std::string CleanWhiteSpaces(std::string inp) {
        std::string out = boost::replace_all_copy(inp, "\r", " ");
        boost::replace_all(out, "\n", " ");
        boost::replace_all(out, "\t", " ");
        while (out.find("  ") != std::string::npos) boost::replace_all(out, "  ", " ");
        return out;
    }
}

PgTableEditor::PgTableEditor(Session & session, std::string databasename,
        std::string tablename, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    //
    session_ = &session;
    snapshotcount_ = 0;
    modelitemchangedupdate_ = false;
    dialog_ = NULL;
    //
    containers_.editing = new Wt::WContainerWidget();
    containers_.table = new Wt::WContainerWidget();
    containers_.message = new Wt::WContainerWidget();
    //
    pgb_ = new PgTableEditorButtons();
    pgb_->hide();
    pgb_->InsertClicked().connect(this, &PgTableEditor::PgbInsertClicked);
    pgb_->DeleteClicked().connect(this, &PgTableEditor::PgbDeleteClicked);
    pgb_->SnapshotClicked().connect(this, &PgTableEditor::PgbSnapshotClicked2);
    pgb_->ReverseClicked().connect(this, &PgTableEditor::PgbReverseClicked2);
    pgb_->CommitClicked().connect(this, &PgTableEditor::PgbCommitClicked);
    pgb_->DisableDeleteButton();
    pgb_->DisableSnapshotButton();
    pgb_->DisableReverseButton();
    pgb_->DisableCommitButton();
    pgq_ = new PgTableQuery(databasename, tablename);
    pgq_->Done().connect(this, &PgTableEditor::PgTableQueryDone);
    //
    selectall_ = new Wt::WCheckBox("Select All");
    selectall_->setChecked(false);
    selectall_->hide();
    selectall_->clicked().connect(this, &PgTableEditor::SelectAllClicked);
    model_ = new Wt::WStandardItemModel(0, 0);
    table_ = new Wt::WTableView();
    table_->hide();
    table_->headerClicked().connect(this, &PgTableEditor::HeaderClicked);
    table_->clicked().connect(this, &PgTableEditor::TableClicked);
    SetTable();
    SizeColumns();
    table_->setModel(model_);
    containers_.table->addWidget(selectall_);
    containers_.table->addWidget(table_);
    //
    message_.text = new Wt::WText(" Please start by searching the curation table in the query section.");
    message_.text->setInline(true);
    message_.icon = new Wt::WImage(INFO16);
    message_.icon->setInline(true);
    containers_.message->addWidget(message_.icon);
    containers_.message->addWidget(message_.text);
    //
    addWidget(containers_.message);
    addWidget(pgq_);
    addWidget(containers_.editing);
    addWidget(pgb_);
    addWidget(containers_.table);
}

void PgTableEditor::PopulateModelHeaders() {
    std::vector<std::string> headers = pgq_->GetColumnHeaders();
    model_->insertColumns(0, headers.size() + 2);
    headerclicks_.clear();
    int col = 0;
    model_->setHeaderData(col++, boost::any(std::string("select")));
    model_->setHeaderData(col++, boost::any(std::string("status")));
    while (!headers.empty()) {
        model_->setHeaderData(col++, boost::any(headers.back()));
        headers.pop_back();
        headerclicks_.push_back(0);
    }
}

void PgTableEditor::SetTable() {
    table_->setSortingEnabled(true);
    table_->setAlternatingRowColors(true);
    table_->setRowHeight(TABLECELLHEIGHT);
    table_->decorationStyle().setBackgroundColor(Wt::WColor(222, 222, 255));
    table_->setSelectable(true);
    table_->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
}

void PgTableEditor::SizeColumns() {
    table_->setColumnResizeEnabled(true);
    table_->setColumnWidth(0, Wt::WLength(9, Wt::WLength::FontEx));
    table_->setColumnWidth(1, Wt::WLength(9, Wt::WLength::FontEx));
}

PgTableEditor::~PgTableEditor() {
    delete message_.text;
    delete message_.icon;
    delete containers_.message;
    delete containers_.table;
    delete containers_.editing;
}

void PgTableEditor::PopulateModelFromPg() {
    if (pgq_->GetResults().size() > 0) {
        modelitemchangedupdate_ = false;
        selectall_->show();
        model_->clear();
        PopulateModelHeaders();
        model_->insertRows(0, pgq_->GetResults().size());
        for (int rownum = 0; rownum < pgq_->GetResults().size(); ++rownum) {
            model_->setData(rownum, 0, boost::any(false), Wt::CheckStateRole);
            model_->setData(rownum, 1, boost::any(), Wt::DecorationRole);
            const pqxx::result::tuple row = pgq_->GetResults()[rownum];
            for (int colnum = 0; colnum < row.size(); ++colnum) {
                const pqxx::result::field field = row[colnum];
                std::string mheader = Wt::asString(model_->headerData(colnum + 2)).toUTF8();
                if (mheader.compare("last_update") == 0) {
                    char buff[20];
                    time_t t;
                    field.to(t);
                    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
                    model_->setData(rownum, colnum + 2, boost::any(std::string(buff)));
                } else {
                    std::string saux;
                    field.to(saux);
                    model_->setData(rownum, colnum + 2, boost::any(saux));
                }
            }
        }
        if (model_->rowCount() > 0) pgb_->EnableSnapshotButton();
        selectall_->setChecked(false);
        pgb_->DisableDeleteButton();
        pgb_->DisableCommitButton();
        model_->itemChanged().connect(this, &PgTableEditor::ModelItemChanged);
        modelitemchangedupdate_ = true;
        table_->show();
    }
}

void PgTableEditor::MakeModelUserEditable() {
    for (int ir = 0; ir < model_->rowCount(); ir++)
        if (model_->item(ir, 0) != NULL)
            model_->item(ir, 0)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
}

void PgTableEditor::ModelItemChanged(Wt::WStandardItem * s) {
    if (modelitemchangedupdate_)
        if (s != NULL)
            if (s->column() == 0) {
                bool anychecked = false;
                for (int ir = 0; ir < model_->rowCount(); ir++) {
                    if (model_->item(ir, 0)->isChecked()) anychecked = true;
                    if (anychecked) break;
                }
                if (anychecked)
                    pgb_->EnableDeleteButton();
                else
                    pgb_->DisableDeleteButton();
            } else {
                pgb_->EnableCommitButton();
            }
}

void PgTableEditor::PgTableQueryDone(long unsigned int i) {
    std::stringstream s;
    s << " " << i << " result(s) returned.";
    message_.text->setText(s.str());
    if (i == 0) {
        message_.icon->setImageRef(ATTENTION);
        model_->clear();
        pgb_->hide();
        containers_.table->hide();
    } else {
        message_.icon->setImageRef(OK);
        PopulateModelFromPg();
        MakeModelUserEditable();
        pgb_->show();
    }
}

void PgTableEditor::HeaderClicked(int i) {
    if (headerclicks_[i] == 2) {
        table_->setColumnWidth(i, Wt::WLength(1, Wt::WLength::FontEx));
        table_->setSortingEnabled(i, false);
    } else if (headerclicks_[i] == 3) {
        headerclicks_[i] = 0;
        table_->setColumnWidth(i, Wt::WLength(20, Wt::WLength::FontEx));
        table_->setSortingEnabled(i, true);
    }
    headerclicks_[i]++;
}

void PgTableEditor::TableClicked(Wt::WModelIndex index, Wt::WMouseEvent event) {
    int ic = index.column();
    if (ic > 2) {
        if (dialog_ == NULL) {
            dialog_ = new Wt::WDialog("Table Cell Editor");
            dialog_->setModal(false);
            dialog_->setResizable(true);
            dialog_->setClosable(true);
            dialog_->setWidth(Wt::WLength(35, Wt::WLength::FontEx));
            Wt::WPushButton * ok = new Wt::WPushButton("Ok");
            ok->clicked().connect(dialog_, &Wt::WDialog::accept);
            if ((Wt::asString(model_->headerData(ic)) == "position_in_sofastring") ||
                    (Wt::asString(model_->headerData(ic)) == "term")) {
                containers_.editing->clear();
                Wt::WText * notloggedinnote = new Wt::WText();
                notloggedinnote->setText("This column cannot be edited.");
                dialog_->contents()->decorationStyle().setBackgroundColor(Wt::WColor(255, 221, 34));
                containers_.editing->addWidget(notloggedinnote);
            } else if (session_->login().state() != 0) {
                Wt::WString loginname("");
                loginname = session_->login().user().identity("loginname");
                int iowner;
                for (int ic = 3; ic < model_->columnCount(); ic++)
                    if (Wt::asString(model_->headerData(ic)).toUTF8().compare("owner") == 0)
                        iowner = ic;
                int ir = index.row();
                if (model_->item(ir, iowner) != NULL)
                    if (model_->item(ir, iowner)->text() == loginname) {
                        Wt::WTextArea * t = new Wt::WTextArea();
                        containers_.editing->clear();
                        containers_.editing->addWidget(t);
                        t->setText(Wt::asString(index.data(Wt::EditRole), "%s"));
                        ok->clicked().connect(boost::bind(&PgTableEditor::EditingEnterPressed, this, index, t));
                        t->enterPressed().connect(boost::bind(&PgTableEditor::EditingEnterPressed, this, index, t));
                        t->escapePressed().connect(boost::bind(&PgTableEditor::EditingEscapePressed, this, index, t));
                        t->enterPressed().connect(dialog_, &Wt::WDialog::accept);
                        t->escapePressed().connect(dialog_, &Wt::WDialog::reject);
                        t->setFocus(true);
                    } else {
                        containers_.editing->clear();
                        Wt::WText * notloggedinnote = new Wt::WText();
                        notloggedinnote->setText("You can only edit entries that you own.");
                        dialog_->contents()->decorationStyle().setBackgroundColor(Wt::WColor(255, 221, 34));
                        containers_.editing->addWidget(notloggedinnote);
                    }
            } else {
                containers_.editing->clear();
                Wt::WText * notloggedinnote = new Wt::WText();
                notloggedinnote->setText("You need to be logged in to be able to edit.");
                dialog_->contents()->decorationStyle().setBackgroundColor(Wt::WColor(255, 221, 34));
                containers_.editing->addWidget(notloggedinnote);
            }
            dialog_->contents()->addWidget(containers_.editing);
            dialog_->contents()->addWidget(new Wt::WBreak());
            dialog_->contents()->addWidget(ok);
            dialog_->finished().connect(this, &PgTableEditor::DialogDone);
            dialog_->show();
        }
    }
}

void PgTableEditor::DialogDone(Wt::WDialog::DialogCode code) {

    dialog_->contents()->removeWidget(containers_.editing);
    delete dialog_;
    dialog_ = NULL;
}

void PgTableEditor::EditingEnterPressed(Wt::WModelIndex index, Wt::WTextArea * t) {
    Wt::WStandardItem * s = model_->itemFromIndex(index);
    s->setIcon(MODIFIEDSTATUS);
    std::string status = Wt::asString(model_->data(index, Wt::DecorationRole)).toUTF8();
    if (status.compare(DELETEDSTATUS) != 0) {

        int r = s->row();
        s = model_->item(r, 1);
        s->setIcon(MODIFIEDSTATUS);
    }
    model_->setData(index, boost::any(t->text()), Wt::EditRole);
    table_->closeEditor(index, false);
    pgb_->EnableCommitButton();
    delete(t);
}

void PgTableEditor::EditingEscapePressed(Wt::WModelIndex index, Wt::WTextArea * t) {

    table_->closeEditor(index, false);
    delete(t);
}

void PgTableEditor::SelectAllClicked() {
    if (model_ != NULL) {
        bool state = selectall_->isChecked();
        for (int i = 0; i < model_->rowCount(); i++)
            model_->item(i, 0)->setChecked(state);
        if (state)
            pgb_->EnableDeleteButton();

        else
            pgb_->DisableDeleteButton();
    }
}

void PgTableEditor::PgbInsertClicked() {
    std::vector<int> inserts;
    for (int row = 0; row < model_->rowCount(); row++)
        if (Wt::asString(model_->data(row, 0, Wt::CheckStateRole)) == "true")
            inserts.push_back(row);
    if (!inserts.empty()) {
        message_.text->setText("Inserted rows before checked rows.");
        message_.icon->setImageRef(INFO16);
    } else {
        message_.text->setText("No rows inserted as there were no rows checked!");
        message_.icon->setImageRef(ATTENTION);
    }
    // find index of owner column
    int iowner;
    for (int ic = 3; ic < model_->columnCount(); ic++)
        if (Wt::asString(model_->headerData(ic)).toUTF8().compare("owner") == 0)
            iowner = ic;
    // find loginname
    Wt::WString loginname("");
    loginname = session_->login().user().identity("loginname");
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
        if (!loginname.empty()) {
            model_->setData(row, iowner, loginname);
        } else
            model_->setData(row, iowner, Wt::WString("textpresso"));
    }
    MakeModelUserEditable();
}

void PgTableEditor::PgbDeleteClicked() {
    if (model_ != NULL) {
        for (int ir = 0; ir < model_->rowCount(); ir++) {
            if (model_->item(ir, 0)->isChecked()) {
                std::string status = Wt::asString(model_->data(ir, 1, Wt::DecorationRole)).toUTF8();
                if (status.compare(DELETEDSTATUS) != 0) {
                    model_->item(ir, 1)->setIcon(DELETEDSTATUS);
                } else {
                    model_->item(ir, 1)->setIcon("");
                    bool modified = false;
                    for (int ic = 3; ic < model_->columnCount(); ic++) {
                        std::string s = Wt::asString(model_->data(ir, ic, Wt::DecorationRole)).toUTF8();
                        modified = (s.compare(MODIFIEDSTATUS) == 0);
                        if (modified) break;
                    }

                    if (modified) model_->item(ir, 1)->setIcon(MODIFIEDSTATUS);
                }
            }
        }
    }
}

void PgTableEditor::PgbSnapshotClicked2() {
    snapshotcount_ = (snapshotcount_ < 0) ? 0 : snapshotcount_;
    std::stringstream snapshotid;
    snapshotid << "/tmp/";
    snapshotid << Wt::WApplication::instance()->sessionId();
    snapshotid << ".snapshot.";
    snapshotid << snapshotcount_++;
    snapshotid << ".model";
    std::ofstream f(snapshotid.str().c_str());
    f << Wt::asString(model_->headerData(0));
    for (int col = 1; col < model_->columnCount(); col++)
        f << "\t" << Wt::asString(model_->headerData(col));
    f << std::endl;
    for (int row = 0; row < model_->rowCount(); row++) {
        f << Wt::asString(model_->data(row, 0, 0)); // DisplayRole
        f << "\t";
        f << Wt::asString(model_->data(row, 0, 1)); // DecorationRole
        f << "\t";
        f << Wt::asString(model_->data(row, 0, 4)); // CheckStateRole
        for (int col = 1; col < model_->columnCount(); col++) {
            f << "\t";
            f << Wt::asString(model_->data(row, col, 0)); // DisplayRole
            f << "\t";
            f << Wt::asString(model_->data(row, col, 1)); // DecorationRole
            f << "\t";
            f << Wt::asString(model_->data(row, col, 4)); // CheckStateRole
        }
        f << std::endl;
    }
    f.close();
    message_.text->setText(" Snapshot added as " + Wt::WString(snapshotid.str()));
    pgb_->EnableReverseButton();
}

void PgTableEditor::PgbSnapshotClicked() {
    snapshotcount_ = (snapshotcount_ < 0) ? 0 : snapshotcount_;
    std::stringstream snapshotid;
    snapshotid << "/tmp/";
    snapshotid << Wt::WApplication::instance()->sessionId();
    snapshotid << ".snapshot.";
    snapshotid << snapshotcount_++;
    snapshotid << ".model";
    std::ofstream f(snapshotid.str().c_str());
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
    message_.text->setText(" Snapshot added as " + Wt::WString(snapshotid.str()));
    pgb_->EnableReverseButton();
}

void PgTableEditor::PgbReverseClicked2() {
    modelitemchangedupdate_ = false;
    std::stringstream snapshotid;
    snapshotid << "/tmp/";
    snapshotid << Wt::WApplication::instance()->sessionId();
    snapshotid << ".snapshot.";
    snapshotid << --snapshotcount_;
    snapshotid << ".model";
    if (snapshotcount_ == 0) pgb_->DisableReverseButton();
    std::ifstream f(snapshotid.str().c_str());
    std::string in;
    getline(f, in);
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
        int col = model_->columnCount();
        if (!splits.empty()) model_->insertRow(model_->rowCount());
        while (!splits.empty()) {
            col--;
            model_->setData(row, col, boost::any());
            if (col == 0) {
                model_->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
                model_->item(row, col)->setChecked(splits.back().compare("true") == 0);
            }
            splits.pop_back();
            model_->setData(row, col, boost::any(splits.back()), 1);
            splits.pop_back();
            model_->setData(row, col, boost::any(splits.back()), 0);
            splits.pop_back();
            //            if (col == 0) {
            //                model_->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
            //                model_->item(row, col)->setChecked(splits.back().compare("true") == 0);
            //                model_->item(row, col)->setIcon("");
            //            }
        }
        row++;
    }
    f.close();
    MakeModelUserEditable();
    SizeColumns();
    message_.text->setText(" Reversed to " + Wt::WString(snapshotid.str()));
    modelitemchangedupdate_ = true;
}

void PgTableEditor::PgbReverseClicked() {
    modelitemchangedupdate_ = false;
    std::stringstream snapshotid;
    snapshotid << "/tmp/";
    snapshotid << Wt::WApplication::instance()->sessionId();
    snapshotid << ".snapshot.";
    snapshotid << --snapshotcount_;
    snapshotid << ".model";
    if (snapshotcount_ == 0) pgb_->DisableReverseButton();
    std::ifstream f(snapshotid.str().c_str());
    std::string in;
    getline(f, in);
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
                role == Wt::CheckStateRole;
            model_->setData(row, --col, boost::any(splits.back()), role);
            if (col == 0) {

                model_->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
                model_->item(row, col)->setChecked(splits.back().compare("true") == 0);
                model_->item(row, col)->setIcon("");
            }
            splits.pop_back();
        }
        row++;
    }
    f.close();
    SizeColumns();
    message_.text->setText(" Reversed to " + Wt::WString(snapshotid.str()));
    modelitemchangedupdate_ = true;
}

void PgTableEditor::PgbCommitClicked() {
    int iid;
    std::vector<std::string> deletionlist;
    for (int ic = 0; ic < model_->columnCount(); ic++)
        if (Wt::asString(model_->headerData(ic)) == "iid") iid = ic;
    // work on deletions first
    for (int ir = 0; ir < model_->rowCount(); ir++) {
        std::string s = Wt::asString(model_->data(ir, 1, Wt::DecorationRole)).toUTF8();
        if (s.compare(DELETEDSTATUS) == 0)
            deletionlist.push_back(Wt::asString(model_->data(ir, iid)).toUTF8());
    }
    TpCurApi * tca = new TpCurApi();
    tca->DeleteInPg(deletionlist);
    delete (tca);
    // work on insertions and modifications next
    for (int ir = 0; ir < model_->rowCount(); ir++) {
        std::string s = Wt::asString(model_->data(ir, 1, Wt::DecorationRole)).toUTF8();
        if ((s.compare(MODIFIEDSTATUS) == 0) || (s.compare(INSERTEDSTATUS) == 0)) {
            std::string curiid = Wt::asString(model_->data(ir, iid)).toUTF8();
            TpCurApi * tca = new TpCurApi();
            TpCurApi::dataentry d;
            for (int ic = 3; ic < model_->columnCount(); ic++) {
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "biological_entities")
                    d.biological_entities = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "comment")
                    d.comment = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "curation_status")
                    d.curation_status = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "curation_use")
                    d.curation_use = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "dbxref")
                    d.dbxref = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "eid")
                    d.eid = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "evidence_code")
                    d.evidence_code = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "file_location")
                    d.file_location = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                d.last_update = time(0);
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "ontology_terms")
                    d.ontology_terms = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "owner")
                    d.owner = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "paper_id")
                    d.paper_id = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "position_in_sofastring")
                    d.position_in_sofastring = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "source")
                    d.source = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "term")
                    d.term = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
                if (Wt::asString(model_->headerData(ic)).toUTF8() == "version")
                    d.version = CleanWhiteSpaces(Wt::asString(model_->data(ir, ic)).toUTF8());
            }
            TpCurApi * tpca = new TpCurApi();
            if (s.compare(MODIFIEDSTATUS) == 0)
                tpca->UpdatePg(curiid, d);
            else
                tpca->SubmitRow(d);
            delete (tpca);
        }
    }
    message_.icon->setImageRef(INFO16);
    message_.text->setText(" Changes committed.");
    pgb_->DisableCommitButton();
    pgq_->SearchPg();
    PopulateModelFromPg();
    MakeModelUserEditable();
}