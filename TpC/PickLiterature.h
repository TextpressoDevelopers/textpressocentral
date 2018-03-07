/* 
 * File:   PickLiterature.h
 * Author: mueller
 *
 * Created on March 9, 2015, 2:03 PM
 */

#ifndef PICKLITERATURE_H
#define	PICKLITERATURE_H

#include "Session.h"
#include "PickedLiteratureContents.h"
#include <Wt/WDialog>

class PickLiterature : public Wt::WDialog {
public:
    bool GetCheckState(std::string s) { return plc_->GetCheckState(s); }
    bool SaveBoxIsChecked() { return plc_->SaveBoxIsChecked(); }
    PickLiterature(Session * session, std::map<std::string, bool> & literatures);
    virtual ~PickLiterature();
private:
    PickedLiteratureContents * plc_;
};

#endif	/* PICKLITERATURE_H */
