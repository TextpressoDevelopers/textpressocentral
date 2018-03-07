/* 
 * File:   CurationGroupBox.h
 * Author: mueller
 *
 * Created on March 7, 2014, 11:05 AM
 */

#ifndef CURATIONGROUPBOX_H
#define	CURATIONGROUPBOX_H

#include "Session.h"
#include <Wt/WCssDecorationStyle>
#include <Wt/WGroupBox>
#include <Wt/WImage>
#include <Wt/WComboBox>
#include <Wt/WPushButton>

class CurationGroupBox : public Wt::WGroupBox {
public:
    // Basic operations
    CurationGroupBox(Session * session, Wt::WString title, Wt::WContainerWidget * parent = NULL);
    virtual ~CurationGroupBox();
    void SetLoginState();
    // Sniplet related operations

    inline Wt::Signal<bool> & CancelAllSnipletsSignal() {
        return cancelAllSnipletsSignal_;
    }
    void CancelAllSniplets();

    inline Wt::Signal<Wt::WText*, Wt::WText*> & CancelSingleSnipletSignal() {
        return cancelSingleSnipletSignal_;
    }

    inline Wt::Signal<Wt::WText*> & ScrollIntoViewSignal() {
        return scrollIntoViewSignal_;
    }
    void SubmitNewWtextEntry(Wt::WText * t);
    // Curation button

    inline Wt::EventSignal<Wt::WMouseEvent> & CBClicked() {
        return curateButton_->clicked();
    }

    inline void DisableCurateButton() {
        curateButton_->disable();
    }

    inline void EnableCurateButton() {
        curateButton_->enable();
    }
    
    inline std::string GetFormName() {
        return formnameComboBox_->currentText().toUTF8();
    }
private:
    // Basic operations and variables
    Session * session_;
    Wt::WContainerWidget * container_;
    Wt::WStackedWidget * loggedinLoggedoutStack_;
    Wt::WText * howToUseViewerExplanation_;
    Wt::WText * loggedoutExplanation_;
    Wt::WContainerWidget * loggedinContainer_;
    int loggedinLoggedoutState_;
    void InitializeVars();
    void CreateContents();
    void UpdateContents();
    void PopulateFormComboBox();
    inline void SetCursorHand(Wt::WWidget * w) {
        if (w->isDisabled())
            w->decorationStyle().setCursor(Wt::ArrowCursor);
        else
            w->decorationStyle().setCursor(Wt::PointingHandCursor);
    }
    // Sniplet related operations and variables
    Wt::WImage * cancelAllSnipletsImage_;
    Wt::WText * cancelAllSnipletsText_;
    Wt::Signal<bool> cancelAllSnipletsSignal_;
    Wt::Signal<Wt::WText*, Wt::WText*> cancelSingleSnipletSignal_;
    bool cancelAllSnipletsState_;
    // added 031114
    std::vector<Wt::WText*> listOfSnipletStarts_;
    //
    std::map<Wt::WText*, Wt::WText*> snipletStarts_;
    std::map<Wt::WText*, Wt::WText*> snipletEnds_;
    std::map<Wt::WText*, Wt::WText*> endWtext_;
    std::map<Wt::WText*, Wt::WImage*> snipletCancelImage_;
    Wt::WText * lastStart_;
    Wt::Signal<Wt::WText*> scrollIntoViewSignal_;
    Wt::WContainerWidget * snipletsContainer_;
    void RemoveCurationEntry(Wt::WText * t);
    void CancelAllSnipletsClicked(bool imageortext);

    inline void ShowCancelAllSniplets() {
        cancelAllSnipletsImage_->show();
        cancelAllSnipletsText_->show();
    }

    inline void HideCancelAllSniplets() {
        cancelAllSnipletsImage_->hide();
        cancelAllSnipletsText_->hide();
    }
    void SendScrollIntoViewSignal(Wt::WText * t);
    // Collection status line
    Wt::WIconPair * collectionStatusIconPair_;
    Wt::WText * collectionStatusLabel_;
    // Form name
    Wt::WComboBox * formnameComboBox_;
    // Curation button
    Wt::WPushButton * curateButton_;
};

#endif	/* CURATIONGROUPBOX_H */
