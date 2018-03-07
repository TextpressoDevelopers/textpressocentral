/* 
 * File:   AssistedFieldValueForm.cpp
 * Author: mueller
 * 
 * Created on June 25, 2013, 3:03 PM
 */

#include "AssistedFieldValueForm.h"
#include "OntologyTable.h"

#include <Wt/WText>
#include <Wt/WCssDecorationStyle>
#include <Wt/WLineEdit>
#include <Wt/WSuggestionPopup>

namespace {

    Wt::WLineEdit * PrepLineEdit() {
        Wt::WLineEdit * edit = new Wt::WLineEdit();
        edit->setEmptyText("Enter a term");
        edit->hide();
        edit->setVerticalAlignment(Wt::AlignMiddle);
        return edit;
    }

    Wt::WSuggestionPopup * PrepSuggestionPopup(Wt::WStringListModel * spslm, Wt::WLineEdit * edit) {
        Wt::WSuggestionPopup::Options contactOptions;
        contactOptions.highlightBeginTag = "<span class=\"highlight\">";
        contactOptions.highlightEndTag = "</span>";
        contactOptions.listSeparator = ',';
        contactOptions.whitespace = " \\n";
        contactOptions.wordSeparators = "-., \"@\\n;";
        //    contactOptions.appendReplacedText = ", ";
        Wt::WSuggestionPopup * sp = new Wt::WSuggestionPopup(
                Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
                Wt::WSuggestionPopup::generateReplacerJS(contactOptions));
        sp->setFilterLength(3);
        sp->setModel(spslm);
        sp->forEdit(edit);
        sp->setMaximumSize(Wt::WLength::Auto, 200);
        return sp;
    }

    Wt::WComboBox * PrepComboBox(std::vector<std::string> & headers) {
        std::sort(headers.begin(), headers.end(), std::greater<std::string>());
        Wt::WComboBox * cb = new Wt::WComboBox();
        cb->setVerticalAlignment(Wt::AlignMiddle);
        cb->addItem(Wt::WString());
        while (!headers.empty()) {
            cb->addItem(headers.back());
            headers.pop_back();
        }
        return cb;
    }
}

AssistedFieldValueForm::AssistedFieldValueForm(Wt::WStandardItemModel * model, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    /////
    model_ = model;
    Wt::WText * title1 = new Wt::WText("Pick a column name and enter a term: ");
    title1->decorationStyle().font().setWeight(Wt::WFont::Bold);
    title1->decorationStyle().setForegroundColor(Wt::WColor(248, 148, 6));
    title1->setVerticalAlignment(Wt::AlignMiddle);
    this->addWidget(title1);
    /////
    std::vector<std::string> headers;
    for (int i = 2; i < model->columnCount(); i++) {
//        headers.push_back(boost::any_cast<std::string > (model->headerData(i)));
        headers.push_back(Wt::asString(model->headerData(i)).toUTF8());
    }
    /////
    Wt::WLineEdit * edit = PrepLineEdit();
    edit->enterPressed().connect(boost::bind(&AssistedFieldValueForm::LineEditEnterPressed, this, edit));
    /////
    // Suggestion  box for WLineEdit
    spslm_ = new Wt::WStringListModel(this);
    Wt::WSuggestionPopup * sp = PrepSuggestionPopup(spslm_, edit);
    sp->filterModel().connect(this, &AssistedFieldValueForm::PopulateModelFromModel);
    /////
    Wt::WComboBox * cb = PrepComboBox(headers);
    cb->changed().connect(boost::bind(&AssistedFieldValueForm::ComboBoxValueChanged, this, cb, edit));
    /////
    this->addWidget(cb);
    this->addWidget(edit);
    /////
}

