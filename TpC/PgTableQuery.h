/* 
 * File:   PgTableQuery.h
 * Author: mueller
 *
 * Created on March 24, 2014, 12:55 PM
 */

#ifndef PGTABLEQUERY_H
#define	PGTABLEQUERY_H

#include "SuggestionBoxFromPg.h"

#include <Wt/WContainerWidget>
#include <Wt/WSignal>

#include <pqxx/pqxx>

class PgTableQuery : public Wt::WContainerWidget {
public:
    PgTableQuery(std::string databasename, std::string tablename, Wt::WContainerWidget * parent = NULL);
    Wt::Signal<long unsigned int> & Done() { return done_; }
    std::vector<std::string> GetColumnHeaders();
    pqxx::result GetResults() { return pgresult_; }
    void SearchPg();
private:
    void LineEditEnterPressed();
    void ComboBoxValueChanged();
    void WhereFieldEnterPressed();
    std::string databasename_;
    std::string tablename_;
    Wt::WComboBox * headerscombobox_;
    Wt::WLineEdit * lineedit_;
    SuggestionBoxFromPg * suggestionbox4lineedit_;
    Wt::WLineEdit * where_;
    Wt::Signal<long unsigned int> done_;
    std::string condition_;
    pqxx::result pgresult_;

};

#endif	/* PGTABLEQUERY_H */

