/* 
 * File:   ConfigurableAnnotatorContainer.h
 * Author: mueller
 *
 * Created on April 2, 2015, 12:44 PM
 */

#ifndef CONFIGURABLEANNOTATORCONTAINER_H
#define	CONFIGURABLEANNOTATORCONTAINER_H

#include "CurationFieldConfiguration.h"
#include "PgColumn.h"
#include "FetchList.h"
#include "UrlParameters.h"

#include <Wt/WContainerWidget>
#include <Wt/WPushButton>
#include <Wt/WDialog>
#include <Wt/WCheckBox>
#include <Wt/Http/Client>
#include <boost/system/error_code.hpp>

class ConfigurableAnnotatorContainer : public Wt::WContainerWidget {
public:

    struct viewerinfo {
        std::vector<Wt::WText*> & curB;
        std::vector<Wt::WText*> & curE;
        std::vector<Wt::WText*> & terms;
        std::map<Wt::WText*, int32_t> & pointer2B;
        std::map<Wt::WText*, int32_t> & pointer2E;
        const char * filename;
        Wt::WString owner;
        std::string paperid;
        std::string additionalAnnotations;
    };

    ConfigurableAnnotatorContainer(UrlParameters * urlparams, std::string rootfn,
            Wt::WContainerWidget * parent = NULL);
    void CreateEmptyViewerInfoForm();
    void PopulateViewerInfoContent(viewerinfo inp);
    void PopulateFieldsContent (std::string s);
    void CreateAllEmpty();
    inline void SetFormname(std::string s) { formname_ = s; }
    inline Wt::EventSignal<Wt::WMouseEvent> & SignalCaClicked() {
        return ca_->clicked();
    }
    int GetMaxX() { return maxx_; }
    int GetMaxY() {return maxy_; }
    bool ViewerInfoIsEmpty() { return (subcont_.viewerinfo == NULL); }
    bool FieldsIsEmpty() { return (subcont_.fields == NULL); }
    bool ButtonsIsEmpty() { return (subcont_.buttons == NULL); }
    virtual ~ConfigurableAnnotatorContainer();
private:
    Wt::WContainerWidget * parent_;
    Wt::WContainerWidget * cont4subs_;
    Wt::WCheckBox * clicknfilltext_;
    Wt::WCheckBox * clicknfillcat_;
    Wt::WCheckBox * includepaperinfo_;
    Wt::WCheckBox * includetpdbid_;
    //Wt::WPushButton * sf_;
    Wt::WPushButton * ca_;
    Wt::WPushButton * postdatabutton_;
    Wt::WMessageBox * infomessage_;
    Wt::WDialog * httpdialog_;
    Wt::WDialog * mpdialog_;
    std::vector<std::string> moreurlparams_;
    Wt::Http::Client * client_;
    int tpdbid_;
    UrlParameters * urlparams_;
    std::string rootfn_;
    std::string formname_;
    //std::string formurl_;
    //std::string paramformat_;
    int maxx_;
    int maxy_;
    struct subcontainer {
        Wt::WContainerWidget * viewerinfo;
        Wt::WContainerWidget * fields;
        Wt::WContainerWidget * buttons;
    } subcont_;

    struct Texts {
        Wt::WText * term;
        Wt::WText * bestring;
        Wt::WText * additionalannotations;
        Wt::WText * filename;
        Wt::WText * owner;
        Wt::WText * paperid;
        Wt::WText * lastupdate;
        Wt::WText * version;
    } texts_;

    struct curationdata {
        Wt::WString term;
        std::string bestring;
        std::string additionalannotations;
        std::string filename;
        Wt::WString owner;
        std::string paperid;
        time_t lastupdate;
        Wt::WString version;
    } curationdata_;
    
    struct fielddata {
        std::string label;
        CurationFieldConfiguration::fieldtype fieldtype;
        Wt::WLineEdit * le;
        Wt::WTextArea * ta;
        Wt::WCheckBox * cb;
        Wt::WComboBox * co;
    };
    std::vector<fielddata> fielddata_;

    Wt::WString GetTextFromTerms(Wt::WText * strtptr, Wt::WText * endptr,
            std::vector<Wt::WText*> & terms);
    Wt::WString GetCatFromTerms (Wt::WText * strtptr, Wt::WText * endptr, 
            std::vector<Wt::WText*> & terms, std::multimap<Wt::WText*, Wt::WString*> & cats);
    std::string GetBEFromViewerInfo(Wt::WText * strtptr, Wt::WText * endptr,
            viewerinfo inp);
    void CreateViewerInfoContent(viewerinfo inp);
    void CreateFieldsContent();
    void CreateButtonsContent();
    int GetNewRecordIdForSavingToTpDatabase();
    void SaveToTpDatabase(int newrecordid);
    void CaButtonClicked();
    void PostDataButtonClicked();
    void DialogDone(Wt::WDialog::DialogCode code);
    void MPDialogDone(Wt::WDialog::DialogCode code);
    void HandDownToHttpClient(std::string uri, std::string msg, long unsigned int iparamformat);
    void HandleHttpResponse(boost::system::error_code err, const Wt::Http::Message & response);
    void SetCursorHand(Wt::WWidget * w);
    void InfoIconClicked(Wt::WString text);
    void InfoOkButtonClicked(Wt::StandardButton b);
    void LineEditClicked(Wt::WLineEdit * le);
    void TextAreaClicked(Wt::WTextArea * ta);
    void CheckBoxClicked(Wt::WCheckBox * cb);
    void ComboBoxChanged(Wt::WComboBox * co);
    void LineEditChanged(Wt::WLineEdit * pLE, FetchList * feli);
    std::string GeneratePrepopulationText(int ppid);
    void MoreParametersClicked();
};

#endif	/* CONFIGURABLEANNOTATORCONTAINER_H */
