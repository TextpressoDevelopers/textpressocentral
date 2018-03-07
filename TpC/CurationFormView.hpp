/* 
 * File:   CurationFormView.hpp
 * Author: mueller
 *
 * Created on February 26, 2014, 6:50 PM
 * Adapted from witty website
 */

#ifndef CURATIONFORMVIEW_HPP
#define	CURATIONFORMVIEW_HPP

//#include <Wt/WApplication>
#include <Wt/WBoostAny>
#include <Wt/WComboBox>
#include <Wt/WDate>
#include <Wt/WDateEdit>
#include <Wt/WDateValidator>
//#include <Wt/WFormModel>
//#include <Wt/WImage>
#include <Wt/WIntValidator>
#include <Wt/WLengthValidator>
//#include <Wt/WModelIndex>
#include <Wt/WPushButton>
#include <Wt/WSpinBox>
#include <Wt/WStandardItem>
#include <Wt/WStandardItemModel>
//#include <Wt/WString>
#include <Wt/WTemplateFormView>
#include <Wt/WTextArea>

//#include "Viewer.h"
//#include <Wt/WValidator>

class CurationFormModel : public Wt::WFormModel {
public:
    // Associate each field with a unique string literal.
    // With C++11 you can actually put these directly inside the
    // CurationFormModel class like this:
    //
    //   static constexpr Field FirstNameField = "first-name";

    static const Field FirstNameField;
    static const Field LastNameField;
    static const Field CountryField;
    static const Field CityField;
    static const Field BirthField;
    static const Field ChildrenField;
    static const Field RemarksField;

    CurationFormModel(Wt::WObject * parent = 0)
    : Wt::WFormModel(parent) {
        initializeModels();

        addField(FirstNameField);
        addField(LastNameField);
        addField(CountryField);
        addField(CityField);
        addField(BirthField);
        addField(ChildrenField);
        addField(RemarksField);

        setValidator(FirstNameField, createNameValidator(FirstNameField));
        setValidator(LastNameField, createNameValidator(LastNameField));
        setValidator(CountryField, createCountryValidator());
        setValidator(CityField, createCityValidator());
        setValidator(BirthField, createBirthValidator());
        setValidator(ChildrenField, createChildrenValidator());

        // Here you could populate the model with initial data using
        // setValue() for each field.

        setValue(BirthField, Wt::WDate());
        setValue(CountryField, std::string());
    }

    Wt::WAbstractItemModel *countryModel() {
        return countryModel_;
    }

    int countryModelRow(const std::string& code) {
        for (int i = 0; i < countryModel_->rowCount(); ++i)
            if (countryCode(i) == code)
                return i;

        return -1;
    }

    Wt::WAbstractItemModel *cityModel() {
        return cityModel_;
    }

    void updateCityModel(const std::string& countryCode) {
        cityModel_->clear();

        CityMap::const_iterator i = cities.find(countryCode);

        if (i != cities.end()) {
            const std::vector<std::string>& cities = i->second;

            // The initial text shown in the city combo box should be an empty
            // string.
            cityModel_->appendRow(new Wt::WStandardItem());

            for (unsigned j = 0; j < cities.size(); ++j)
                cityModel_->appendRow(new Wt::WStandardItem(cities[j]));
        } else {
            cityModel_->appendRow(
                    new Wt::WStandardItem("(Choose Country first)"));
        }
    }

    // Get the user data from the model

    Wt::WString userData() {
        return
        Wt::asString(value(FirstNameField)) + " " +
                Wt::asString(value(LastNameField))
                + ": country code=" + Wt::asString(value(CountryField))
                + ", city=" + Wt::asString(value(CityField))
                + ", birth=" + Wt::asString(value(BirthField))
                + ", children=" + Wt::asString(value(ChildrenField))
                + ", remarks=" + Wt::asString(value(RemarksField))
                + ".";
    }

    // Get the right code for the current index.

    std::string countryCode(int row) {
        return boost::any_cast<std::string >
                (countryModel_->data(row, 0, Wt::UserRole));
    }

    typedef std::map< std::string, std::vector<std::string> > CityMap;
    typedef std::map<std::string, std::string> CountryMap;
    
private:
    static const CityMap cities;
    static const CountryMap countries;
    Wt::WStandardItemModel *countryModel_, *cityModel_;

    static const int MAX_LENGTH = 25;
    static const int MAX_CHILDREN = 15;

