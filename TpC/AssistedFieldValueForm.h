/* 
 * File:   AssistedFieldValueForm.h
 * Author: mueller
 *
 * Created on June 25, 2013, 3:03 PM
 */

#ifndef ASSISTEDFIELDVALUEFORM_H
#define	ASSISTEDFIELDVALUEFORM_H

#include <Wt/WContainerWidget>
#include <Wt/WSignal>
#include <Wt/WString>
#include <Wt/WLineEdit>
#include <Wt/WComboBox>
#include <Wt/WStringListModel>
#include <Wt/WStandardItemModel>
#include "../TextpressoCentralGlobalDefinitions.h"


class AssistedFieldValueForm : public Wt::WContainerWidget {
public:
    AssistedFieldValueForm(Wt::WStandardItemModel * model, Wt::WContainerWidget * parent = NULL);
    AssistedFieldValueForm(Wt::WContainerWidget * parent = NULL);
    AssistedFieldValueForm(const AssistedFieldValueForm & orig);
    Wt::Signal<Wt::WString, Wt::WString> & done() { return done_; }
private:
    void LineEditEnterPressed(Wt::WLineEdit * le);
    void ComboBoxValueChanged(Wt::WComboBox * cb, Wt::WLineEdit * edit);
    void PopulateModel(const Wt::WString & s);
    void PopulateModelFromModel(const Wt::WString & s);
    Wt::WStringListModel * spslm_;
    Wt::WString comboboxvalue_;
    Wt::WString linestring_;
    Wt::Signal<Wt::WString, Wt::WString> done_;
    Wt::WStandardItemModel * model_;

};

#endif	/* ASSISTEDFIELDVALUEFORM_H */

