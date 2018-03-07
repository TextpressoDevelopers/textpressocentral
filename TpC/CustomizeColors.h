/**
    Project: textpressocentral
    File name: CustomizeColors.h
    
    @author valerio
    @version 1.0 5/28/17.
*/

#ifndef TEXTPRESSOCENTRAL_CUSTOMIZECOLORS_H
#define TEXTPRESSOCENTRAL_CUSTOMIZECOLORS_H

#include "Session.h"
#include "ColorSet.h"
#include <Wt/WContainerWidget>
#include <Wt/WGridLayout>
#include <Wt/WCheckBox>
#include <Wt/WPushButton>
#include <Wt/WTemplate>

class CustomizeColors: public Wt::WContainerWidget {
public:
    CustomizeColors(Session* session);
    virtual ~CustomizeColors();
    void onColorChanged(std::string, std::string);
    inline Wt::Signal<int>& signalSaveClicked() {
        return colors.signalColorsChanged();
    }

private:
    void addColorPicker(const std::string&, const std::string&, Wt::WTemplate*, const std::string& = "#000000");
    void readColors();
    void saveColors();
    void setPickerColor(const std::string&, const std::string&);
    Session * session_;
    colors::ColorSet colors;
    Wt::WPushButton* btnSave;

    void setDescrColor(const std::string &id, const std::string &rgbColorStr);
};

#endif //TEXTPRESSOCENTRAL_CUSTOMIZECOLORS_H
