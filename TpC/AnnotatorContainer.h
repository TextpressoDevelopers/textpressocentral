/* 
 * File:   AnnotatorContainer.h
 * Author: mueller
 *
 * Created on March 12, 2014, 12:33 PM
 */

#ifndef ANNOTATORCONTAINER_H
#define	ANNOTATORCONTAINER_H

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WCheckBox>

#include <boost/system/error_code.hpp>

class AnnotatorContainer : public Wt::WContainerWidget {
public:

    struct immutables {
        std::vector<Wt::WText*> & curB;
        std::vector<Wt::WText*> & curE;
        std::vector<Wt::WText*> & terms;
        std::map<Wt::WText*, int32_t> & pointer2B;
        std::map<Wt::WText*, int32_t> & pointer2E;
        const char * filename;
        Wt::WString owner;
        std::string paperid;
    };

    AnnotatorContainer(std::string rootfn, Wt::WContainerWidget * parent = NULL);
    Wt::WContainerWidget * CreateEmptyImmutablesForm();
    void PopulateImmutablesContent(immutables inp);
    void CreateAllEmpty();
    void CreateAll(immutables inp);
    void DeleteAll();
    virtual ~AnnotatorContainer();

    inline Wt::EventSignal<Wt::WMouseEvent> & SignalSfClicked() {
        return sf_->clicked();
    }

    inline Wt::EventSignal<Wt::WMouseEvent> & SignalCaClicked() {
        return ca_->clicked();
    }
private:
    Wt::WContainerWidget * CreateImmutablesContent(immutables inp);
    Wt::WContainerWidget * CreateManualsContent();
    Wt::WContainerWidget * CreateButtonsContent();
    Wt::WString GetTextFromTerms(Wt::WText * strtptr, Wt::WText * endptr, std::vector<Wt::WText*> & terms);
    std::string GetBEFromInp(Wt::WText * strtptr, Wt::WText * endptr, immutables inp);
    void DeleteImmutablesContent();
    void DeleteManualsContent();
    void DeleteButtonsContent();
    void SfButtonClicked();
    void CaButtonClicked();
    void JsonButtonClicked();
    void SetCursorHand(Wt::WWidget * w);
    void InfoIconClicked(Wt::WString text);
    void InfoOkButtonClicked(Wt::StandardButton b);
    void HandDownToHttpClient(std::string msg, Wt::WComboBox * servercombobox);
    void HandleHttpResponse(boost::system::error_code err, const Wt::Http::Message & response);
    void DialogDone(Wt::WDialog::DialogCode code);
    void LineEditClicked(Wt::WLineEdit * le);
    void TextAreaClicked(Wt::WTextArea * le);
    struct subcontainer {
        Wt::WContainerWidget * immutables;
        Wt::WContainerWidget * manuals;
        Wt::WContainerWidget * buttons;
    } subcont_;

    struct psqldata {
        Wt::WString term;
        std::string bestring;
        std::string filename;
        Wt::WString owner;
        std::string paperid;
        time_t lastupdate;
        Wt::WString version;
        Wt::WLineEdit * eid;
        Wt::WLineEdit * dbxref;
        Wt::WLineEdit * evidencecode;
        Wt::WLineEdit * curationstatus;
        Wt::WLineEdit * curationuse;
        Wt::WLineEdit * source;
        Wt::WTextArea * ontologyterms;
        Wt::WTextArea * biologicalentities;
        Wt::WTextArea * comment;
    } psqldata_;
    Wt::WText * header_;

    struct Texts {
        Wt::WText * term;
        Wt::WText * bestring;
        Wt::WText * filename;
        Wt::WText * owner;
        Wt::WText * paperid;
        Wt::WText * lastupdate;
        Wt::WText * version;
    } texts_;
    Wt::WContainerWidget * parent_;
    Wt::WCheckBox * clicknfill_;
    Wt::WPushButton * sf_;
    Wt::WPushButton * ca_;
    Wt::WPushButton * js_;
    Wt::WMessageBox * infomessage_;
    Wt::WDialog * httpdialog_;
    std::string rootfn_;
};

#endif	/* ANNOTATORCONTAINER_H */
