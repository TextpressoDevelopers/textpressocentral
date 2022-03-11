/* 
 * File:   Viewer.h
 * Author: mueller
 *
 * Created on January 22, 2014, 11:39 AM
 */

#ifndef VIEWER_H
#define	VIEWER_H

#define TPCAS2TPCENTRALDESCRIPTOR "/usr/local/uima_descriptors/Tpcas2TpCentral.xml"

#define ANNOTATIONCOLOR Wt::WColor(245, 245, 245)
#define DEFAULTBACKGROUND Wt::WColor(255, 255, 255)
#define NOTIFYCOLOR Wt::WColor(255, 200, 200)
#define SPANHIGHLIGHTCOLOR Wt::WColor(255, 255, 150)

#define NUMBEROFTHREADS 6

#include "XmlTag.h"
#include "PdfTag.h"
#include "PdfToken.h"
#include "Session.h"
#include "AnnotationGroupBox.h"
#include "CurationGroupBox.h"
#include "ConfigurableAnnotatorContainer.h"
#include "UrlParameters.h"
#include "ColorSet.h"
#include "TCNavWeb.h"

typedef std::multimap<int32_t, uima::ANIterator>::iterator mmiaittype;

class Viewer : public Wt::WContainerWidget {
public:
    Viewer(UrlParameters * urlparams, Session * session, PaperAddress* pa,
            Wt::WContainerWidget * parent = NULL);
    virtual ~Viewer();
    std::vector<Wt::WText*> GetCurB();
    std::vector<Wt::WText*> GetCurE();
    std::vector<Wt::WText*> GetTerms();
    std::multimap<Wt::WText*, Wt::WString*> & GetPointer2Annotations() {
        return wtpt_.Pointer2Annotations;
    }
private:
    Wt::WContainerWidget * parent_;

    enum rawsourcetype {
        unknown, nxml, pdf, tai
    };

    UrlParameters * urlparams_;
    Session * session_;
    std::string filename_;
    std::string rootfn_;
    std::string paperdir_;
    std::string fname_;
    std::string accession_;
    std::vector<int32_t> searchresultspansbegin_;
    std::vector<int32_t> searchresultspansend_;
    std::vector<Wt::WText*> scrollintoviewpoints_;
    int scrollintoviewpointsindex_;

    colors::ColorSet colors;
    // struct for display status

    struct displaystatus {
        bool bold;
        bool italic;
        bool sup;
        bool sub;
        bool title;
        int fontsize;
        //..
    } dispstatus_;
    // struct for display user option

    struct displayuseroption {
        bool value;
        bool term;
        bool content;
        bool graphic;
    } dispuseroption_;
    // do another struct for text-collecting, highlighting, etc.

    struct userstatus {
        int displaytextoption;
        bool highlightsearchresults;
        bool showannotations;
        AnnotationGroupBox * annotationbox;
        CurationGroupBox * curationbox;
        std::vector<Wt::WText*> curationB;
        std::map<Wt::WText*, Wt::WText*> curationE;
        Wt::WText* laststart;
        int currentCatBgcolor;
        int currentKeycolor;
        long unsigned int currentlypointingat;
        std::set<long unsigned int> indicesofhighlighted;
    } userstatus_;
    
    // struct of all Witty object pointer
    struct wtpt {
        std::vector<Wt::WText*> Values;
        std::vector<Wt::WText*> Terms;
        std::vector<Wt::WText*> Contents;
        std::vector<Wt::WImage*> Graphics;
        std::multimap<Wt::WText*, Wt::WString*> Pointer2Annotations;
        // mapping of Wt::Text fragment to begin and end position in CAS string.
        std::map<Wt::WText*, int32_t> Pointer2B;
        std::map<Wt::WText*, int32_t> Pointer2E;
        std::multimap<Wt::WString*, Wt::WText*> Annotation2Pointers;
    } wtpt_;
    // UIMA related
    void CreateUimaEngine(const char * descriptor);
    void GetCas(const char * pszInputFile);
    uima::AnalysisEngine * pEngine_;
    uima::CAS * pCas_;
    //
    rawsourcetype rawsource_;
    std::multimap<int32_t, uima::ANIterator> sortedAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedXmlTagAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedLexAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedPdfTokenAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedPdfTagAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedTaiTokenAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedTaiPageAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedTaiDblBrkAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedTaiSectionAnnotationIds_{};
    std::multimap<int32_t, uima::ANIterator> sortedTaiImageAnnotationIds_{};
    std::set<int32_t> allbeginnings_;
    int32_t firstallbeginnings_;
    int32_t lastallbeginnings_;
    //
    Wt::WContainerWidget * papercontainer_;
    ConfigurableAnnotatorContainer * annotatorContainer_;
    Wt::WBorderLayout * mainlayout_;
    Wt::WPanel * curationpanel_;

    std::vector<Wt::WText*> previously_highlighed;

    //
    void SetImage(std::string location, Wt::WContainerWidget * parent);
    void SetTextAndAnnotations(std::string s, Wt::WText * text,
            std::vector<uima::UnicodeStringRef> & lexann, int32_t beginpos, int32_t endpos);
    void PrepareAnnotationIds();
    void AddXmlTagAsWidget(XmlTag * x, Wt::WContainerWidget * parent);
    Wt::WContainerWidget * DisplayAnnotationRangeNxml(int32_t b, int32_t e,
            Wt::WContainerWidget * parent = NULL);
    Wt::WContainerWidget * DisplayAnnotationRangePdf(int32_t b, int32_t e,
            Wt::WContainerWidget * parent = NULL);
    Wt::WContainerWidget * DisplayAnnotationRangeTai(int32_t b, int32_t e,
            Wt::WContainerWidget * parent = NULL);
    int32_t CheckTagDownStream(uima::UnicodeStringRef & s, int32_t pos,
            bool DownstreamChangeIsPositive, int valuethreshold, int posoffset);
    void SetOptionsInContainer(Wt::WContainerWidget * w);
    void DisplayLevelClicked(Wt::WStackedWidget * sw);
    void DisplayAnnotationInBackgroundClicked(Wt::WStackedWidget * ad);
    void HighlightSearchResultsClicked(Wt::WStackedWidget * hsr);
    void SetAnnotationAndCurationBox(Wt::WContainerWidget * w);
    void ParticularLabelActivated(Wt::WComboBox* cb);
    void SetAnnotationText(Wt::WString s);
    void HideAnnotationText();
    void KeywordEntered(Wt::WLineEdit* le);
    void ResetLineEdit(Wt::WLineEdit * le, Wt::WTimer * timer);
    void ForwardOrBackwardClicked(bool b);
    void UpdateViewerInfoContent();
    void TextClicked(Wt::WText * t);
    void CAButtonClicked();
    void RemoveCurationEntry(Wt::WText * s, Wt::WText * e);
    void HighlightSentenceAndScrollIntoView(Wt::WText *p);
    void HighlightSearchResultSpans();
    void SetScrollIntoViewPoints();
    void SetCursorHand(Wt::WWidget * w);
    void CancelAllSnipletsSignalReceived(bool b);
    void CancelSingleSnipletSignalReceived(Wt::WText * s, Wt::WText* e);
    void CBButtonClicked();

    void readColors();

    Wt::WColor NextCatBgColor();

    Wt::WColor NextKeyColor();

    void HighlightCategory(const string &category, bool scroll_to_first = false);

    WLineEdit *keyword;

    void HighlightKeyword(std::string ss_s, bool scroll_to_first = false);

    bool case_sensitive;
};

#endif	/* VIEWER_H */
