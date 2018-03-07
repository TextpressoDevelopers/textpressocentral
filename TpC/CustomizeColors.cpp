/**
    Project: textpressocentral
    File name: CustomizeColors.cpp
    
    @author valerio
    @version 1.0 5/28/17.
*/

#include "CustomizeColors.h"
#include "Preference.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include "ColorSet.h"
#include <Wt/WBreak>
#include <boost/regex.hpp>
#include <boost/filesystem.hpp>
#include <Wt/WCssDecorationStyle>
#include <Wt/WApplication>
#include <Wt/WTemplate>

using namespace std;
using namespace Wt;
using namespace colors;

CustomizeColors::CustomizeColors(Session* session): colors() {
    session_ = session;
    session_->login().changed().connect(boost::bind(&CustomizeColors::readColors, this));
    Wt::WTemplate *t = new WTemplate(WString::tr("customize-colors"));
    for (int i=1; i <= numColorsPerType; ++i) {
        addColorPicker("K" + to_string(i), "Keyword " + to_string(i), t);
        addColorPicker("C" + to_string(i), "Category " + to_string(i), t);
        addColorPicker("BgC" + to_string(i), "Category " + to_string(i), t);
    }
    btnSave = new WPushButton("Save");
    btnSave->clicked().connect(std::bind(&CustomizeColors::saveColors, this));
    t->bindWidget("save-btn", btnSave);
    WPushButton* btnReset = new WPushButton("Reset");
    btnReset->clicked().connect(std::bind(&CustomizeColors::readColors, this));
    t->bindWidget("reset-btn", btnReset);
    addWidget(t);
    readColors();
}

/*!
 * set the picker color
 *
 * @param pickerElementId DOM element id of the picker
 * @param rgbColorStr value to set. Must be a string containing the RGB hex code (e.g. #ff0000)
 */
void CustomizeColors::setPickerColor(const string& id, const string& rgbColorStr) {
    stringstream stream;
    stream << "$('#color" + id + "').val('" + rgbColorStr + "');";
    doJavaScript(stream.str());
}

void CustomizeColors::setDescrColor(const string& id, const string& rgbColorStr) {
    stringstream ss;
    if (id.substr(0, 2) == "Bg") {
        ss << "$('#desc" + id + "').css('background-color', '" + rgbColorStr + "')";
    } else {
        ss << "$('#desc" + id + "').css('color', '" + rgbColorStr + "')";
    }
    doJavaScript(ss.str());
}

/*!
 * read colors from database and set the picker colors accordingly
 */
void CustomizeColors::readColors() {
    // load preferences for colors
    std::string username("default");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname").toUTF8();
    colors.loadColorsFromDB(username);
    for (int i=1; i <= numColorsPerType; ++i) {
        setPickerColor("K" + to_string(i), getRGBStringFromWColor(colors.getKeywordColor(i - 1)));
        setDescrColor("K" + to_string(i), getRGBStringFromWColor(colors.getKeywordColor(i - 1)));
        setPickerColor("C" + to_string(i), getRGBStringFromWColor(colors.getCategoryColor(i - 1)));
        setDescrColor("C" + to_string(i), getRGBStringFromWColor(colors.getCategoryColor(i - 1)));
        setPickerColor("BgC" + to_string(i), getRGBStringFromWColor(colors.getCategoryBgColor(i - 1)));
        setDescrColor("BgC" + to_string(i), getRGBStringFromWColor(colors.getCategoryBgColor(i - 1)));
    }
}

/*!
 * add a color picker to the container as a new widget and bind function onColorChanged to it
 *
 * @param id the id to assign to the new html color picker element
 * @param title the title to be assigned to the color picker element. This will appear as a label for the
 *     picker
 * @param value the initial value to assign to the color picker, in hexadecimal color notation (e.g. #ff0000)
 * @param tmplt a template where the color picker must be added
 */
void CustomizeColors::addColorPicker(const string& id, const string& title, WTemplate* tmplt, const string& value) {
    stringstream ss;
    if (id.substr(0, 2) == "Bg") {
        ss << "$('#desc" + id + "').css('background-color', '" + value + "')";
    } else {
        ss << "$('#desc" + id + "').css('color', '" + value + "')";
    }
    doJavaScript(ss.str());
    tmplt->bindWidget("desc" + id, new WText(title));
    // use html5 input type color
    tmplt->bindWidget("pick" + id, new WText("<input type='color' style='width:50px' id='color" + id + "' value='" + value + "'>",
                                             XHTMLUnsafeText));
    JSignal<string, string>* onColorChangedSignal =
            new JSignal<string, string>(this, "onColorChangedSignal");
    onColorChangedSignal->connect(this, &CustomizeColors::onColorChanged);
    stringstream ss1;
    ss1 << "$('#color" + id + "').bind('input', function() {";
    ss1 << onColorChangedSignal->createCall("$(this).attr('id')", "$(this).val()")  << "});";
    doJavaScript(ss1.str());
}

/*!
 * react to colorChanged event
 *
 * @param id the DOM element id of the input color that has changed
 * @param value the new value (color) of the element
 */
void CustomizeColors::onColorChanged(string id, string value) {
    setDescrColor(id.substr(5, id.size() - 5), value);
    int intId;
    string colType = id.substr(5, 1);
    // names start from 1 - internal index starts from 0
    if (colType == "K") {
        istringstream(id.substr(6, id.size() - 8)) >> intId;
        colors.setKeywordColor(getWColorFromRGBString(value), --intId);
    } else if (colType == "C"){
        istringstream(id.substr(6, id.size() - 6)) >> intId;
        colors.setCategoryColor(getWColorFromRGBString(value), --intId);
    } else if (colType == "B") {
        istringstream(id.substr(8, id.size() - 8)) >> intId;
        colors.setCategoryBgColor(getWColorFromRGBString(value), --intId);
    }
}

CustomizeColors::~CustomizeColors() {
}

/*!
 * save the actual color configuration to database
 */
void CustomizeColors::saveColors() {
    std::string username("default");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname").toUTF8();
    colors.saveColorsToDB(username);

}
