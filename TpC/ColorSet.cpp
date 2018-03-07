/**
    Project: textpressocentral
    File name: ColorSet.cpp
    
    @author valerio
    @version 1.0 5/30/17.
*/

#include <iomanip>
#include "ColorSet.h"
#include "Preference.h"
#include "TextpressoCentralGlobalDefinitions.h"

using namespace std;
using namespace Wt;
using namespace colors;

void colors::ColorSet::setDefaultColors() {
    keywordColors = vector<WColor> {WColor(255, 127, 127), WColor(127, 255, 127), WColor(230, 190, 0),
                                    WColor(127, 127, 255), WColor(255, 127, 255), WColor(127, 255, 255),
                                    WColor(127, 64, 64), WColor(64, 127, 64), WColor(127, 127, 64),
                                    WColor(64, 64, 127)};

    categoryColorVec = vector<WColor> {WColor(255, 127, 127), WColor(127, 255, 127), WColor(230, 190, 0),
                                       WColor(127, 127, 255), WColor(255, 127, 255), WColor(127, 255, 255),
                                       WColor(127, 64, 64), WColor(64, 127, 64), WColor(127, 127, 64),
                                       WColor(64, 64, 127)};

    categoryBgColorVec = vector<WColor> {WColor(255, 127, 127), WColor(127, 255, 127), WColor(230, 190, 0),
                                         WColor(127, 127, 255), WColor(255, 127, 255), WColor(127, 255, 255),
                                         WColor(127, 64, 64), WColor(64, 127, 64), WColor(127, 127, 64),
                                         WColor(64, 64, 127)};
}

void colors::ColorSet::loadColorsFromDB(const string& dbUsername) {
    keywordColors = vector<WColor>();
    categoryColorVec = vector<WColor>();
    categoryBgColorVec = vector<WColor>();
    Preference prefTable(PGCUSTOMCOLORS, PGCUSTOMCOLORSTABLENAME, dbUsername);
    if (!prefTable.HasPreferences()) {
        setDefaultColors();
        saveColorsToDB(dbUsername);
    } else {
        vector<string> prefColorVec (prefTable.GetPreferencesVec());
        if (prefColorVec.size() == (numColorsPerType) * 3) {
            int counter = 0;
            for (auto color : prefColorVec) {
                switch (counter / numColorsPerType) {
                    case 0: keywordColors.push_back(getWColorFromRGBString(color));
                        break;
                    case 1: categoryColorVec.push_back(getWColorFromRGBString(color));
                        break;
                    case 2: categoryBgColorVec.push_back(getWColorFromRGBString(color));
                        break;
                    default:
                        throw new runtime_error("number of saved colors does not match");
                }
                ++counter;
            }
        } else {
            setDefaultColors();
            saveColorsToDB(dbUsername);
        }
    }
}

/*!
 * get the ith color from a color vector
 *
 * @return the ith color of the vector
 */
WColor colors::ColorSet::getColorFromVec(vector<WColor> vec, vector<WColor>::size_type i) {
    return vec.at(i >= numColorsPerType ? 0 : i);
}

/*!
 * get the color for keyword highlighting
 *
 * @return the color for keyword highlighting
 */
WColor colors::ColorSet::getKeywordColor(vector<WColor>::size_type i) {
    return getColorFromVec(keywordColors, i);
}

/*!
 * get the ith color for category highlighting
 *
 * @param i the index of the color to retrieve
 * @return the color of the requested category
 */
WColor colors::ColorSet::getCategoryColor(vector<WColor>::size_type i) {
    return getColorFromVec(categoryColorVec, i);
}

/*!
 * get the ith background color for category highlighting
 *
 * @param i the index of the color to retrieve
 * @return the color of the requested category
 */
WColor colors::ColorSet::getCategoryBgColor(vector<WColor>::size_type i) {
    return getColorFromVec(categoryBgColorVec, i);
}

/*!
 * set the color for the specified keyword
 *
 * @param color the color to set
 * @param i the index of the category to set
 */
void ColorSet::setKeywordColor(const Wt::WColor& color, std::vector<Wt::WColor>::size_type i) {
    if (i < keywordColors.size()) {
        keywordColors[i] = color;
    }
}

/*!
 * set the color for the specified category
 *
 * @param color the color to set
 * @param i the index of the category to set
 */
void ColorSet::setCategoryColor(const Wt::WColor& color, std::vector<Wt::WColor>::size_type i) {
    if (i < categoryColorVec.size()) {
        categoryColorVec[i] = color;
    }
}

/*!
 * set the background color for the specified category
 *
 * @param color the color to set
 * @param i the index of the category to set
 */
void ColorSet::setCategoryBgColor(const Wt::WColor& color, std::vector<Wt::WColor>::size_type i) {
    if (i < categoryBgColorVec.size()) {
        categoryBgColorVec[i] = color;
    }
}

void ColorSet::saveColorsToDB(const std::string& dbUsername) {
    vector<string> prefColorVec;
    for (auto catColor : keywordColors) {
        prefColorVec.push_back(getRGBStringFromWColor(catColor));
    }
    for (auto catColor : categoryColorVec) {
        prefColorVec.push_back(getRGBStringFromWColor(catColor));
    }
    for (auto catColor : categoryBgColorVec) {
        prefColorVec.push_back(getRGBStringFromWColor(catColor));
    }
    Preference prefTable(PGCUSTOMCOLORS, PGCUSTOMCOLORSTABLENAME, dbUsername);
    prefTable.SavePreferencesVector(dbUsername, prefColorVec);
    colorsChangedSignal.emit(0);
}

/*!
 * get the RGB string representation of a WColor
 *
 * @param color a WColor object
 * @return the RGB string representing the WColor
 */
string colors::getRGBStringFromWColor(const WColor& color) {
    stringstream red, green, blue;
    red << hex << setw(2) << setfill('0') << color.red();
    green << hex << setw(2) << setfill('0') << color.green();
    blue << hex << setw(2) << setfill('0') << color.blue();
    string ret = "#" + red.str() + green.str() + blue.str();
    return ret;
}

/*!
 * extract the int value associated to the red, green or blue components from a hex RGB string
 *
 * @param hexColorStr a hex RGB string
 * @param pos the component to extract. 0 for red, 1 for green and
 * @return
 */
int colors::getIntFromRGBString(const string& hexColorStr, ColorComponent type) {
    string hexStr = hexColorStr.substr((unsigned long) (1 + ((int) type * 2)), 2);
    int value;
    stringstream stream;
    stream << hex << hexStr;
    stream >> value;
    return value;
}

/*!
 * create a WColor object from a RGB hex string
 *
 * @param colorStr a RGB hex color string
 * @return a new WColor object
 */
Wt::WColor colors::getWColorFromRGBString(const std::string & colorStr) {
    return WColor(getIntFromRGBString(colorStr, ColorComponent::RED),
           getIntFromRGBString(colorStr, ColorComponent::GREEN),
           getIntFromRGBString(colorStr, ColorComponent::BLUE));
}
