/* 
 * File:   OntologyDisplay.h
 * Author: mueller
 *
 * Created on July 17, 2017, 2:48 PM
 */

#ifndef ONTOLOGYDISPLAY_H
#define ONTOLOGYDISPLAY_H

#include "PickCategoryContainer.h"
#include "OntologyTermQuery.h"
#include "TpOntApi.h"
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>

class OntologyDisplay : public Wt::WContainerWidget {
    typedef std::multimap<std::string, std::string> mmsstype;
public:
    OntologyDisplay(PickCategoryContainer *pcc, OntologyTermQuery *otc,
            Wt::WLength h, Wt::WContainerWidget *parent = NULL);
    OntologyDisplay(const OntologyDisplay& orig);
    virtual ~OntologyDisplay();
private:
    Wt::WContainerWidget * ftc_;
    Wt::WComboBox * ftcolname_;
    Wt::WLineEdit * ftvalue_;
    Wt::WContainerWidget * csc_;
    Wt::WText * statusline_;
    Wt::WTableView * CreateTableHeader(Wt::WStandardItemModel * model,
            const std::vector<std::string> & headers);
    void AddToModel(Wt::WStandardItemModel * model,
            std::vector<TpOntEntry*> items,
            std::string ftcolname, std::string ftvalue);
    void SelectionOkClicked(PickCategoryContainer *pcc);
    void SearchTermEntered(OntologyTermQuery *otc);
    void PopulateTable(PickCategoryContainer *pcc,
            std::string ftcolname, std::string ftvalue);
};

#endif /* ONTOLOGYDISPLAY_H */

