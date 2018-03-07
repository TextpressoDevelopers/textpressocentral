/* 
 * File:   PgTableEditorItemDelegate.h
 * Author: mueller
 *
 * Created on March 28, 2014, 10:47 AM
 */

#ifndef PGTABLEEDITORITEMDELEGATE_H
#define	PGTABLEEDITORITEMDELEGATE_H

#include <Wt/WItemDelegate>

class PgTableEditorItemDelegate : public Wt::WItemDelegate {
public:
    boost::any editState(Wt::WWidget * editor) const;
    void setEditState(Wt::WWidget * editor, const boost::any & value) const;
    void setModelData(const boost::any& editState,
            Wt::WAbstractItemModel *model, const Wt::WModelIndex& index) const;
protected:
    Wt::WWidget * createEditor(const Wt::WModelIndex & index, Wt::WFlags<Wt::ViewItemRenderFlag> flags) const;
private:
    void doCloseEditor(Wt::WWidget * editor, bool save) const;
};
#endif	/* PGTABLEEDITORITEMDELEGATE_H */

