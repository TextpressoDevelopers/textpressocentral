/* 
 * File:   Ontology.h
 * Author: mueller
 *
 * Created on April 4, 2013, 10:20 AM
 */

#ifndef ONTOLOGY_H
#define	ONTOLOGY_H

#define MODIFIEDSTATUS "resources/icons/modified.png"
#define INSERTEDSTATUS "resources/icons/inserted.png"
#define DELETEDSTATUS "resources/icons/deleted.png"
#define ATTENTION "resources/icons/exclamation-red.png"

#include "OntologyTable.h"
#include "Session.h"
#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WStandardItem>
#include <Wt/WComboBox>
#include <Wt/WLineEdit>
#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WPushButton>
#include <Wt/WCheckBox>
#include "../TextpressoCentralGlobalDefinitions.h"


class Ontology : public Wt::WContainerWidget {
public:
    Ontology(Session & session, Wt::WContainerWidget * parent = NULL);
private:
    void OntSearchClicked();
    void OntFilterClicked();
    void OntInsertClicked();
//    void OntInsertClicked(Session * session);
    void OntDeleteClicked();
    void OntLexicalClicked ();
    void OntCommitClicked();
    void OntUploadClicked();
    void OntSnapshotClicked();
    void OntReverseClicked();
    void OntResetClicked();
    void OntLoadModel(const char * filename);
    void OntSaveModel(const char * filename);
    void OntSelectAllClicked();
    void OntWhereFieldEnterPressed(Wt::WLineEdit * where);
    void OntItemChanged(Wt::WStandardItem * si);
    void SearchAfvfComplete(Wt::WString fieldname, Wt::WString editname);
    void FilterAfvfComplete(Wt::WString fieldname, Wt::WString editname);
    Wt::WContainerWidget * messagecontainer_;
    Wt::WString loginname_;
    Wt::WContainerWidget * actioncontainer_;
    Wt::WContainerWidget * resultcontainer_;
    Wt::WText * message_;
    void AuthEvent(Session * session);
    Wt::WCheckBox * selectall_;
    OntologyTable * ot_;
    Wt::WStandardItemModel * model_;
    bool itemchangedupdate_;
    int snapshotcount_;
    Wt::WPushButton * ontsnapshotbutton_;
    Wt::WPushButton * ontreversebutton_;
//    Wt::WPushButton * ontinsertbutton_;
    Wt::WPushButton * ontdelbutton_;
    Wt::WPushButton * ontlexicalbutton_;
    Wt::WPushButton * ontcommitbutton_;
    void UploadfileTooLarge();
    void ProcessUploadedFile(Wt::WFileUpload * upload);
};

#endif	/* ONTOLOGY_H */