    void initializeModels() {
        // Create a country model.
        unsigned countryModelRows = countries.size() + 1;
        const unsigned countryModelColumns = 1;
        countryModel_ =
                new Wt::WStandardItemModel(countryModelRows, countryModelColumns,
                this);

        // The initial text shown in the country combo box should be an empty
        // string.
        int row = 0;
        countryModel_->setData(row, 0, std::string(" "), Wt::DisplayRole);
        countryModel_->setData(row, 0, std::string(), Wt::UserRole);

        // For each country, update the model based on the key (corresponding
        // to the country code):
        // - set the country name for the display role,
        // - set the city names for the user role.
        row = 1;
        for (CountryMap::const_iterator i = countries.begin();
                i != countries.end(); ++i) {
            countryModel_->setData(row, 0, i->second, Wt::DisplayRole);
            countryModel_->setData(row++, 0, i->first, Wt::UserRole);
        }

        // Create a city model.
        cityModel_ = new Wt::WStandardItemModel(this);
        updateCityModel(std::string());
    }

    Wt::WValidator *createNameValidator(const std::string& field) {
        Wt::WLengthValidator *v = new Wt::WLengthValidator();
        v->setMandatory(true);
        v->setMinimumLength(1);
        v->setMaximumLength(MAX_LENGTH);
        return v;
    }

    Wt::WValidator *createCountryValidator() {
        Wt::WLengthValidator *v = new Wt::WLengthValidator();
        v->setMandatory(true);
        return v;
    }

    Wt::WValidator *createCityValidator() {
        Wt::WLengthValidator *v = new Wt::WLengthValidator();
        v->setMandatory(true);
        return v;
    }

    Wt::WValidator *createBirthValidator() {
        Wt::WDateValidator *v = new Wt::WDateValidator();
        v->setBottom(Wt::WDate(1900, 1, 1));
        v->setTop(Wt::WDate::currentDate());
        v->setFormat("dd/MM/yyyy");
        v->setMandatory(true);
        return v;
    }

    Wt::WValidator *createChildrenValidator() {
        Wt::WIntValidator *v = new Wt::WIntValidator(0, MAX_CHILDREN);
        v->setMandatory(true);
        return v;
    }

};

const Wt::WFormModel::Field CurationFormModel::FirstNameField = "first-name";
const Wt::WFormModel::Field CurationFormModel::LastNameField = "last-name";
const Wt::WFormModel::Field CurationFormModel::CountryField = "country";
const Wt::WFormModel::Field CurationFormModel::CityField = "city";
const Wt::WFormModel::Field CurationFormModel::BirthField = "birth";
const Wt::WFormModel::Field CurationFormModel::ChildrenField = "children";
const Wt::WFormModel::Field CurationFormModel::RemarksField = "remarks";

// In C++11, this initializing can be done inline, within the declaration:
//
// const CurationFormModel::CityMap CurationFormModel::cities = {
//    { "BE", { "Antwerp", "Bruges", "Brussels", "Ghent" } },
//    { "NL", { "Amsterdam", "Eindhoven", "Rotterdam", "The Hague"} },
//    { "UK", { "London", "Bristol", "Oxford", "Stonehenge"} },
//    { "US", { "Boston", "Chicago", "Los Angeles", "New York"} }
// };

namespace {

    CurationFormModel::CountryMap getCountryMap() {
        CurationFormModel::CountryMap retval;
        retval["BE"] = "Belgium";
        retval["NL"] = "Netherlands";
        retval["UK"] = "United Kingdom";
        retval["US"] = "United States";
        return retval;
    }
}
const CurationFormModel::CountryMap CurationFormModel::countries = getCountryMap();

namespace {

    CurationFormModel::CityMap getCityMap() {
        std::vector<std::string> beCities;
        beCities.push_back("Antwerp");
        beCities.push_back("Bruges");
        beCities.push_back("Brussels");
        beCities.push_back("Ghent");

        std::vector<std::string> nlCities;
        nlCities.push_back("Amsterdam");
        nlCities.push_back("Eindhoven");
        nlCities.push_back("Rotterdam");
        nlCities.push_back("The Hague");

        std::vector<std::string> ukCities;
        ukCities.push_back("London");
        ukCities.push_back("Bristol");
        ukCities.push_back("Oxford");
        ukCities.push_back("Stonehenge");

        std::vector<std::string> usCities;
        usCities.push_back("Boston");
        usCities.push_back("Chicago");
        usCities.push_back("Los Angeles");
        usCities.push_back("New York");

        CurationFormModel::CityMap retval;
        retval["BE"] = beCities;
        retval["NL"] = nlCities;
        retval["UK"] = ukCities;
        retval["US"] = usCities;
        return retval;
    }
}