AssistedFieldValueForm::AssistedFieldValueForm(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    /////
    Wt::WText * title1 = new Wt::WText("Pick a column name and enter a term: ");
    title1->decorationStyle().font().setWeight(Wt::WFont::Bold);
    title1->decorationStyle().setForegroundColor(Wt::WColor(0, 68, 204));
    title1->setVerticalAlignment(Wt::AlignMiddle);
    this->addWidget(title1);
    /////   
    TpOntApi * toa = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    std::vector<std::string> headers = toa->GetColumnHeaders();
    delete toa;
    /////
    Wt::WLineEdit * edit = PrepLineEdit();
    edit->enterPressed().connect(boost::bind(&AssistedFieldValueForm::LineEditEnterPressed, this, edit));
    /////
    // Suggestion  box for WLineEdit
    spslm_ = new Wt::WStringListModel(this);
    Wt::WSuggestionPopup * sp = PrepSuggestionPopup(spslm_, edit);
    sp->filterModel().connect(this, &AssistedFieldValueForm::PopulateModel);
    /////
    Wt::WComboBox * cb = PrepComboBox(headers);
    cb->changed().connect(boost::bind(&AssistedFieldValueForm::ComboBoxValueChanged, this, cb, edit));
    /////
    this->addWidget(cb);
    this->addWidget(edit);
    /////
}

AssistedFieldValueForm::AssistedFieldValueForm(const AssistedFieldValueForm & orig) {
}

void AssistedFieldValueForm::ComboBoxValueChanged(Wt::WComboBox * cb, Wt::WLineEdit * edit) {
    comboboxvalue_ = cb->currentText();
    edit->show();
}

void AssistedFieldValueForm::LineEditEnterPressed(Wt::WLineEdit * le) {
    linestring_ = le->text();
    done_.emit(comboboxvalue_, linestring_);
}

void AssistedFieldValueForm::PopulateModel(const Wt::WString & s) {
    spslm_->removeRows(0, spslm_->rowCount());
    TpOntApi * toa = new TpOntApi(PGONTOLOGYTABLENAME, PCRELATIONSTABLENAME, PADCRELATIONSTABLENAME);
    std::string search = "(^| )" + s.toUTF8();
    std::transform(search.begin(), search.end(), search.begin(), ::tolower);
    std::vector<std::string> cols = toa->GetOneColumnLC(comboboxvalue_.toUTF8(), search);
    std::set<std::string> seen;
    std::cerr << cols.size() << " cols returned from postgres.";
    while (!cols.empty()) {
        std::set<std::string>::iterator it = seen.find(cols.back());
        if (it == seen.end()) {
            spslm_->addString(Wt::WString::fromUTF8(cols.back()));
            seen.insert(cols.back());
        }
        cols.pop_back();
    }
    std::cerr << seen.size() << " terms suggested." << std::endl;
    delete toa;
    spslm_->sort(0);
}

void AssistedFieldValueForm::PopulateModelFromModel(const Wt::WString & s) {
    spslm_->removeRows(0, spslm_->rowCount());
    std::set<std::string> seen;
    for (int i = 2; i < model_->columnCount(); i++) {
        std::string aux = Wt::asString(model_->headerData(i)).toUTF8();
//        std::string aux = boost::any_cast<std::string > (model_->headerData(i));
        if (comboboxvalue_.toUTF8().compare(aux) == 0) {
            for (int j = 0; j < model_->rowCount(); j++) {
                //std::stringstream ss;
                //std::cerr << ss.str() << std::endl;
                std::string aux2 = Wt::asString(model_->data(j, i, Wt::DisplayRole)).toUTF8();
                std::set<std::string>::iterator it = seen.find(aux2);
                if (it == seen.end()) {
                    std::size_t pos = aux2.find(s.toUTF8());
                    std::size_t pos2 = aux2.find(' ' + s.toUTF8());
                    if ((pos != std::string::npos) || (pos2 != std::string::npos)) {
                        spslm_->addString(Wt::WString::fromUTF8(aux2));
                        seen.insert(aux2);
                    }
                }
            }
        }
    }
    spslm_->sort(0);
}