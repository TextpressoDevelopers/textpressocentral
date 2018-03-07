/* 
 * File:   OntologyDisplay.cpp
 * Author: mueller
 * 
 * Created on July 17, 2017, 2:48 PM
 */

#include <Wt/WContainerWidget>
#include "TextpressoCentralGlobalDefinitions.h"
#include "OntologyDisplay.h"
#include "PgList.h"
#include <Wt/WScrollArea>
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <Wt/WComboBox>
#include <boost/algorithm/string/predicate.hpp>

#define CELLHEIGHT 2
#define CELLWIDTH 20

OntologyDisplay::OntologyDisplay(PickCategoryContainer *pcc,
        OntologyTermQuery *otc, Wt::WLength h, Wt::WContainerWidget *parent)
: Wt::WContainerWidget(parent) {
    ftc_ = new Wt::WContainerWidget();
    ftc_->hide();
    Wt::WText * ftlabel = new Wt::WText("Filter: ");
    ftcolname_ = new Wt::WComboBox();
    PgList * allowed = new PgList(PGONTOLOGYBROWSER, PGONTOLOGYBROWSWERCOLUMNS);
    if (!allowed->HasList()) {
        std::set<std::string> defaults;
        defaults.insert("eid");
        defaults.insert("term");
        defaults.insert("category");
        allowed->SaveList(defaults);
    }
    for (auto colname : allowed->GetList())
        ftcolname_->addItem((Wt::WString(colname)));
    delete allowed;
    ftvalue_ = new Wt::WLineEdit();
    ftlabel->setInline(true);
    ftcolname_->setInline(true);
    ftvalue_->setInline(true);
    ftc_->addWidget(ftlabel);
    ftc_->addWidget(ftcolname_);
    ftc_->addWidget(ftvalue_);
    addWidget(ftc_);
    Wt::WScrollArea * cs = new Wt::WScrollArea();
    cs->setHeight(h);
    cs->setScrollBarPolicy(Wt::WScrollArea::ScrollBarAsNeeded);
    addWidget(cs);
    csc_ = new Wt::WContainerWidget();
    cs->setWidget(csc_);
    toa_ = new TpOntApi(PGONTOLOGYTABLENAME,
            PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    pcc->OkClicked().connect(boost::bind(&OntologyDisplay::SelectionOkClicked, this, pcc));
    otc->TermEntered().connect(boost::bind(&OntologyDisplay::SearchTermEntered, this, otc));
}

OntologyDisplay::OntologyDisplay(const OntologyDisplay& orig) {
}

OntologyDisplay::~OntologyDisplay() {
    delete toa_;
}

void OntologyDisplay::SelectionOkClicked(PickCategoryContainer *pcc) {
    PopulateTable(pcc, "", "");
    ftvalue_->setText("");
    ftc_->show();
    ftvalue_->keyWentUp().connect(std::bind([ = ](){
        PopulateTable(pcc, ftcolname_->currentText().toUTF8(),
        ftvalue_->text().toUTF8());
    }));

}

void OntologyDisplay::SearchTermEntered(OntologyTermQuery *otc) {
    ftc_->hide();
    csc_->clear();
    statusline_ = new Wt::WText();
    csc_->addWidget(statusline_);
    std::vector<std::string> headers(toa_->GetColumnHeaders());
    toa_->DeleteAllResults();
    std::string where =  "where term ~ '\\m" + (otc->GetTerm()).toUTF8() + "\\M'";
    toa_->SearchDbWithWhereClause(where);
    Wt::WStandardItemModel * model = new Wt::WStandardItemModel(0, 0);
    Wt::WTableView * table(CreateTableHeader(model, headers));
    AddToModel(model, toa_->GetResultList(), "", "");
    csc_->addWidget(table);
}

void OntologyDisplay::PopulateTable(PickCategoryContainer *pcc,
        std::string ftcolname, std::string ftvalue) {
    csc_->clear();
    statusline_ = new Wt::WText();
    csc_->addWidget(statusline_);
    std::set<Wt::WString> selected(pcc->GetSelected(true));
    if (selected.size() > 15) {
        statusline_->setText("Too many categories. Reduce your selection.");
        statusline_->decorationStyle().setForegroundColor(Wt::red);
    } else {
        statusline_->setText("");
        std::vector<std::string> headers(toa_->GetColumnHeaders());
        Wt::WStandardItemModel * model = new Wt::WStandardItemModel(0, 0);
        Wt::WTableView * table(CreateTableHeader(model, headers));
        for (auto cat : selected) {
            toa_->DeleteAllResults();
            toa_->SearchDbString(TpOntApi::category, cat.toUTF8());
            AddToModel(model, toa_->GetResultList(), ftcolname, ftvalue);
        }
        csc_->addWidget(table);
    }
}

Wt::WTableView * OntologyDisplay::CreateTableHeader(Wt::WStandardItemModel * model,
        const std::vector<std::string> & headers) {
    PgList * allowed = new PgList(PGONTOLOGYBROWSER, PGONTOLOGYBROWSWERCOLUMNS);
    std::vector<std::string> allowedheaders;
    for (auto colname : headers)
        if (allowed->IsInList(colname))
            allowedheaders.push_back(colname);
    delete allowed;
    Wt::WTableView * ret = new Wt::WTableView();
    ret->setSortingEnabled(true);
    ret->setAlternatingRowColors(true);
    ret->setRowHeight(Wt::WLength(CELLHEIGHT, Wt::WLength::FontEm));
    ret->decorationStyle().setBackgroundColor(Wt::WColor(200, 200, 255));
    ret->setColumnResizeEnabled(true);
    //
    if (allowedheaders.size() > 0) {
        model->insertColumns(0, allowedheaders.size());
        int col = 0;
        for (auto colname : allowedheaders)
            model->setHeaderData(col++, boost::any(colname));
    }
    ret->setModel(model);
    double tablewidth = 120.;
    double colwidth = tablewidth / double(model->columnCount());
    for (int i = 0; i < model->columnCount(); i++)
        ret->setColumnWidth(i, Wt::WLength(colwidth, Wt::WLength::FontEm));
    ret->setWidth(Wt::WLength(tablewidth, Wt::WLength::FontEm));
    return ret;
}

void OntologyDisplay::AddToModel(Wt::WStandardItemModel * model,
        std::vector<TpOntEntry*> items,
        std::string ftcolname, std::string ftvalue) {
    std::vector<TpOntEntry*> filtered;
    if (ftvalue.empty())
        filtered = items;
    else
        for (int r = 0; r < items.size(); r++) {
            std::string v(items[r]->
                    GetColumn(boost::any_cast<std::string>(ftcolname)));
            if (boost::starts_with(v, ftvalue)) filtered.push_back(items[r]);
        }
    if (model->columnCount() > 0) {
        int upperlimit = 5000;
        int noaddableitems =
                std::min(int(filtered.size()), upperlimit - model->rowCount());
        if (int(filtered.size()) + model->rowCount() > upperlimit) {
            std::string taux = "More than ";
            taux += std::to_string(upperlimit);
            taux += " items returned. Use filter function or modify search term.";
            statusline_->setText(taux);
            statusline_->decorationStyle().setForegroundColor(Wt::red);
        } else
            statusline_->setText("");
        model->insertRows(0, noaddableitems);
        for (int r = 0; r < noaddableitems; r++)
            for (int c = 0; c < model->columnCount(); c++) {
                std::string s = filtered[r]->GetColumn(boost::any_cast<std::string>
                        (model->headerData(c)));
                model->setData(r, c, boost::any(s));
            }
    }
}
