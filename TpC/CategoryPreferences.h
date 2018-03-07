/* 
 * File:   CategoryPreferences.h
 * Author: mueller
 *
 * Created on March 16, 2015, 2:54 PM
 */

#ifndef CATEGORYPREFERENCES_H
#define	CATEGORYPREFERENCES_H

#include "Session.h"

#include <Wt/WDialog>
#include <Wt/WCheckBox>

class CategoryPreferences : public Wt::WDialog {
public:
    CategoryPreferences(Session * session, std::set<std::string> & preloaded);
    virtual ~CategoryPreferences();
    bool GetCheckState(std::string s) { return choices_[s]; }
    bool SaveBoxIsChecked() { return saveclicked_; }
private:
    std::map<std::string, bool> choices_;
    bool saveclicked_;
    void ChangedChoices(Wt::WCheckBox * cb, std::string s);
    void ChangedSaveBox(Wt::WCheckBox * cb);
};

#endif	/* CATEGORYPREFERENCES_H */
