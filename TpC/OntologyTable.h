/* 
 * File:   OntologyTable.h
 * Author: mueller
 *
 * Created on June 10, 2013, 7:03 PM
 */

#ifndef ONTOLOGYTABLE_H
#define	ONTOLOGYTABLE_H

#include <Wt/WContainerWidget>
#include <Wt/WTableView>
#include "../TpOntology/TpOntApi.h"

class OntologyTable : public Wt::WContainerWidget {
public:
    OntologyTable();
    OntologyTable(Wt::WStandardItemModel * model,
            TpOntApi * toa = NULL, Wt::WContainerWidget * parent = NULL);
    OntologyTable(const OntologyTable & orig);
    void PopulateModelFromTpoeList(Wt::WStandardItemModel * model, std::vector<TpOntEntry*> & results);
    void CreateTableWithOneEmptyLine(Wt::WStandardItemModel * model, TpOntApi * toa);
    void PopulateModelFromPg(Wt::WStandardItemModel * model, TpOntApi * toa);
    //    void ShowTable () { table_->show(); }
    //    void HideTable () { table_->hide(); }
    void SizeColumns(Wt::WStandardItemModel * model);
private:
    void SetTable(Wt::WStandardItemModel * model);
    Wt::WTableView * table_;
};

#endif	/* ONTOLOGYTABLE_H */

