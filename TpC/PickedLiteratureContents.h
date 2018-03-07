/* 
 * File:   PickedLiteratureContents.h
 * Author: mueller
 *
 * Created on April 15, 2015, 3:09 PM
 */

#ifndef PICKEDLITERATURECONTENTS_H
#define	PICKEDLITERATURECONTENTS_H

#include "Session.h"
#include <Wt/WContainerWidget>
#include <Wt/WGridLayout>
#include <Wt/WCheckBox>
#include <Wt/WPushButton>

const int squaresize = 5;

class PickedLiteratureContents : public Wt::WContainerWidget {
public:
    PickedLiteratureContents(Session * session,
            std::map<std::string, bool> & literatures,
            Wt::WContainerWidget * parent = NULL);

    inline Wt::EventSignal<Wt::WMouseEvent> & SignalOkClicked() {
        return ok_->clicked();
    }

    bool GetCheckState(std::string s) {
        return pickedliterature_[s];
    }

    bool SaveBoxIsChecked() {
        return savebox_->isChecked();
    }

    void SetSaveBoxChecked(bool b) {
        savebox_->setChecked(b);
    }
    void EnableSaveBox(bool b);
    void ShowSaveBox(bool b);
    virtual ~PickedLiteratureContents();
private:
    Session * session_;
    Wt::WContainerWidget * gridcont_;
    Wt::WGridLayout * grid_;
    std::map<std::string, bool> pickedliterature_;
    void ChangedCheckBox(Wt::WCheckBox * cb, std::string s);
    void ChangedSaveBox(Wt::WCheckBox * cb);
    void ReloadPressed();
    void UpdateLiteraturePreferences(bool checkpermissions);
    void ReadLitPref();
    void PopulateGrid(std::map<std::string, bool> & literatures,
            int squaresize);
    void SessionLoginChanged();
    Wt::WCheckBox * checkall_;
    Wt::WCheckBox * savebox_;
    Wt::WPushButton * ok_;
    std::map<std::string, Wt::WCheckBox*> cbxes_;
};

#endif	/* PICKEDLITERATURECONTENTS_H */
