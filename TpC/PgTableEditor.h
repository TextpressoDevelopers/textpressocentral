/* 
 * File:   PgTableEditor.h
 * Author: mueller
 *
 * Created on March 21, 2014, 2:36 PM
 */

#ifndef PGTABLEEDITOR_H
#define	PGTABLEEDITOR_H

#define MODIFIEDSTATUS "resources/icons/modified.png"
#define INSERTEDSTATUS "resources/icons/inserted.png"
#define DELETEDSTATUS "resources/icons/deleted.png"
#define OPENCOLUMN "resources/icons/open11.png"
#define CLOSEDCOLUMN "resources/icons/close11.png"
#define OK "resources/icons/check_box.png"
#define INFO16 "resources/icons/information16.png"
#define INFO32 "resources/icons/information32.png"
#define ATTENTION "resources/icons/exclamation-red.png"

#define TABLECELLHEIGHT 24
#define TABLECELLWIDTH Wt::WLength(20, Wt::WLength::FontEx)

#include "PgTableQuery.h"
#include "PgTableEditorButtons.h"
#include "PgTableEditorItemDelegate.h"
#include "Session.h"

#include <Wt/WContainerWidget>
#include <Wt/WDialog>

class PgTableEditor : public Wt::WContainerWidget {
public:
    PgTableEditor(Session & session, std::string databasename, std::string tablename, Wt::WContainerWidget * parent = NULL);
    virtual ~PgTableEditor();
private:
    Session * session_;

    struct containers {
        Wt::WContainerWidget * editing;
        Wt::WContainerWidget * table;
        Wt::WContainerWidget * message;
    } containers_;

    struct message {
        Wt::WText * text;
        Wt::WImage * icon;
    } message_;

//    struct panels {
//        Wt::WPanel * control;
//        Wt::WPanel * editing;
//        Wt::WPanel * query;
//        Wt::WPanel * table;
//    } panels_;
    Wt::WStandardItemModel * model_;
    Wt::WCheckBox * selectall_;
    Wt::WTableView * table_;
    std::vector<int> headerclicks_;
    PgTableQuery * pgq_;
    PgTableEditorButtons * pgb_;
    int snapshotcount_;
    bool modelitemchangedupdate_;
    Wt::WDialog * dialog_;
    void PopulateModelHeaders();
    void SizeColumns();
    void SetTable();
    void PopulateModelFromPg();
    void MakeModelUserEditable();
    void ModelItemChanged (Wt::WStandardItem * s);
    void PgTableQueryDone(long unsigned int);
    void HeaderClicked(int i);
    void TableClicked(Wt::WModelIndex model, Wt::WMouseEvent event);
    void DialogDone(Wt::WDialog::DialogCode code);
    void EditingEnterPressed(Wt::WModelIndex index, Wt::WTextArea * t);
    void EditingEscapePressed(Wt::WModelIndex index, Wt::WTextArea * t);
    void SelectAllClicked();
    void PgbInsertClicked();
    void PgbDeleteClicked();
    void PgbSnapshotClicked();
    void PgbSnapshotClicked2();
    void PgbReverseClicked();
    void PgbReverseClicked2();
    void PgbCommitClicked();
};

#endif	/* PGTABLEEDITOR_H */