const CurationFormModel::CityMap CurationFormModel::cities = getCityMap();

class CurationFormView : public Wt::WTemplateFormView {
public:
    // inline constructor

    CurationFormView() {
        model = new CurationFormModel(this);

        setTemplateText(tr("curationform-template"));
        addFunction("id", &Wt::WTemplate::Functions::id);
        addFunction("block", &Wt::WTemplate::Functions::id);

        /*
         * First Name
         */
        Wt::WLineEdit * lefn = new Wt::WLineEdit();
        lefn->setVerticalAlignment(Wt::AlignSub);
        setFormWidget(CurationFormModel::FirstNameField, lefn);

        /*
         * Last Name
         */
        setFormWidget(CurationFormModel::LastNameField, new Wt::WLineEdit());

        /*
         * Country
         */
        Wt::WComboBox *countryCB = new Wt::WComboBox();
        countryCB->setModel(model->countryModel());
        countryCB->setWidth(Wt::WLength(24, Wt::WLength::FontEx));
        countryCB->setHeight(Wt::WLength(3, Wt::WLength::FontEx));

        countryCB->activated().connect(std::bind([ = ] (){
            std::string code = model->countryCode(countryCB->currentIndex());
            model->updateCityModel(code);
        }));

        setFormWidget(CurationFormModel::CountryField, countryCB,
                [ = ] (){// updateViewValue()
            std::string code = boost::any_cast<std::string >
            (model->value(CurationFormModel::CountryField));
            int row = model->countryModelRow(code);
            countryCB->setCurrentIndex(row);
        },

        [ = ] (){// updateModelValue()
            std::string code = model->countryCode(countryCB->currentIndex());
            model->setValue(CurationFormModel::CountryField, code);
        });

        /*
         * City
         */
        Wt::WComboBox *cityCB = new Wt::WComboBox();
        cityCB->setModel(model->cityModel());
        cityCB->setWidth(Wt::WLength(24, Wt::WLength::FontEx));
        cityCB->setHeight(Wt::WLength(3, Wt::WLength::FontEx));
        setFormWidget(CurationFormModel::CityField, cityCB);

        /*
         * Birth Date
         */
        Wt::WDateEdit *dateEdit = new Wt::WDateEdit();
        setFormWidget(CurationFormModel::BirthField, dateEdit,
                [ = ] (){// updateViewValue()
            Wt::WDate date = boost::any_cast<Wt::WDate >
            (model->value(CurationFormModel::BirthField));
            dateEdit->setDate(date);
        },

        [ = ] (){// updateModelValue()
            Wt::WDate date = dateEdit->date();
            model->setValue(CurationFormModel::BirthField, date);
        });

        /*
         * Children
         */
        Wt::WSpinBox * sp = new Wt::WSpinBox();
        sp->setWidth(Wt::WLength(5, Wt::WLength::FontEx));
        sp->setHeight(Wt::WLength(3, Wt::WLength::FontEx));
        setFormWidget(CurationFormModel::ChildrenField, sp);

        /*
         * Remarks
         */
        Wt::WTextArea *remarksTA = new Wt::WTextArea();
        remarksTA->setColumns(10);
        remarksTA->setRows(5);
        setFormWidget(CurationFormModel::RemarksField, remarksTA);

        /*
         * Title & Buttons
         */
        Wt::WString title = Wt::WString("Curator Entries");
        bindString("title", title);

        Wt::WPushButton *button = new Wt::WPushButton("Save and finish!");
        bindWidget("submit-button", button);

        bindString("submit-info", Wt::WString());

        button->clicked().connect(this, &CurationFormView::process);
        
        updateView(model);
    }

private:

    void process() {
        updateModel(model);

        if (model->validate()) {
            // Do something with the data in the model: show it.
            bindString("submit-info",
                    Wt::WString::fromUTF8("Saved user data for ")
                    + model->userData(), Wt::PlainText);
            // Udate the view: Delete any validation message in the view, etc.
            updateView(model);
            // Set the focus on the first field in the form.
            Wt::WLineEdit *viewField =
                    resolve<Wt::WLineEdit*>(CurationFormModel::FirstNameField);
            viewField->setFocus();
        } else {
            bindEmpty("submit-info"); // Delete the previous user data.
            updateView(model);
        }
    }

    CurationFormModel *model;
    Wt::WComboBox *cityCB;
};


//CurationFormView *view = new CurationFormView();

#endif	/* CURATIONFORMVIEW_HPP */

