/* 
 * File:   AnnotationGroupBox.h
 * Author: mueller
 *
 * Created on March 7, 2014, 9:35 AM
 */

#ifndef ANNOTATIONGROUPBOX_H
#define	ANNOTATIONGROUPBOX_H

#define NOTIFYCOLOR Wt::WColor(255, 200, 200)

#include <Wt/WGroupBox>
#include <Wt/WString>
#include <Wt/WContainerWidget>
#include <Wt/WText>

class AnnotationGroupBox : public Wt::WGroupBox {
public:
    AnnotationGroupBox(Wt::WString title, Wt::WContainerWidget * parent = NULL);
    inline void SetAnnotationText(Wt::WString s) { anntext_->setText(s); }
    inline void ShowAnnotationText() { anntext_->show(); }
    inline void HideAnnotationText() { anntext_->hide(); }
    virtual ~AnnotationGroupBox();
private:
    Wt::WContainerWidget * container_;
    Wt::WText * anntext_;
};

#endif	/* ANNOTATIONGROUPBOX_H */
