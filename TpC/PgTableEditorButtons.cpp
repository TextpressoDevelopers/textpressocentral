/* 
 * File:   PgTableEditorButtons.cpp
 * Author: mueller
 * 
 * Created on March 31, 2014, 10:41 AM
 */

#include "PgTableEditorButtons.h"
#include <Wt/WGridLayout>

PgTableEditorButtons::PgTableEditorButtons(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    Wt::WGridLayout * l = new Wt::WGridLayout();
    setLayout(l);
    InsertButton_ = new Wt::WPushButton("Insert");
    InsertButton_->setMaximumSize(Wt::WLength(15, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    DeleteButton_ = new Wt::WPushButton("Toggle delete");
    DeleteButton_->setMaximumSize(Wt::WLength(15, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    SnapshotButton_ = new Wt::WPushButton("Snapshot");
    SnapshotButton_->setMaximumSize(Wt::WLength(15, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    ReverseButton_ = new Wt::WPushButton("Reverse");
    ReverseButton_->setMaximumSize(Wt::WLength(15, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    ReverseButton_->disable();
    CommitButton_ = new Wt::WPushButton("Commit");
    CommitButton_->setMaximumSize(Wt::WLength(15, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    l->addWidget(InsertButton_, 0, 0);
    l->addWidget(DeleteButton_, 0, 1);
    l->addWidget(SnapshotButton_, 0, 2);
    l->addWidget(ReverseButton_, 0, 3);
    l->addWidget(CommitButton_, 0, 4);
    l->setContentsMargins(5, 5, 5, 5);
    setMaximumSize(Wt::WLength(80, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));
}
