/* 
 * File:   PgTableEditorButtons.h
 * Author: mueller
 *
 * Created on March 31, 2014, 10:41 AM
 */

#ifndef PGTABLEEDITORBUTTONS_H
#define	PGTABLEEDITORBUTTONS_H

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>

class PgTableEditorButtons : public Wt::WContainerWidget {
public:
    PgTableEditorButtons(Wt::WContainerWidget * parent = NULL);
    Wt::EventSignal<Wt::WMouseEvent> & InsertClicked() { return InsertButton_->clicked(); }
    Wt::EventSignal<Wt::WMouseEvent> & DeleteClicked() { return DeleteButton_->clicked(); }
    Wt::EventSignal<Wt::WMouseEvent> & SnapshotClicked() { return SnapshotButton_->clicked(); }
    Wt::EventSignal<Wt::WMouseEvent> & ReverseClicked() { return ReverseButton_->clicked(); }
    Wt::EventSignal<Wt::WMouseEvent> & CommitClicked() { return CommitButton_->clicked(); }
    void EnableInsertButton() { InsertButton_->enable(); }
    void EnableDeleteButton() { DeleteButton_->enable(); }
    void EnableSnapshotButton() { SnapshotButton_->enable(); }
    void EnableReverseButton() { ReverseButton_->enable(); }
    void EnableCommitButton() { CommitButton_->enable(); }
    void DiableInsertButton() { InsertButton_->disable(); }
    void DisableDeleteButton() { DeleteButton_->disable(); }
    void DisableSnapshotButton() { SnapshotButton_->disable(); }
    void DisableReverseButton() { ReverseButton_->disable(); }
    void DisableCommitButton() { CommitButton_->disable(); }
private:
    Wt::WPushButton * InsertButton_;
    Wt::WPushButton * DeleteButton_;
    Wt::WPushButton * SnapshotButton_;
    Wt::WPushButton * ReverseButton_;
    Wt::WPushButton * CommitButton_;
};

#endif	/* PGTABLEEDITORBUTTONS_H */

