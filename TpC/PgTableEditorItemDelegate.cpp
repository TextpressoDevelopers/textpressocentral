/* 
 * File:   PgTableEditorItemDelegate.cpp
 * Author: mueller
 * 
 * Created on March 28, 2014, 10:47 AM
 */

#include "PgTableEditorItemDelegate.h"
#include <Wt/WContainerWidget>
#include <Wt/WStandardItemModel>
#include <Wt/WTextArea>
#include <Wt/WModelIndex>
#include <Wt/WHBoxLayout>

Wt::WWidget * PgTableEditorItemDelegate::createEditor(const Wt::WModelIndex & index, Wt::WFlags<Wt::ViewItemRenderFlag> flags) const {
        std::cerr << "@createEditor" << std::endl;
    Wt::WContainerWidget * result = new Wt::WContainerWidget();
    result->setSelectable(true);
    Wt::WTextArea * areaEdit = new Wt::WTextArea();
    areaEdit->setText(Wt::asString(index.data(Wt::EditRole), "%s"));
    areaEdit->enterPressed().connect(boost::bind(&PgTableEditorItemDelegate::doCloseEditor, this, result, true));
    areaEdit->escapePressed().connect(boost::bind(&PgTableEditorItemDelegate::doCloseEditor, this, result, false));
    if (flags & Wt::RenderFocused) areaEdit->setFocus();
    // We use a layout so that the line edit fills the entire cell.
    result->setLayout(new Wt::WHBoxLayout());
    result->layout()->setContentsMargins(1, 1, 1, 1);
    result->layout()->addWidget(areaEdit);

    return result;
}

void PgTableEditorItemDelegate::doCloseEditor(Wt::WWidget * editor, bool save) const {
    closeEditor().emit(editor, save);
}

boost::any PgTableEditorItemDelegate::editState(Wt::WWidget * editor) const {
    std::cerr << "@editState" << std::endl;
    Wt::WContainerWidget *w = dynamic_cast<Wt::WContainerWidget *> (editor);
    //   Wt::WLineEdit *lineEdit = dynamic_cast<Wt::WLineEdit *>(w->widget(0));
    Wt::WTextArea * areaEdit = dynamic_cast<Wt::WTextArea *> (w->widget(0));
    return boost::any(areaEdit->text());
}

void PgTableEditorItemDelegate::setEditState(Wt::WWidget * editor, const boost::any & value) const {
    std::cerr << "@setEditState" << std::endl;
    Wt::WContainerWidget * w = dynamic_cast<Wt::WContainerWidget *> (editor);
    //   Wt::WLineEdit *lineEdit = dynamic_cast<Wt::WLineEdit *>(w->widget(0));
    Wt::WTextArea * areaEdit = dynamic_cast<Wt::WTextArea *> (w->widget(0));
    areaEdit->setText(boost::any_cast<Wt::WString > (value));
}

void PgTableEditorItemDelegate::setModelData(const boost::any& editState,
        Wt::WAbstractItemModel *model, const Wt::WModelIndex& index) const {
    model->setData(index, editState, Wt::EditRole);
}
