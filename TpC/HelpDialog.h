/* 
 * File:   HelpDialog.h
 * Author: mueller
 *
 * Created on September 22, 2015, 10:21 AM
 */

#ifndef HELPDIALOG_H
#define	HELPDIALOG_H

#include <Wt/WDialog>
#include <Wt/WString>
#include <Wt/WContainerWidget>

class HelpDialog : public Wt::WDialog {
public:
    HelpDialog(Wt::WString title, bool modal, Wt::WContainerWidget * contents);
    HelpDialog(const HelpDialog& orig);
    virtual ~HelpDialog();
    void HelpDialogDone();
private:

};

#endif	/* HELPDIALOG_H */

