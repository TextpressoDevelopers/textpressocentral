/* 
 * File:   LiteratureProcessProgress.h
 * Author: mueller
 *
 * Created on October 19, 2016, 2:34 PM
 */

#ifndef LITERATUREPROCESSPROGRESS_H
#define	LITERATUREPROCESSPROGRESS_H

#include "Session.h"
#include <Wt/WContainerWidget>
#include <Wt/WTableView>
#include <Wt/WStandardItemModel>

class LiteratureProcessProgress : public Wt::WContainerWidget {
public:
    LiteratureProcessProgress(Session & session, Wt::WContainerWidget * parent = NULL);
    virtual ~LiteratureProcessProgress();
    void PopulateLiteratureTable();
private:
 Session * session_;
    Wt::WText * statusline_;
    Wt::WContainerWidget * literaturecontainer_;
    Wt::WTableView * table_;
    Wt::WStandardItemModel * model_;
    std::string literatureroot_;

    void setDirStructure();

    int get_num_processed_files();

    int get_num_uploaded_files();

    int get_num_tokenized_files();
};

#endif	/* LITERATUREPROCESSPROGRESS_H */

