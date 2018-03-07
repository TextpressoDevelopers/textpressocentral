/* 
 * File:   OntologyTable.cpp
 * Author: mueller
 * 
 * Created on June 10, 2013, 7:03 PM
 */

#include "OntologyTable.h"
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WText>


namespace {

    void PopulateModelHeaders(Wt::WStandardItemModel * model, TpOntApi * toa) {
        std::vector<std::string> headers = toa->GetColumnHeaders();
        model->insertColumns(0, headers.size() + 1);
        int col = 0;
        model->setHeaderData(col++, boost::any(std::string("select")));
        model->setHeaderData(col++, boost::any(std::string("status")));
        while (!headers.empty()) {
            model->setHeaderData(col++, boost::any(headers.back()));
            headers.pop_back();
        }
    }

    void MakeModelItemsEditable(Wt::WStandardItemModel * model) {
        for (int row = 0; row < model->rowCount(); ++row)
            for (int col = 0; col < model->columnCount(); ++col) {
                if (model->item(row, col) != NULL) {
                    if (col == 0)
                        model->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsUserCheckable);
                    else if (col > 2)
                        model->item(row, col)->setFlags(Wt::ItemIsSelectable | Wt::ItemIsEditable);
                }
            }
    }
}

void OntologyTable::PopulateModelFromTpoeList(Wt::WStandardItemModel * model,
        std::vector<TpOntEntry*> & results) {
    std::vector<TpOntEntry*>::iterator toeid;
    int row = 0;
    for (toeid = results.begin(); toeid != results.end(); toeid++, row++) {
        model->setData(row, 0,
                boost::any(false), Wt::CheckStateRole);
        model->setData(row, 1,
                boost::any(), Wt::DecorationRole);
        model->setData(row, 2,
                boost::any((*toeid)->GetIid()));
        for (int j = 3; j < model->columnCount(); j++) {
            std::string mheader = Wt::asString(model->headerData(j)).toUTF8();
            if (mheader.compare("eid") == 0)
                model->setData(row, j, boost::any((*toeid)->GetEntryId()));
            else if (mheader.compare("dbxref") == 0)
                model->setData(row, j, boost::any((*toeid)->GetDbXref()));
            else if (mheader.compare("term") == 0)
                model->setData(row, j, boost::any((*toeid)->GetTerm()));
            else if (mheader.compare("category") == 0)
                model->setData(row, j, boost::any((*toeid)->GetCategory()));
            else if (mheader.compare("attributes") == 0)
                model->setData(row, j, boost::any((*toeid)->GetAttributes()));
            else if (mheader.compare("annotationtype") == 0)
                model->setData(row, j, boost::any((*toeid)->GetAnnotationType()));
            else if (mheader.compare("lexicalvariations") == 0)
                model->setData(row, j, boost::any((*toeid)->GetLexicalVariations()));
            else if (mheader.compare("curation_status") == 0)
                model->setData(row, j, boost::any((*toeid)->GetStatus()));
            else if (mheader.compare("curation_use") == 0)
                model->setData(row, j, boost::any((*toeid)->GetUse()));
            else if (mheader.compare("comment") == 0)
                model->setData(row, j, boost::any((*toeid)->GetComment()));
            else if (mheader.compare("owner") == 0)
                model->setData(row, j, boost::any((*toeid)->GetOwner()));
            else if (mheader.compare("source") == 0)
                model->setData(row, j, boost::any((*toeid)->GetSource()));
            else if (mheader.compare("version") == 0)
                model->setData(row, j, boost::any((*toeid)->GetVersion()));
            else if (mheader.compare("last_update") == 0) {
                char buff[20];
                time_t t = (*toeid)->GetLastUpdate();
                strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&t));
                model->setData(row, j,
                        boost::any(std::string(buff)));
            }
        }
    }
}

void OntologyTable::CreateTableWithOneEmptyLine(Wt::WStandardItemModel * model, TpOntApi * toa) {
    model->clear();
    model->insertRows(0, 1);
    PopulateModelHeaders(model, toa);
    model->setData(0, 0,
            boost::any(false), Wt::CheckStateRole);
    model->setData(0, 1,
            boost::any(), Wt::DecorationRole);
    for (int j = 2; j < model->columnCount(); j++) {
            model->setData(0, j, boost::any());
    }
    MakeModelItemsEditable(model);
    SizeColumns(model);
}

void OntologyTable::PopulateModelFromPg(Wt::WStandardItemModel * model, TpOntApi * toa) {
    std::vector<TpOntEntry*> results = toa->GetResultList();
    if (results.size() > 0) {
        model->insertRows(0, results.size());
        PopulateModelHeaders(model, toa);
        PopulateModelFromTpoeList(model, results);
        MakeModelItemsEditable(model);
    }
    SizeColumns(model);
}

OntologyTable::OntologyTable() {

}

void OntologyTable::SetTable(Wt::WStandardItemModel * model) {

#define CELLHEIGHT 24
#define CELLWIDTH 20

    table_ = new Wt::WTableView(this);
    //    table_->setMargin(10, Wt::Top | Wt::Bottom);
    //    table_->setMargin(Wt::WLength::Auto, Wt::Left | Wt::Right);
    table_->setSortingEnabled(true);
    table_->setAlternatingRowColors(true);
    table_->setRowHeight(CELLHEIGHT);
    //  other options:
    //  table_->decorationStyle().setBackgroundColor(Wt::WColor(200,255,200));
    //  table->setColumnHidden(1,true);
    //table_->setSelectionMode(Wt::NoSelection);
    table_->setSelectable(true);
    //  table->setColumnAlignment(0, AlignCenter);
    //  table->setHeaderAlignment(0, AlignCenter);
    //
    //  // Editing does not really work without Ajax, it would require an
    //  // additional button somewhere to confirm the edited value.
    //  if (WApplication::instance()->environment().ajax()) {
    table_->resize(Wt::WLength(8 * CELLWIDTH, Wt::WLength::FontEx), 17 * CELLHEIGHT);
    //    table_->resize(600, Wt::WLength::Auto);
    table_->setEditTriggers(Wt::WAbstractItemView::SingleClicked);
    //  } else {
    //    table->resize(600, WLength::Auto);
    //    table->setEditTriggers(WAbstractItemView::NoEditTrigger);
    //  }
    //
    //  // We use a single delegate for all items which rounds values to
    //  // the closest integer value.
    //      Wt::WItemDelegate * delegate = new Wt::WItemDelegate(this);
    //      delegate->setTextFormat("%.f");
    //      table->setItemDelegate(delegate);

    SizeColumns(model);
}

void OntologyTable::SizeColumns(Wt::WStandardItemModel * model) {
    table_->setColumnResizeEnabled(true);
    table_->setColumnWidth(0, Wt::WLength(9, Wt::WLength::FontEx));
    table_->setColumnWidth(1, Wt::WLength(9, Wt::WLength::FontEx));
    for (int i = 2; i < model->columnCount(); ++i)
        table_->setColumnWidth(i, Wt::WLength(20, Wt::WLength::FontEx));
}

OntologyTable::OntologyTable(Wt::WStandardItemModel * model, TpOntApi * toa,
        Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    SetTable(model);
    if (toa != NULL)
        if (toa->GetResultListSize() > 0)
            PopulateModelFromPg(model, toa);
    table_->setModel(model);
}

OntologyTable::OntologyTable(const OntologyTable & orig) {
}
