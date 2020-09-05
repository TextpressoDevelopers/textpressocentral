/* 
 * File:   Customization.h
 * Author: mueller
 *
 * Created on April 15, 2015, 1:48 PM
 */

#ifndef CUSTOMIZATION_H
#define	CUSTOMIZATION_H

#include "Session.h"
#include "PickedLiteratureContents.h"
#include "CreateCurationForm.h"
#include "CustomizeColors.h"
#include <Wt/WContainerWidget>

class Customization : public Wt::WContainerWidget {
public:
    Customization(Wt::WContainerWidget * parent = NULL);
    void LoadContent(Session & session);
    CreateCurationForm * GetCreateCurationFormInstance() { return ccf_; }
    CustomizeColors* getCustomizeColorsInstance() { return ccol; }
    virtual ~Customization();

    void SessionLoginChanged();

private:
    bool alreadyloaded_;
    Session * session_;
    Wt::WText * statusline_;
    Wt::WTabWidget * tabwidget_;
    std::map<std::string, bool> pickedliterature_;
    CreateCurationForm * ccf_;
    CustomizeColors* ccol;
    void AddLitPrefFromFile(std::string fname, bool checkpermissions);
    void ReadLitPref();
    void PlcOkClicked(PickedLiteratureContents * plc);
    void colorSaveClicked();
    void EraseStatusLine();

    void FillContainerWithHelpText(Wt::WContainerWidget * p);
};

#endif	/* CUSTOMIZATION_H */
