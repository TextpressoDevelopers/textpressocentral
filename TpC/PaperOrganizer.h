/* 
 * File:   PaperOrganizer.h
 * Author: mueller
 *
 * Created on October 14, 2016, 10:44 AM
 */

#ifndef PAPERORGANIZER_H
#define	PAPERORGANIZER_H

#include "Session.h"
#include <Wt/WContainerWidget>
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>
#include <boost/serialization/map.hpp>
#include <boost/serialization/set.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

class PaperOrganizer : public Wt::WContainerWidget {
public:
    PaperOrganizer(Session & session, Wt::WContainerWidget * parent = NULL);
    virtual ~PaperOrganizer();
    void PopulateUploadTable();
private:
    Session * session_;
    Wt::WText * statusline_;
    Wt::WContainerWidget * filetablecontainer_;
    Wt::WContainerWidget * pushbuttoncontainer_;
    Wt::WContainerWidget * literaturecontainer_;
    Wt::WTableView * table_;
    Wt::WStandardItemModel * model_;
    std::string uploadroot_;
    std::string username_;
    std::map<std::string, std::set<std::string>> papers_lit_map;
    void SelectionChanged();
    void DeleteSelectedItems();
    void DeleteSelectedItemsFromCurrentLiterature();
    void AddSelectedItemsToCurrentLiterature();
    void UpdateLiteraturesInComboBox();
    void AddLiterature(Wt::WLineEdit * litname);
    void LitNameEntered(Wt::WLineEdit * litname);
    void RemoveLiterature();
    void SetCursorHand(Wt::WWidget * w);
    void load_papers_lit_map();
    void save_papers_lit_map();

    void loginChanged();

    std::set<std::string> get_available_literatures();

    Wt::WComboBox *litcombo;
};

#endif	/* PAPERORGANIZER_H */

