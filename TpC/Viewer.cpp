/* 
 * File:   Viewer.cpp
 * Author: mueller
 * 
 * Created on January 22, 2014, 11:39 AM
 */

#include "Viewer.h"
#include "SuggestionBoxFromPgOntology.h"
#include "StopWords.h"
#include "displaySwitches.h"

#include "xercesc/util/XMLString.hpp"
#include "uima/xmideserializer.hpp"
#include "TCNavWeb.h"

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <thread>
#include <chrono>
#include <algorithm>
#include <regex>

#include <Wt/WBorderLayout>
#include <Wt/WBreak>
#include <Wt/WComboBox>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WScrollArea>
#include <Wt/WStackedWidget>
#include <Wt/WTextArea>
#include <Wt/WTimer>
#include <Wt/WVBoxLayout>
#include <Wt/WDialog>
#include <Wt/WPanel>
#include <bits/basic_string.h>

namespace {

    std::string uncompressGzip2(std::string gzFile) {
        std::ifstream filein(gzFile.c_str(), std::ios_base::in | std::ios_base::binary);
        boost::iostreams::filtering_streambuf<boost::iostreams::input> in;
        in.push(boost::iostreams::gzip_decompressor());
        in.push(filein);
        char tmpname[L_tmpnam];
        char * pDummy = tmpnam(tmpname);
        std::string tmpfile(tmpname);
        while (boost::filesystem::exists(tmpfile)) {
            char * pDummy = tmpnam(tmpname);
            tmpfile = std::string(tmpname);
        }
        std::ofstream out(tmpfile.c_str());
        boost::iostreams::copy(in, out);
        out.close();
        return tmpfile;
    }

    long unsigned int FindIndexFromPointer(Wt::WText * x,
            std::vector<Wt::WText*> & textvec) {
        for (long unsigned int i = 0; i < textvec.size(); i++)
            if (textvec[i] == x) return i;
    }

    void TextUnderline(Wt::WText * s, Wt::WText * e, std::vector<Wt::WText*> & v,
            Wt::WCssDecorationStyle::TextDecoration underlinetype) {
        long unsigned int is = FindIndexFromPointer(s, v);
        long unsigned int ie = FindIndexFromPointer(e, v);
        long unsigned st = (is < ie) ? is : ie;
        long unsigned en = (is < ie) ? ie : is;
        for (long unsigned int i = st; i <= en; i++)
            v[i]->decorationStyle().setTextDecoration(underlinetype);
    }

}

//[ Basic

Viewer::Viewer(UrlParameters * urlparams, Session * session, PaperAddress* pa, Wt::WContainerWidget * parent) :
pEngine_(NULL),
urlparams_(urlparams),
pCas_(NULL),
parent_(parent),
previously_highlighed() {
    //
    previously_highlighed = vector<WText*>();
    session_ = session;
    //
    rootfn_ = pa->rootdir;
    paperdir_ = pa->paperdir;
    fname_ = pa->paperfile;
    filename_ = rootfn_ + "/" + paperdir_ + "/" + fname_;
    accession_ = pa->accession;
    case_sensitive = pa->case_sensitive;
    std::vector<std::string> bepairs;
    boost::split(bepairs, pa->bestring, boost::is_any_of(" "));
    Wt::WText * benotice = NULL;
    int beupperlimit = 100;
    if (bepairs.size() > beupperlimit) {
        std::string alert = "More than " + std::to_string(beupperlimit) +
                " search matches. Restricting search highlights to " +
                std::to_string(beupperlimit) + ".";
        benotice = new Wt::WText(alert);
    }
    int imax = (bepairs.size() > beupperlimit) ? beupperlimit : bepairs.size();
    std::map<int, int> aux;
    for (int i = 0; i < imax; i++) {
        std::vector<std::string> be;
        boost::split(be, bepairs[i], boost::is_any_of(","));
        std::stringstream ssb(std::string(be[0].begin() + 1, be[0].end()));
        int32_t b(-1);
        ssb >> b;
        std::stringstream sse(std::string(be[1].begin(), be[1].end() - 1));
        int32_t e(-1);
        sse >> e;
        if ((b > -1) && (e > -1)) {
            aux.insert(std::make_pair(b, e));
        }
    }
    // keys of aux should be sorted numerically
    for (std::map<int, int>::iterator it = aux.begin(); it != aux.end(); it++) {
        searchresultspansbegin_.push_back((*it).first);
        searchresultspansend_.push_back((*it).second);
    }
    aux.clear();
    //
    dispuseroption_.value = false;
    dispuseroption_.content = false;
    dispuseroption_.graphic = true;
    dispuseroption_.term = true;
    userstatus_.displaytextoption = 1;
    userstatus_.highlightsearchresults = true;
    userstatus_.showannotations = true;
    userstatus_.currentCatBgcolor = 0;
    userstatus_.currentKeycolor = 0;
    userstatus_.currentlypointingat = 0;
    userstatus_.curationB.clear();
    userstatus_.curationE.clear();
    userstatus_.laststart = NULL;
    dispstatus_.bold = false;
    dispstatus_.italic = false;
    dispstatus_.sup = false;
    dispstatus_.sub = false;
    dispstatus_.title = false;
    dispstatus_.fontsize = 0;
    //
    readColors();
    session_->login().changed().connect(boost::bind(&Viewer::readColors, this));
    //
    (void) uima::ResourceManager::createInstance("TPCAS2TPCENTRALAE");
    CreateUimaEngine(TPCAS2TPCENTRALDESCRIPTOR);
    std::string tmpfl = uncompressGzip2(filename_);
    GetCas(tmpfl.c_str());
    boost::filesystem::remove(tmpfl);
    PrepareAnnotationIds();
    if (rawsource_ == nxml)
        papercontainer_ = DisplayAnnotationRangeNxml(firstallbeginnings_, lastallbeginnings_);
    else if (rawsource_ == pdf)
        papercontainer_ = DisplayAnnotationRangePdf(firstallbeginnings_, lastallbeginnings_);
    else if (rawsource_ == tai)
        papercontainer_ = DisplayAnnotationRangeTai(firstallbeginnings_, lastallbeginnings_);
    mainlayout_ = new Wt::WBorderLayout();
    setLayout(mainlayout_);
    //
    // C E N T E R
    annotatorContainer_ = new ConfigurableAnnotatorContainer(urlparams_, pa->rootdir, this);
    annotatorContainer_->CreateAllEmpty();
    annotatorContainer_->SignalCaClicked().connect(this, &Viewer::CAButtonClicked);
    Wt::WContainerWidget * center = new Wt::WContainerWidget();
    mainlayout_->addWidget(center, Wt::WBorderLayout::Center);
    curationpanel_ = new Wt::WPanel();
    curationpanel_->setTitle("Curation Panel");
    curationpanel_->setCollapsible(true);
    curationpanel_->setCollapsed(true);
    curationpanel_->setCentralWidget(annotatorContainer_);
    curationpanel_->centralWidget()->hide();
    center->addWidget(curationpanel_);
    //

    Wt::WContainerWidget * scrollandpaper = new Wt::WContainerWidget();
    if (searchresultspansbegin_.size() > 0) {
        Wt::WPushButton * minus = new Wt::WPushButton("<");
        Wt::WPushButton * plus = new Wt::WPushButton(">");
        plus->setToolTip("Next");
        minus->setToolTip("Previous");
        plus->setStyleClass("btn-mini");
        minus->setStyleClass("btn-mini");
        plus->setInline(true);
        minus->setInline(true);
        //
        HighlightSearchResultSpans();
        SetScrollIntoViewPoints();
        HighlightSentenceAndScrollIntoView(scrollintoviewpoints_[scrollintoviewpointsindex_]);
        plus->clicked().connect(std::bind([ = ] (){
            if (scrollintoviewpointsindex_ < scrollintoviewpoints_.size() - 1) scrollintoviewpointsindex_++;
                    HighlightSentenceAndScrollIntoView(scrollintoviewpoints_[scrollintoviewpointsindex_]);
            }));
        minus->clicked().connect(std::bind([ = ] (){
            if (scrollintoviewpointsindex_ > 0) scrollintoviewpointsindex_--;
                    HighlightSentenceAndScrollIntoView(scrollintoviewpoints_[scrollintoviewpointsindex_]);
            }));
        Wt::WText * scrollinglabel = new Wt::WText("Scroll through search results: ");
        scrollinglabel->setInline(true);
        scrollinglabel->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 150));
        scrollinglabel->decorationStyle().setBackgroundColor(Wt::WColor(240, 255, 240));
        scrollinglabel->setFloatSide(Wt::Right);
        minus->setFloatSide(Wt::Right);
        plus->setFloatSide(Wt::Right);
        scrollandpaper->addWidget(plus);
        scrollandpaper->addWidget(minus);
        scrollandpaper->addWidget(scrollinglabel);
        scrollandpaper->addWidget(new Wt::WBreak());
    }
    //
    Wt::WScrollArea * cs = new Wt::WScrollArea();
    papercontainer_->setPadding(Wt::WLength(5, Wt::WLength::Pixel));
    cs->setWidget(papercontainer_);
    cs->setScrollBarPolicy(Wt::WScrollArea::ScrollBarAsNeeded);
    cs->setMaximumSize(Wt::WLength::Auto, Wt::WLength(64, Wt::WLength::FontEx));
    scrollandpaper->addWidget(cs);
    //
    Wt::WPanel * paperpanel = new Wt::WPanel();
    paperpanel->setTitle(accession_);
    paperpanel->titleBarWidget()->decorationStyle().font().setWeight(Wt::WFont::Bold);
    paperpanel->setCollapsible(true);
    paperpanel->setCollapsed(false);
    paperpanel->setCentralWidget(scrollandpaper);
    center->addWidget(paperpanel);
    //
    // W E S T
    Wt::WContainerWidget * west = new Wt::WContainerWidget(this);

    mainlayout_->addWidget(west, Wt::WBorderLayout::West);
    SetAnnotationAndCurationBox(west);
    //
    // N O R T H
    Wt::WContainerWidget * north = new Wt::WContainerWidget(this);
    mainlayout_->addWidget(north, Wt::WBorderLayout::North);
    Wt::WContainerWidget * northnorth = new Wt::WContainerWidget();
    Wt::WContainerWidget * northsouth = new Wt::WContainerWidget();
    north->addWidget(northnorth);
    north->addWidget(northsouth);
    SetOptionsInContainer(northnorth);
    if (benotice != NULL) {
        northsouth->addWidget(benotice);
        benotice->setFloatSide(Wt::Left);
        benotice->decorationStyle().setForegroundColor(Wt::red);
    }
    for (const auto& keyword : pa->keywords) {
        HighlightKeyword(keyword);
    }
    for (const auto& category : pa->categories) {
        HighlightCategory(category);
    }
}

Viewer::~Viewer() {
    if (pEngine_ != NULL) {
        /* call collectionProcessComplete */
        pEngine_->collectionProcessComplete();
        /* Free UIMA annotator */
        pEngine_->destroy();
        delete pEngine_;
    }
    if (pCas_ != NULL) delete pCas_;
    std::vector<Wt::WText*>::iterator itt;
    for (itt = wtpt_.Contents.begin(); itt != wtpt_.Contents.end(); itt++)
        delete (*itt);
    for (itt = wtpt_.Terms.begin(); itt != wtpt_.Terms.end(); itt++)
        delete (*itt);
    for (itt = wtpt_.Values.begin(); itt != wtpt_.Values.end(); itt++)
        delete (*itt);
    std::vector<Wt::WImage*>::iterator itg;
    for (itg = wtpt_.Graphics.begin(); itg != wtpt_.Graphics.end(); itg++)
        delete (*itg);
    std::multimap<Wt::WText*, Wt::WString*>::iterator itmts;
    for (itmts = wtpt_.Pointer2Annotations.begin();
            itmts != wtpt_.Pointer2Annotations.end(); itmts++)
        delete (*itmts).second;
}
//] Basic

//[ Uima related

void Viewer::CreateUimaEngine(const char * descriptor) {
    uima::ErrorInfo errorInfo;
    pEngine_ = uima::Framework::createAnalysisEngine(descriptor, errorInfo);
    if (errorInfo.getErrorId() != UIMA_ERR_NONE) {
        std::cerr << std::endl
                << "  Error string  : "
                << uima::AnalysisEngine::getErrorIdAsCString(errorInfo.getErrorId())
                << std::endl
                << "  UIMACPP Error info:" << std::endl
                << errorInfo << std::endl;
    }
}

void Viewer::GetCas(const char * pszInputFile) {
    pCas_ = pEngine_->newCAS();
    if (pCas_ == NULL) {
        std::cerr << "pEngine_->newCAS() failed." << std::endl;
    } else {
        try {
            /* initialize from an xmicas */
            XMLCh * native = XMLString::transcode(pszInputFile);
            LocalFileInputSource fileIS(native);
            XMLString::release(&native);
            uima::XmiDeserializer::deserialize(fileIS, * pCas_, true);
        } catch (uima::Exception e) {
            uima::ErrorInfo errInfo = e.getErrorInfo();
            std::cerr << "Error " << errInfo.getErrorId() << " " << errInfo.getMessage() << std::endl;
            std::cerr << errInfo << std::endl;
        }
    }
}
//] Uima related

//[ Signals from annotatorContainer_

void Viewer::CAButtonClicked() {
    userstatus_.curationbox->EnableCurateButton();
    curationpanel_->setCollapsed(true);
    curationpanel_->centralWidget()->hide();
}
//] Signals from annotatorContainer_

//[ Display paper

Wt::WContainerWidget * Viewer::DisplayAnnotationRangeNxml(int32_t b, int32_t e, Wt::WContainerWidget * parent) {

    /*
     * 
     * annotations to consider
     * token(t), sentence(s), lexical(l), xmltag(x):
     * 
     */

    Wt::WContainerWidget * ret = new Wt::WContainerWidget(parent);
    ret->setContentAlignment(Wt::AlignJustify);
    size_t sizeinterval = sortedXmlTagAnnotationIds_.size() / NUMBEROFTHREADS;
    if (sizeinterval > 0) {
        std::vector<std::thread*> threadvec;
        typedef std::vector<XmlTag*> vpx;
        std::vector<vpx*> vvpx;
        bool moveonflag = true;
        mmiaittype itstart(sortedXmlTagAnnotationIds_.begin());
        mmiaittype itstop(itstart);
        while (moveonflag) {
            size_t counter(sizeinterval);
            while ((itstop != sortedXmlTagAnnotationIds_.end() && (counter > 0))) {
                counter--;
                itstop++;
            }
            if (itstop == sortedXmlTagAnnotationIds_.end()) moveonflag = false;
            vpx * vx = new vpx;
            vvpx.push_back(vx);
            std::thread * t = new std::thread([ = ](vpx * vx){
                mmiaittype mmit;
                for (mmit = itstart; mmit != itstop; mmit++) {
                    int32_t beginpos = (*mmit).first;
                    if ((beginpos >= b) && (beginpos < e)) {
                        std::vector<uima::ANIterator> lexannits;
                                std::set<std::string> seen;
                                int32_t endpos = (*mmit).second.get().getEndPosition();
                                std::pair<std::multimap<int32_t, uima::ANIterator>::iterator,
                                std::multimap<int32_t, uima::ANIterator>::iterator> range;
                        for (int32_t i = beginpos; i != endpos; i++) {
                            range = sortedLexAnnotationIds_.equal_range(i);
                            for (std::multimap<int32_t, uima::ANIterator>::iterator it
                                    = range.first; it != range.second; it++) {
                                uima::Type t = (*it).second.get().getType();
                                        uima::Feature f = t.getFeatureByBaseName("category");
                                        std::string s((*it).second.get().getStringValue(f).asUTF8());
                                if (seen.find(s) == seen.end()) {
                                    lexannits.push_back((*it).second);
                                            seen.insert(s);
                                }
                            }
                        }
                        XmlTag * x = new XmlTag((*mmit).second, lexannits);
                                vx->push_back(x);
                    }
                }
            }, vx);
            itstart = itstop;
            threadvec.push_back(t);
        }
        // wait for all threads to finish
        while (threadvec.size() > 0) {
            threadvec.back()->join();
            delete threadvec.back();
            threadvec.pop_back();
        }
        for (std::vector<vpx*>::iterator vvpxit = vvpx.begin(); vvpxit != vvpx.end(); vvpxit++)
            for (vpx::iterator vpxit = (*vvpxit)->begin(); vpxit != (*vvpxit)->end(); vpxit++) {
                AddXmlTagAsWidget(*vpxit, ret);
                delete *vpxit;
            }
    }
    return ret;
}

Wt::WContainerWidget * Viewer::DisplayAnnotationRangePdf(int32_t b, int32_t e, Wt::WContainerWidget * parent) {

    /*
     * 
     * annotations to consider
     * token(t), sentence(s), lexical(l), pdftag(p):
     * 
     */

    Wt::WContainerWidget * ret = new Wt::WContainerWidget(parent);
    ret->setContentAlignment(Wt::AlignJustify);

    //std::chrono::time_point<std::chrono::system_clock> bt, et;
    //bt = std::chrono::system_clock::now();
    //clock_t begin_time = clock();
    //[ PRELIMS

    struct threaddata {
        int32_t pos;
        bool istag;
        bool istoken;
        std::vector<uima::UnicodeStringRef> lexann;
        PdfTag * tag;
        PdfToken * token;
    };

    typedef std::vector<threaddata*> threaddatavec;

    size_t sizeinterval = allbeginnings_.size() / NUMBEROFTHREADS;
    if (sizeinterval == 0) return ret;

    std::vector<std::thread*> threadvec;
    std::vector<threaddatavec*> votdv;
    bool moveonflag = true;
    std::set<int32_t>::iterator itstart(allbeginnings_.begin());
    std::set<int32_t>::iterator itstop(itstart);
    while (moveonflag) {
        size_t counter(sizeinterval);
        while ((itstop != allbeginnings_.end() && (counter > 0))) {
            counter--;
            itstop++;
        }
        if (itstop == allbeginnings_.end()) moveonflag = false;
        threaddatavec * tdv = new threaddatavec;
        votdv.push_back(tdv);
        std::thread * t = new std::thread([ = ](threaddatavec * tdv){
            int32_t currentpos = 0;
            for (std::set<int32_t>::iterator siit = itstart; siit != itstop; siit++)
                if ((*siit >= b) && (*siit < e)) {
                    std::pair<mmiaittype, mmiaittype> pdftagrange = sortedPdfTagAnnotationIds_.equal_range(*siit);
                            std::pair<mmiaittype, mmiaittype> pdftokenrange = sortedPdfTokenAnnotationIds_.equal_range(*siit);
                            //            clock_t begin_time = clock();
                    for (mmiaittype tagit = pdftagrange.first; tagit != pdftagrange.second; tagit++) {
                        uima::ANIterator anit = tagit->second;
                                currentpos = anit.get().getEndPosition();
                                PdfTag * p = new PdfTag(anit);
                                threaddata * td = new threaddata;
                                td->istag = true;
                                td->istoken = false;
                                td->pos = *siit;
                                td->tag = p;
                                tdv->push_back(td);
                    }
                    if (*siit > currentpos) {
                        for (mmiaittype tagit = pdftokenrange.first; tagit != pdftokenrange.second; tagit++) {
                            uima::ANIterator anit = tagit->second;
                                    PdfToken * t = new PdfToken(anit);
                                    threaddata * td = new threaddata;
                                    td->istag = false;
                                    td->istoken = true;
                                    td->pos = *siit;
                                    td->token = t;
                                    std::set<std::string> seen;
                                    int32_t beginpos = t->GetBeginPosition();
                                    int32_t endpos = t->GetEndPosition();
                                    std::pair<std::multimap<int32_t, uima::ANIterator>::iterator,
                                    std::multimap<int32_t, uima::ANIterator>::iterator> range;
                            for (int32_t i = beginpos; i != endpos; i++) {
                                range = sortedLexAnnotationIds_.equal_range(i);
                                for (std::multimap<int32_t, uima::ANIterator>::iterator it
                                        = range.first; it != range.second; it++) {
                                    uima::Type ty = (*it).second.get().getType();
                                            uima::Feature f = ty.getFeatureByBaseName("category");
                                            std::string s((*it).second.get().getStringValue(f).asUTF8());
                                    if (seen.find(s) == seen.end()) {
                                        td->lexann.push_back((*it).second.get().getStringValue(f));
                                                seen.insert(s);
                                    }

                                }

                            }
                            tdv->push_back(td);
                        }
                    }
                }
        }, tdv);
        itstart = itstop;
        threadvec.push_back(t);
    }
    // wait for all threads to finish
    while (threadvec.size() > 0) {
        threadvec.back()->join();
        delete threadvec.back();
        threadvec.pop_back();
    }
    //] PRELIMS
    //[ Display
    int32_t currentpos = 0;
    std::vector<threaddatavec*>::iterator votdvit;
    for (votdvit = votdv.begin(); votdvit != votdv.end(); votdvit++) {
        threaddatavec * tdv = (*votdvit);
        for (threaddatavec::iterator tdvit = tdv->begin(); tdvit != tdv->end(); tdvit++) {
            threaddata * td = (*tdvit);
            int32_t indexpos = td->pos;
            if (td->istag) {
                currentpos = td->tag->GetEndPosition();
                PdfTag p = *(td->tag);
                uima::UnicodeStringRef tagtype(p.GetTagType());
                uima::UnicodeStringRef value(p.GetValue());
                if (!tagtype.isEmpty()) {
                    if (tagtype.compare("_page") == 0) {
                        dispstatus_.bold = false;
                        dispstatus_.fontsize = 0;
                        dispstatus_.italic = false;
                        dispstatus_.sub = false;
                        dispstatus_.sup = false;
                        dispstatus_.title = false;
                        ret->addWidget(new Wt::WBreak());
                        ret->addWidget(new Wt::WBreak());
                        Wt::WText * newpage = new Wt::WText(" -- (new page) -- ");
                        newpage->decorationStyle().font().setSize(Wt::WFont::XXSmall);
                        ret->addWidget(newpage);
                        ret->addWidget(new Wt::WBreak());
                        ret->addWidget(new Wt::WBreak());
                    } else if (tagtype.compare("_sbr") == 0) {
                        ret->addWidget(new Wt::WBreak());
                        ret->addWidget(new Wt::WBreak());
                        dispstatus_.sup = false;
                        dispstatus_.sub = false;
                        dispstatus_.bold = false;
                    } else if (tagtype.compare("_cr") == 0) {
                        dispstatus_.fontsize = 0;
                        dispstatus_.sup = false;
                        dispstatus_.sub = false;
                    } else if (tagtype.compare("_fnc") == 0) {
                        std::string tolowerstring(value.asUTF8());
                        boost::to_lower(tolowerstring);
                        if (tolowerstring.find("bold") != std::string::npos) {
                            if (tolowerstring.find("semibold") == std::string::npos) {
                                dispstatus_.bold = true;
                            }
                        } else {
                            dispstatus_.bold = false;
                        }
                        if (tolowerstring.find("italic") != std::string::npos) {
                            dispstatus_.italic = true;
                        } else {
                            dispstatus_.italic = false;
                        }
                    } else if (tagtype.compare("_fsc") == 0) {
                        uima::UnicodeStringRef tag("_fsc");
                        std::string numberstring = value.asUTF8();
                        int number = atoi(numberstring.c_str());
                        int valuethreshold = 15;
                        int posoffset = 3;
                        if (number > valuethreshold) {
                            if (CheckTagDownStream(tag, indexpos, false, valuethreshold, posoffset) != indexpos) {
                                // BOLD ON
                                dispstatus_.bold = true;
                            } else {
                                dispstatus_.bold = false;
                            }
                        }
                    } else if (tagtype.compare("_ydiff") == 0) {
                        uima::UnicodeStringRef tag("_ydiff");
                        std::string numberstring = value.asUTF8();
                        int number = atoi(numberstring.c_str());
                        if (number > 0) {
                            int32_t postagdown = CheckTagDownStream(tag, indexpos, false, 0, 2);
                            if (postagdown != indexpos) {
                                // **SUPER ON**
                                if (dispstatus_.sub == false) {
                                    dispstatus_.sup = true;
                                    dispstatus_.fontsize = -1;
                                } else {
                                    dispstatus_.sub = false;
                                    dispstatus_.fontsize = 0;
                                }
                            } else {
                                // **SUB OFF**"
                                dispstatus_.sub = false;
                                dispstatus_.fontsize = 0;
                            }
                        } else {
                            int32_t postagdown = CheckTagDownStream(tag, indexpos, true, 0, 2);
                            if (postagdown != indexpos) {
                                // **SUB ON**"
                                if (dispstatus_.sup == false) {
                                    dispstatus_.sub = true;
                                    dispstatus_.fontsize = -1;
                                } else {
                                    dispstatus_.sup = false;
                                    dispstatus_.fontsize = 0;
                                }
                            } else {
                                // **SUPER OFF**
                                dispstatus_.sup = false;
                                dispstatus_.fontsize = 0;
                            }
                        }
                    } else if (tagtype.compare("_image") == 0)
                        if (!value.isEmpty()) {
                            std::string aux = value.asUTF8();
                            long unsigned int i = aux.rfind("/");
                            //                            std::string location("images/" + paperdir_ + "/");
                            std::string location("images/" + paperdir_ + "/images/");
                            location += aux.substr(i + 1, aux.length() - i - 1);
                            SetImage(location, ret);
                        }
                }
                delete td->tag;
            }
            if (indexpos > currentpos) {
                if (td->istoken) {
                    PdfToken t = *(td->token);
                    Wt::WText * text = new Wt::WText;
                    std::string s = t.GetContent().asUTF8();
                    SetTextAndAnnotations(s, text, td->lexann, t.GetBeginPosition(), t.GetEndPosition());
                    ret->addWidget(text);
                    delete td->token;
                }
            }
            delete td;
        }
        delete (tdv);
    }
    //] Display
    return ret;
}

Wt::WContainerWidget * Viewer::DisplayAnnotationRangeTai(int32_t b, int32_t e, Wt::WContainerWidget * parent) {

    /*
     * 
     * annotations to consider:
     * token(t), sentence(s), lexical(l), page(p), section(c), image(i):
     * 
     */

    Wt::WContainerWidget * ret = new Wt::WContainerWidget(parent);
    ret->setContentAlignment(Wt::AlignJustify);

    struct threaddata {
        int32_t pos;
        size_t begin;
        size_t end;
        uima::UnicodeStringRef taitagtype;
        std::vector<uima::UnicodeStringRef> lexann;
        uima::UnicodeStringRef content;
        int32_t value;
        uima::UnicodeStringRef type;
        uima::UnicodeStringRef filename;
    };

    typedef std::vector<threaddata*> threaddatavec;
    size_t sizeinterval = allbeginnings_.size() / NUMBEROFTHREADS;
    if (sizeinterval == 0) return ret;

    std::vector<std::thread*> threadvec;
    std::vector<threaddatavec*> votdv;
    bool moveonflag = true;
    std::set<int32_t>::iterator itstart(allbeginnings_.begin());
    std::set<int32_t>::iterator itstop(itstart);
    while (moveonflag) {
        size_t counter(sizeinterval);
        while ((itstop != allbeginnings_.end() && (counter > 0))) {
            counter--;
            itstop++;
        }
        if (itstop == allbeginnings_.end()) moveonflag = false;
        threaddatavec * tdv = new threaddatavec;
        votdv.push_back(tdv);
        std::thread * t = new std::thread([ = ](threaddatavec * tdv){
            for (std::set<int32_t>::iterator siit = itstart; siit != itstop; siit++)
                if ((*siit >= b) && (*siit < e)) {
                    std::pair<mmiaittype, mmiaittype> taiTokenRange = sortedTaiTokenAnnotationIds_.equal_range(*siit);

                            std::pair<mmiaittype, mmiaittype> taiPageRange = sortedTaiPageAnnotationIds_.equal_range(*siit);

                            std::pair<mmiaittype, mmiaittype> taiDblBrkRange = sortedTaiDblBrkAnnotationIds_.equal_range(*siit);

                            std::pair<mmiaittype, mmiaittype> taiSectionRange = sortedTaiSectionAnnotationIds_.equal_range(*siit);

                            std::pair<mmiaittype, mmiaittype> taiImageRange = sortedTaiImageAnnotationIds_.equal_range(*siit);

                            //*****
                    for (mmiaittype tagit = taiTokenRange.first; tagit != taiTokenRange.second; tagit++) {
                        uima::ANIterator anit = tagit->second;
                                threaddata * td = new threaddata;
                                uima::Type t = anit.get().getType();
                                td->taitagtype = t.getName();
                                uima::Feature f = t.getFeatureByBaseName("content");
                                td->begin = anit.get().getBeginPosition();
                                td->end = anit.get().getEndPosition();
                        if (f.isValid()) td->content = anit.get().getStringValue(f);
                                td->pos = *siit;
                                std::set<std::string> seen;
                                int32_t beginpos = anit.get().getBeginPosition();
                                int32_t endpos = anit.get().getEndPosition();
                                std::pair<std::multimap<int32_t, uima::ANIterator>::iterator,
                                std::multimap<int32_t, uima::ANIterator>::iterator> range;
                            for (int32_t i = beginpos; i != endpos; i++) {
                                range = sortedLexAnnotationIds_.equal_range(i);
                                for (std::multimap<int32_t, uima::ANIterator>::iterator it
                                        = range.first; it != range.second; it++) {
                                    uima::Type ty = (*it).second.get().getType();
                                            uima::Feature f = ty.getFeatureByBaseName("category");
                                            std::string s((*it).second.get().getStringValue(f).asUTF8());
                                    if (seen.find(s) == seen.end()) {
                                        td->lexann.push_back((*it).second.get().getStringValue(f));
                                                seen.insert(s);
                                    }
                                }
                            }
                        tdv->push_back(td);
                    }
                    //*****
                    for (mmiaittype tagit = taiPageRange.first; tagit != taiPageRange.second; tagit++) {
                        uima::ANIterator anit = tagit->second;
                                threaddata * td = new threaddata;
                                uima::Type t = anit.get().getType();
                                td->taitagtype = t.getName();
                                uima::Feature f = t.getFeatureByBaseName("value");
                        if (f.isValid()) td->value = anit.get().getIntValue(f);
                                td->pos = *siit;
                                tdv->push_back(td);
                        }
                    //*****
                    for (mmiaittype tagit = taiDblBrkRange.first; tagit != taiDblBrkRange.second; tagit++) {
                        uima::ANIterator anit = tagit->second;
                                threaddata *td = new threaddata;
                                uima::Type t = anit.get().getType();
                                td->taitagtype = t.getName();
                                tdv->push_back(td);
                    }
                    //*****
                    for (mmiaittype tagit = taiSectionRange.first; tagit != taiSectionRange.second; tagit++) {
                        uima::ANIterator anit = tagit->second;
                                threaddata * td = new threaddata;
                                uima::Type t = anit.get().getType();
                                td->taitagtype = t.getName();
                                uima::Feature f = t.getFeatureByBaseName("type");
                        if (f.isValid()) td->type = anit.get().getStringValue(f);
                                td->pos = *siit;
                                tdv->push_back(td);
                        }
                    //*****
                    for (mmiaittype tagit = taiImageRange.first; tagit != taiImageRange.second; tagit++) {
                        uima::ANIterator anit = tagit->second;
                                threaddata * td = new threaddata;
                                uima::Type t = anit.get().getType();
                                td->taitagtype = t.getName();
                                uima::Feature f = t.getFeatureByBaseName("filename");
                        if (f.isValid()) td->filename = anit.get().getStringValue(f);
                                f = t.getFeatureByBaseName("page");
                            if (f.isValid()) td->value = anit.get().getIntValue(f);
                                    td->pos = *siit;
                                    tdv->push_back(td);
                            }
                    //*****
                }
        }, tdv);
        itstart = itstop;
        threadvec.push_back(t);
    }
    // wait for all threads to finish
    while (threadvec.size() > 0) {
        threadvec.back()->join();
        delete threadvec.back();
        threadvec.pop_back();
    }
    //] PRELIMS
    //[ Display
    //    int32_t currentpos = 0;
    std::vector<threaddatavec*>::iterator votdvit;
    for (votdvit = votdv.begin(); votdvit != votdv.end(); votdvit++) {
        threaddatavec * tdv = (*votdvit);
        for (threaddatavec::iterator tdvit = tdv->begin(); tdvit != tdv->end(); tdvit++) {
            threaddata * td = (*tdvit);

            uima::UnicodeStringRef tag = td->taitagtype;
            if (tag.compare("org.apache.uima.textpresso.token") == 0) {
                Wt::WText * text = new Wt::WText;
                std::string s = td->content.asUTF8();
                dispstatus_.bold = false;
                dispstatus_.fontsize = 0;
                dispstatus_.italic = false;
                dispstatus_.sub = false;
                dispstatus_.sup = false;
                dispstatus_.title = false;
                SetTextAndAnnotations(s, text, td->lexann, td->begin, td->end);
                ret->addWidget(text);
            }
            if (tag.compare("org.apache.uima.textpresso.page") == 0) {
                ret->addWidget(new Wt::WBreak());
                ret->addWidget(new Wt::WBreak());
                Wt::WText * newpage = new Wt::WText(" -- Page " + std::to_string(td->value) + " -- ");
                newpage->decorationStyle().font().setSize(Wt::WFont::XXSmall);
                ret->addWidget(newpage);
                ret->addWidget(new Wt::WBreak());
                ret->addWidget(new Wt::WBreak());
            }
            if (tag.compare("org.apache.uima.textpresso.dblbrk") == 0) {
                ret->addWidget(new Wt::WBreak());
                ret->addWidget(new Wt::WBreak());
            }
            if (tag.compare("org.apache.uima.textpresso.section") == 0) {
                ret->addWidget(new Wt::WBreak());
                ret->addWidget(new Wt::WBreak());
                std::string s(" -- Section " + td->type.asUTF8() + " -- ");
                Wt::WText * newsection = new Wt::WText(s);
                newsection->decorationStyle().font().setWeight(Wt::WFont::Bold);
                ret->addWidget(newsection);
                ret->addWidget(new Wt::WBreak());
                ret->addWidget(new Wt::WBreak());
            }
            if (tag.compare("org.apache.uima.textpresso.image") == 0) {
                ret->addWidget(new Wt::WBreak());
                Wt::WText * newimage = new Wt::WText(" -- Image on page " + std::to_string(td->value) + "-- ");
                newimage->decorationStyle().font().setSize(Wt::WFont::XXSmall);
                ret->addWidget(newimage);
                std::string location = "images/" + td->filename.asUTF8();
                SetImage(location, ret);
            }
            delete td;
        }
        delete (tdv);
    }
    return ret;
}

int32_t Viewer::CheckTagDownStream(uima::UnicodeStringRef & s, int32_t pos,
        bool DownStreamChangeIsPositive, int valuethreshold, int posoffset) {
    // find pos in SortedPdfTagAnnotationsIds_.
    // go posoffset PdfTags downstream, check if there is another tag s.
    // if DownStreamChangeIsPositive is true, look for 
    //   number > valuethreshold. If so return pos of downstream tag.
    // if DownstreamChangeIsPositive is false, look for
    //   number < -valuethreshold. If so return pos of downstream tag.
    // Otherwise, return pos.
    mmiaittype mmit = sortedPdfTagAnnotationIds_.find(pos);
    for (int i = 0; i < posoffset; i++) {
        mmit++;
        if (mmit != sortedPdfTagAnnotationIds_.end()) {
            PdfTag p(mmit->second);
            uima::UnicodeStringRef tagtype(p.GetTagType());
            uima::UnicodeStringRef value(p.GetValue());
            if (tagtype.compare(s) == 0) {
                std::string numberstring = value.asUTF8();
                int number = atoi(numberstring.c_str());
                if (DownStreamChangeIsPositive) {
                    if (number > valuethreshold) return mmit->first;
                } else {
                    if (number < -valuethreshold) return mmit->first;
                }
            }
            // Need to add search for a downstream _cr, as once in a while, journals are sloppy
            // and use it as a reset. Here we need it as a positive confirmation.
            if (tagtype.compare("_cr") == 0) return pos;
        } else {
            std::cerr << "ERROR in CheckYdiffDownStream: position not found." << std::endl;
        }
    }
    return pos;
}

void Viewer::AddXmlTagAsWidget(XmlTag * x, Wt::WContainerWidget * parent) {
    std::string value = x->GetValue().asUTF8();
    std::string content = x->GetContent().asUTF8();
    // set breaks
    if ((value.compare("p") == 0) || (value.compare("sec") == 0)
            || (value.find("citation") != std::string::npos)
            || (value.find("title") != std::string::npos)
            || (value.find("contrib") != std::string::npos)
            || (value.find("author") != std::string::npos)
            || (value.find("abstract") != std::string::npos)
            || (value.find("aff") != std::string::npos)
            || (value.find("fig") != std::string::npos)) {
        parent->addWidget(new Wt::WBreak());
    }
    // set display status
    if (value.find("title") != std::string::npos)
        dispstatus_.title = true;
    else if (value.compare("bold") == 0)
        dispstatus_.bold = true;
    else if (value.compare("italic") == 0)
        dispstatus_.italic = true;
    else if (value.compare("sup") == 0)
        dispstatus_.sup = true;
    else if (value.compare("sub") == 0)
        dispstatus_.sub = true;
    else if (value.compare("pcdata") != 0) {
        dispstatus_.title = false;
        if (value.compare("p") != 0) {
            Wt::WText * text = new Wt::WText(value + " ");
            if (dispuseroption_.value)
                text->show();
            else
                text->hide();
            wtpt_.Values.push_back(text);
            text->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
            text->decorationStyle().font().setSize(Wt::WFont::XSmall);
            text->setVerticalAlignment(Wt::AlignSuper);
            parent->addWidget(text);
        }
    }
    // display content of xmltag
    if (!content.empty()) {
        Wt::WText * text = new Wt::WText("(" + content + ") ");
        if (dispuseroption_.content)
            text->show();
        else
            text->hide();
        wtpt_.Contents.push_back(text);
        text->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        text->decorationStyle().font().setSize(Wt::WFont::XSmall);
        text->setVerticalAlignment(Wt::AlignSuper);
        parent->addWidget(text);
    }
    // display term of xmltag (as stored in Chunks* vector) 
    std::vector<Chunk*> chunks = x->GetChunks();
    if (!chunks.empty()) {
        std::vector<Chunk*>::iterator i;
        for (i = chunks.begin(); i != chunks.end(); i++) {
            UnicodeString usaux = (*i)->GetChunkTerm();
            std::string aux = uima::UnicodeStringRef(usaux).asUTF8();
            Wt::WText * text = new Wt::WText;
            std::vector<uima::UnicodeStringRef> lexann = (*i)->GetLexAnns();
            SetTextAndAnnotations(aux, text, lexann, (*i)->GetB(), (*i)->GetE());
            parent->addWidget(text);
        }
    }
    // display graphic
    if (value.find("graphic") != std::string::npos) {
        //        std::string location("images/" + paperdir_ + "/");
        std::string location("images/" + paperdir_ + "/images/");
        long unsigned int start = content.find("xlink:href='");
        long unsigned int end = content.find("'", start + 12);
        std::string saux = content.substr(start + 12, end - start - 12);
        location += saux;
        if (saux.find(".jpg") == std::string::npos) location += ".jpg";
        SetImage(location, parent);
    }
    // if bold, italic, sup or sub was set, then the corresponding data (pcdata)
    // have been displayed in this round. Time to reset those flags.
    if (value.compare("pcdata") == 0) {

        dispstatus_.bold = false;
        dispstatus_.italic = false;
        dispstatus_.sup = false;
        dispstatus_.sub = false;
    }
}

void Viewer::SetImage(std::string location, Wt::WContainerWidget * parent) {
    parent->addWidget(new Wt::WBreak);
    parent->addWidget(new Wt::WBreak);
    Wt::WAnchor * anchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::Url, location));
    anchor->setTarget(Wt::TargetNewWindow);
    parent->addWidget(anchor);
    Wt::WImage * img = new Wt::WImage(location, anchor);
    img->setMaximumSize(Wt::WLength(50, Wt::WLength::Percentage), Wt::WLength::Auto);
    if (dispuseroption_.graphic)
        img->show();

    else
        img->hide();
    wtpt_.Graphics.push_back(img);
    img->setAlternateText(img->imageLink().url());
    parent->addWidget(new Wt::WBreak);
    parent->addWidget(new Wt::WBreak);
}

void Viewer::SetTextAndAnnotations(std::string s, Wt::WText * text,
        std::vector<uima::UnicodeStringRef> & lexann, int32_t beginpos, int32_t endpos) {
    if (dispuseroption_.term)
        text->show();
    else
        text->hide();
    if (lexann.size() > 0) {
        if (userstatus_.showannotations)
            text->decorationStyle().setBackgroundColor(ANNOTATIONCOLOR);
        std::vector<uima::UnicodeStringRef>::iterator lait;
        std::stringstream signaltext;
        lait = lexann.begin();
        std::string aux((*lait).asUTF8());
        if (aux.substr(0, 5) != "PTCAT") {
            Wt::WString * ann = new Wt::WString(aux);
            wtpt_.Pointer2Annotations.insert(std::make_pair(text, ann));
            wtpt_.Annotation2Pointers.insert(std::make_pair(ann, text));
            signaltext << (*lait).asUTF8();
        }
        while (++lait != lexann.end()) {
            std::string aux((*lait).asUTF8());
            if (aux.substr(0, 5) != "PTCAT") {
                Wt::WString * ann = new Wt::WString((*lait).asUTF8());
                wtpt_.Pointer2Annotations.insert(std::make_pair(text, ann));
                wtpt_.Annotation2Pointers.insert(std::make_pair(ann, text));
                signaltext << ", " << (*lait).asUTF8();
            }
        }
        text->mouseWentOver().connect(boost::bind(&Viewer::SetAnnotationText,
                this, Wt::WString(signaltext.str())));
        text->mouseWentOut().connect(boost::bind(&Viewer::HideAnnotationText, this));
    }
    wtpt_.Pointer2B.insert(std::make_pair(text, beginpos));
    wtpt_.Pointer2E.insert(std::make_pair(text, endpos));
    text->setText(Wt::WString(s + " ", Wt::UTF8));
    text->clicked().connect(boost::bind(&Viewer::TextClicked, this, text));
    if (dispstatus_.bold || dispstatus_.title)
        text->decorationStyle().font().setWeight(Wt::WFont::Bold);
    if (dispstatus_.italic)
        text->decorationStyle().font().setStyle(Wt::WFont::Italic);
    if (dispstatus_.sup)
        text->setVerticalAlignment(Wt::AlignSuper);
    if (dispstatus_.sub)
        text->setVerticalAlignment(Wt::AlignSub);
    if (dispstatus_.fontsize > 0)
        text->decorationStyle().font().setSize(Wt::WFont::Large);
    else if (dispstatus_.fontsize < 0)
        text->decorationStyle().font().setSize(Wt::WFont::Small);

    else
        text->decorationStyle().font().setSize(Wt::WFont::Medium);
    wtpt_.Terms.push_back(text);
}

void Viewer::PrepareAnnotationIds() {
    uima::Type rst = pCas_->getTypeSystem().getType("org.apache.uima.textpresso.rawsource");
    uima::ANIndex allannindex = pCas_->getAnnotationIndex(rst);
    uima::ANIterator aait = allannindex.iterator();
    rawsource_ = unknown;
    aait.moveToFirst();
    uima::Type currentType = aait.get().getType();
    uima::Feature fvalue = currentType.getFeatureByBaseName("value");
    if (fvalue.isValid()) {
        uima::UnicodeStringRef uvalue = aait.get().getStringValue(fvalue);
        if (uvalue.compare("nxml") == 0)
            rawsource_ = nxml;
        else if (uvalue.compare("pdf") == 0)
            rawsource_ = pdf;
        else if (uvalue.compare("tai") == 0)
            rawsource_ = tai;
    }
    firstallbeginnings_ = -1;
    lastallbeginnings_ = -1;
    allannindex = pCas_->getAnnotationIndex();
    aait = allannindex.iterator();
    aait.moveToFirst();
    while (aait.isValid()) {
        uima::Type currentType = aait.get().getType();
        uima::UnicodeStringRef tnameref = currentType.getName();
        bool isTextpressoAnnotation = (tnameref.indexOf(UnicodeString("textpresso")) > -1);
        if (isTextpressoAnnotation) {
            bool isXmlTagAnnotation = false;
            if (rawsource_ == nxml)
                isXmlTagAnnotation = (tnameref.compare("org.apache.uima.textpresso.xmltag") == 0);
            bool isPdfTokenAnnotation = false;
            bool isPdfTagAnnotation = false;
            if (rawsource_ == pdf) {
                isPdfTokenAnnotation = (tnameref.compare("org.apache.uima.textpresso.token") == 0);
                isPdfTagAnnotation = (tnameref.compare("org.apache.uima.textpresso.pdftag") == 0);
            }
            bool isTaiTokenAnnotation = false;
            bool isTaiPageAnnotation = false;
            bool isTaiDblBrkAnnotation = false;
            bool isTaiSectionAnnotation = false;
            bool isTaiImageAnnotation = false;
            if (rawsource_ == tai) {
                isTaiTokenAnnotation = (tnameref.compare("org.apache.uima.textpresso.token") == 0);
                isTaiPageAnnotation = (tnameref.compare("org.apache.uima.textpresso.page") == 0);
                isTaiDblBrkAnnotation = (tnameref.compare("org.apache.uima.textpresso.dblbrk") == 0);
                isTaiSectionAnnotation = (tnameref.compare("org.apache.uima.textpresso.section") == 0);
                isTaiImageAnnotation = (tnameref.compare("org.apache.uima.textpresso.image") == 0);
            }
            bool isLexAnnotation = (tnameref.compare("org.apache.uima.textpresso.lexicalannotation") == 0);
            int32_t begin = aait.get().getBeginPosition();
            if (firstallbeginnings_ < 0) firstallbeginnings_ = begin;
            if (lastallbeginnings_ < 0) lastallbeginnings_ = begin;
            firstallbeginnings_ = (begin < firstallbeginnings_) ? begin : firstallbeginnings_;
            lastallbeginnings_ = (begin > lastallbeginnings_) ? begin : lastallbeginnings_;
            // all begin positions of textpresso annotations.
            // keys will be sorted.
            allbeginnings_.insert(begin);
            // all textpresso annotations.
            // keys will be sorted.
            sortedAnnotationIds_.insert(std::make_pair(begin, aait));
            if (rawsource_ == nxml)
                if (isXmlTagAnnotation)
                    // all textpresso.xmltag annotations.
                    // keys will be sorted.
                    sortedXmlTagAnnotationIds_.insert(std::make_pair(begin, aait));
            if (rawsource_ == pdf) {
                if (isPdfTokenAnnotation)
                    // all textpresso.token annotations.
                    // keys will be sorted.
                    sortedPdfTokenAnnotationIds_.insert(std::make_pair(begin, aait));
                if (isPdfTagAnnotation)
                    // all textpresso.pdftag annotations.
                    // keys will be sorted.
                    sortedPdfTagAnnotationIds_.insert(std::make_pair(begin, aait));
            }
            if (rawsource_ == tai) {
                if (isTaiTokenAnnotation)
                    sortedTaiTokenAnnotationIds_.insert(std::make_pair(begin, aait));
                if (isTaiPageAnnotation)
                    sortedTaiPageAnnotationIds_.insert(std::make_pair(begin, aait));
                if (isTaiDblBrkAnnotation)
                    sortedTaiDblBrkAnnotationIds_.insert(std::make_pair(begin, aait));
                if (isTaiSectionAnnotation)
                    sortedTaiSectionAnnotationIds_.insert(std::make_pair(begin, aait));
                if (isTaiImageAnnotation)
                    sortedTaiImageAnnotationIds_.insert(std::make_pair(begin, aait));
            }
            if (isLexAnnotation) {
                // all textpresso.lexicalannotations annotations.
                // keys will be sorted.
                // different storage method than in other cases
                // to make loading the paper faster
                int32_t end = aait.get().getEndPosition();

                for (int32_t i = begin; i < end; i++)
                    sortedLexAnnotationIds_.insert(std::make_pair(i, aait));
            }
        }
        aait.moveToNext();
    }
}
//] Display paper

//[ Display options

void Viewer::SetOptionsInContainer(Wt::WContainerWidget * w) {
    //
    Wt::WText * plusoption = new Wt::WText("+ Options");
    Wt::WText * minusoption = new Wt::WText("- Options");
    minusoption->decorationStyle().font().setSize(Wt::WFont::XSmall);
    minusoption->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, minusoption));
    minusoption->decorationStyle().setBackgroundColor(Wt::WColor(255, 200, 200));
    plusoption->decorationStyle().font().setSize(Wt::WFont::XSmall);
    plusoption->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, plusoption));
    plusoption->decorationStyle().setBackgroundColor(Wt::WColor(200, 255, 200));
    w->addWidget(plusoption);
    w->addWidget(minusoption);
    w->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * optionscontainer = new Wt::WContainerWidget();
    optionscontainer->decorationStyle().font().setSize(Wt::WFont::Small);
    optionscontainer->setContentAlignment(Wt::AlignMiddle);
    optionscontainer->setInline(true);
    w->addWidget(optionscontainer);
    //
    plusoption->clicked().connect(std::bind([ = ] (){
        plusoption->hide();
        minusoption->show();
        optionscontainer->show();
        Wt::WBorder bx;
        bx.setStyle(Wt::WBorder::Solid);
        bx.setColor(Wt::black);
        bx.setWidth(Wt::WBorder::Thin);
        w->decorationStyle().setBorder(bx);
        w->decorationStyle().setBackgroundColor(Wt::WColor(250, 250, 250));
    }));
    minusoption->clicked().connect(std::bind([ = ] (){
        plusoption->show();
        minusoption->hide();
        optionscontainer->hide();
        Wt::WBorder bx;
        w->decorationStyle().setBorder(bx);
        w->decorationStyle().setBackgroundColor(Wt::WColor(255, 255, 255));
    }));
    plusoption->show();
    minusoption->hide();
    optionscontainer->hide();
    //
    Wt::WStackedWidget * dlt = new Wt::WStackedWidget();
    dlt->setInline(true);
    dlt->insertWidget(0, new Wt::WImage("resources/icons/square-16.png"));
    dlt->insertWidget(1, new Wt::WImage("resources/icons/7-green.png"));
    dlt->insertWidget(2, new Wt::WImage("resources/icons/b-green.png"));
    dlt->insertWidget(3, new Wt::WImage("resources/icons/f-green.png"));
    dlt->setToolTip("Display level of text.");
    dlt->clicked().connect(boost::bind(&Viewer::DisplayLevelClicked, this, dlt));
    dlt->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, dlt));
    dlt->setCurrentIndex(userstatus_.displaytextoption);
    Wt::WText * tdisplay = new Wt::WText(" Text Display: ");
    optionscontainer->addWidget(tdisplay);
    optionscontainer->addWidget(dlt);
    //
    Wt::WStackedWidget * hsr = new Wt::WStackedWidget();
    hsr->setInline(true);
    hsr->insertWidget(0, new Wt::WImage("resources/icons/square-16.png"));
    hsr->insertWidget(1, new Wt::WImage("resources/icons/f-green.png"));
    hsr->setToolTip("Highlight search results.");
    hsr->clicked().connect(boost::bind(&Viewer::HighlightSearchResultsClicked, this, hsr));
    hsr->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, hsr));
    hsr->setCurrentIndex((userstatus_.highlightsearchresults) ? 1 : 0);
    Wt::WText * hsrdisplay = new Wt::WText(" ~ Highlight Search Results: ");
    optionscontainer->addWidget(hsrdisplay);
    optionscontainer->addWidget(hsr);
    //
    Wt::WStackedWidget * ad = new Wt::WStackedWidget();
    ad->setInline(true);
    ad->insertWidget(0, new Wt::WImage("resources/icons/square-16.png"));
    ad->insertWidget(1, new Wt::WImage("resources/icons/f-green.png"));
    ad->setToolTip("Show annotations in background.");
    ad->clicked().connect(boost::bind(&Viewer::DisplayAnnotationInBackgroundClicked, this, ad));
    ad->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, ad));
    ad->setCurrentIndex((userstatus_.showannotations) ? 1 : 0);
    Wt::WText * adisplay = new Wt::WText(" ~ Annotation Display: ");
    optionscontainer->addWidget(adisplay);
    optionscontainer->addWidget(ad);
    //
    Wt::WComboBox * cb = new Wt::WComboBox();
    cb->setInline(true);
    cb->setWidth(Wt::WLength(24, Wt::WLength::FontEx));
    std::multimap<Wt::WString*, Wt::WText*>::iterator it;
    std::set<Wt::WString> sorted;
    for (it = wtpt_.Annotation2Pointers.begin();
            it != wtpt_.Annotation2Pointers.end(); it++)
        sorted.insert(*(*it).first);
    for (std::set<Wt::WString>::iterator it = sorted.begin();
            it != sorted.end(); it++)
        cb->addItem(*it);
    cb->resize(Wt::WLength(24, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    cb->decorationStyle().font().setSize(Wt::WFont::Small);
    cb->activated().connect(boost::bind(&Viewer::ParticularLabelActivated, this, cb));
    Wt::WText * hpldisplay = new Wt::WText(" ~ Highlight particular labels: ");
    optionscontainer->addWidget(hpldisplay);
    optionscontainer->addWidget(cb);
    //
    keyword = new Wt::WLineEdit();
    keyword->setInline(true);
    keyword->enterPressed().connect(boost::bind(&Viewer::KeywordEntered, this, keyword));
    keyword->setEmptyText("Enter a term");
    keyword->resize(Wt::WLength(30, Wt::WLength::FontEx), Wt::WLength(2, Wt::WLength::FontEx));
    keyword->decorationStyle().font().setSize(Wt::WFont::Small);
    keyword->show();
    //
    Wt::WSuggestionPopup::Options spOptions;
    spOptions.highlightBeginTag = "<span class=\"highlight\">";
    spOptions.highlightEndTag = "</span>";
    spOptions.listSeparator = ',';
    spOptions.whitespace = " ";
    spOptions.wordSeparators = "-., \"@;";
    Wt::WSuggestionPopup * sp = new Wt::WSuggestionPopup(spOptions, optionscontainer);
    sp->forEdit(keyword);
    sp->setMaximumSize(Wt::WLength::Auto, 200);
    std::map < Wt::WString, bool> seen;
    StopWords stopwords;
    for (std::vector<Wt::WText*>::iterator it = wtpt_.Terms.begin();
            it != wtpt_.Terms.end(); it++) {
        Wt::WString t = (*it)->text().trim();
        if (!stopwords.isStopword(t.toUTF8()))
            if (!t.empty())
                if (!seen[t]) {

                    seen[t] = true;
                    sp->addSuggestion(t, t);
                }
    }
    Wt::WText * hpkdisplay = new Wt::WText(" ~ Highlight particular keywords: ");
    optionscontainer->addWidget(hpkdisplay);
    optionscontainer->addWidget(keyword);
    //
    Wt::WImage * fw = new Wt::WImage("resources/icons/control.png");
    fw->setInline(true);
    fw->clicked().connect(boost::bind(&Viewer::ForwardOrBackwardClicked, this, true));
    fw->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, fw));
    Wt::WImage * bw = new Wt::WImage("resources/icons/control-180.png");
    bw->setInline(true);
    bw->clicked().connect(boost::bind(&Viewer::ForwardOrBackwardClicked, this, false));
    bw->mouseWentOver().connect(boost::bind(&Viewer::SetCursorHand, this, bw));
    Wt::WText * nbfh = new Wt::WText(" ~ Navigate through highlights: ");
    optionscontainer->addWidget(nbfh);
    optionscontainer->addWidget(bw);
    optionscontainer->addWidget(fw);
}

void Viewer::ForwardOrBackwardClicked(bool b) {
    // bool b: false: backward, true:forward
    std::set<long unsigned int>::iterator it, nit, pit;
    if (!userstatus_.indicesofhighlighted.empty()) {
        if (userstatus_.currentlypointingat == 0) {
            userstatus_.currentlypointingat = *userstatus_.indicesofhighlighted.begin();
        } else {
            it = userstatus_.indicesofhighlighted.find(userstatus_.currentlypointingat);
            nit = boost::next(it);
            pit = boost::prior(it);
            if (it != userstatus_.indicesofhighlighted.end()) {
                if (b) {
                    if (nit != userstatus_.indicesofhighlighted.end())
                        userstatus_.currentlypointingat = *nit;
                } else {
                    if (pit != userstatus_.indicesofhighlighted.end())
                        userstatus_.currentlypointingat = *pit;

                    else
                        userstatus_.currentlypointingat = *userstatus_.indicesofhighlighted.begin();
                }
            }
        }
        Wt::WText * p = wtpt_.Terms[userstatus_.currentlypointingat];
        HighlightSentenceAndScrollIntoView(p);
    }
}

void Viewer::HighlightSearchResultSpans() {
    for (int i = 0; i < searchresultspansbegin_.size(); i++) {
        for (int j = 0; j < wtpt_.Terms.size(); j++) {
            Wt::WText * paux = wtpt_.Terms[j];
            if (wtpt_.Pointer2B[paux] <= searchresultspansend_[i])
                if (wtpt_.Pointer2E[paux] >= searchresultspansbegin_[i])
                    if (userstatus_.highlightsearchresults)
                        paux->decorationStyle().setBackgroundColor(SPANHIGHLIGHTCOLOR);
                    else
                        paux->decorationStyle().setBackgroundColor(DEFAULTBACKGROUND);
        }
    }
}

void Viewer::SetScrollIntoViewPoints() {
    for (int i = 0; i < searchresultspansbegin_.size(); i++) {
        int32_t minvalue(99999999);
        Wt::WText * minptr(NULL);
        for (int j = 0; j < wtpt_.Terms.size(); j++) {
            Wt::WText * paux = wtpt_.Terms[j];
            int32_t diff = abs(wtpt_.Pointer2B[paux] - searchresultspansbegin_[i]);
            if (diff < minvalue) {
                minvalue = diff;
                minptr = paux;
            }
        }

        if (minptr != NULL) scrollintoviewpoints_.push_back(minptr);
    }
    scrollintoviewpointsindex_ = 0;
}

void Viewer::HighlightSentenceAndScrollIntoView(Wt::WText * p) {
    for (const auto& word_to_restore : previously_highlighed) {
        word_to_restore->decorationStyle().setBackgroundColor(SPANHIGHLIGHTCOLOR);
    }
    previously_highlighed.clear();
    if (p != NULL) {
        p->doJavaScript(p->jsRef() + ".scrollIntoView()");
        for (const auto& paux : wtpt_.Terms) {
            if (wtpt_.Pointer2B[paux] <= searchresultspansend_[scrollintoviewpointsindex_] &&
                    wtpt_.Pointer2E[paux] >= searchresultspansbegin_[scrollintoviewpointsindex_] &&
                    paux->decorationStyle().backgroundColor() == SPANHIGHLIGHTCOLOR) {
                previously_highlighed.push_back(paux);
                paux->decorationStyle().setBackgroundColor(Wt::WColor(255, 200, 100));
            }
        }
    }
}

void Viewer::DisplayLevelClicked(Wt::WStackedWidget * sw) {
    userstatus_.displaytextoption++;
    userstatus_.displaytextoption =
            (userstatus_.displaytextoption > 3) ? 0 : userstatus_.displaytextoption;
    sw->setCurrentIndex(userstatus_.displaytextoption);
    //
    dispuseroption_.term = true;
    dispuseroption_.graphic = (userstatus_.displaytextoption > 0);
    dispuseroption_.value = (userstatus_.displaytextoption > 1);
    dispuseroption_.content = (userstatus_.displaytextoption > 2);
    //
    std::vector<Wt::WImage*>::iterator viit;
    for (viit = wtpt_.Graphics.begin(); viit != wtpt_.Graphics.end(); viit++)
        if (dispuseroption_.graphic)
            (*viit)->show();
        else
            (*viit)->hide();
    //
    std::vector<Wt::WText*>::iterator vtit;
    for (vtit = wtpt_.Contents.begin(); vtit != wtpt_.Contents.end(); vtit++)
        if (dispuseroption_.content)
            (*vtit)->show();
        else
            (*vtit)->hide();
    //       
    for (vtit = wtpt_.Values.begin(); vtit != wtpt_.Values.end(); vtit++)
        if (dispuseroption_.value)
            (*vtit)->show();
        else
            (*vtit)->hide();
    //       
    for (vtit = wtpt_.Terms.begin(); vtit != wtpt_.Terms.end(); vtit++)
        (*vtit)->show();
    //       
}

void Viewer::DisplayAnnotationInBackgroundClicked(Wt::WStackedWidget * ad) {
    userstatus_.showannotations = !userstatus_.showannotations;
    ad->setCurrentIndex((userstatus_.showannotations) ? 1 : 0);
    std::multimap<Wt::WText*, Wt::WString*>::iterator it;
    for (it = wtpt_.Pointer2Annotations.begin();
            it != wtpt_.Pointer2Annotations.end(); it++) {
        if (userstatus_.showannotations)
            (*it).first->decorationStyle().setBackgroundColor(ANNOTATIONCOLOR);
        else
            (*it).first->decorationStyle().setBackgroundColor(DEFAULTBACKGROUND);
    }
    if (!userstatus_.showannotations) {
        userstatus_.currentlypointingat = 0;
        userstatus_.indicesofhighlighted.clear();
        std::vector<Wt::WText*>::iterator vtit;

        for (vtit = wtpt_.Terms.begin(); vtit != wtpt_.Terms.end(); vtit++)
            (*vtit)->decorationStyle().setForegroundColor(Wt::black);
    }
}

void Viewer::HighlightSearchResultsClicked(Wt::WStackedWidget * hsr) {
    userstatus_.highlightsearchresults = !userstatus_.highlightsearchresults;
    hsr->setCurrentIndex((userstatus_.highlightsearchresults) ? 1 : 0);
    HighlightSearchResultSpans();
}

void Viewer::ParticularLabelActivated(Wt::WComboBox * cb) {
    HighlightCategory(cb->currentText().toUTF8(), true);
}

void Viewer::HighlightCategory(const std::string& category, bool scroll_to_first) {
    Wt::WString label = WString(category);
    Wt::WText * p = NULL;
    Wt::WColor c = NextCatBgColor();
    std::multimap<Wt::WString*, Wt::WText*>::iterator it;
    for (it = wtpt_.Annotation2Pointers.begin();
            it != wtpt_.Annotation2Pointers.end(); it++) {
        if (label == *(*it).first) {
            (*it).second->decorationStyle().setBackgroundColor(c);
            long unsigned int i = FindIndexFromPointer((*it).second, wtpt_.Terms);
            userstatus_.indicesofhighlighted.insert(i);
            std::vector<Wt::WText*> temp;
            for (auto &prev : previously_highlighed) {
                if (FindIndexFromPointer(prev, wtpt_.Terms) != i) {
                    temp.push_back(prev);
                }
            }
            previously_highlighed = temp;

            if (p == NULL) {
                p = (*it).second;
                userstatus_.currentlypointingat = i;
            }
        }
    }
    if (scroll_to_first) {
        p->doJavaScript(p->jsRef() + ".scrollIntoView()");
    }
}

void Viewer::KeywordEntered(WLineEdit * le) {
    HighlightKeyword(le->text().toUTF8(), true);
}

void Viewer::HighlightKeyword(std::string ss_s, bool scroll_to_first) {
    std::vector<Wt::WText*>::iterator it;
    Wt::WColor c = NextKeyColor();
    bool success = false;
    Wt::WText * p = NULL;
    for (it = wtpt_.Terms.begin(); it != wtpt_.Terms.end(); it++) {
        std::string ss_it = (*it)->text().toUTF8();
        if (!case_sensitive) {
            std::transform(ss_it.begin(), ss_it.end(), ss_it.begin(), ::tolower);
            std::transform(ss_s.begin(), ss_s.end(), ss_s.begin(), ::tolower);
        }
        ss_it.erase(ss_it.find_last_not_of(" \n\r\t") + 1);
        if (ss_it.compare(ss_s) == 0) {
            success = true;
            (*it)->decorationStyle().setForegroundColor(c);
            userstatus_.indicesofhighlighted.insert(it - wtpt_.Terms.begin());
            if (p == NULL) {
                p = *it;
                userstatus_.currentlypointingat = it - wtpt_.Terms.begin();
            }
        }
    }
    if (success) {
        if (scroll_to_first) {
            p->doJavaScript(p->jsRef() + ".scrollIntoView()");
        }
        keyword->decorationStyle().setBackgroundColor(Wt::green);
        Wt::WTimer *timer = new Wt::WTimer();
        timer->setInterval(2000);
        timer->setSingleShot(true);
        timer->timeout().connect(boost::bind(&Viewer::ResetLineEdit, this, keyword, timer));
        timer->start();
    }
}

void Viewer::ResetLineEdit(Wt::WLineEdit * le, Wt::WTimer * timer) {

    timer->stop();
    delete timer;
    //    le->setText("");
    le->decorationStyle().setBackgroundColor(Wt::white);
}
//] Display options

//[ Annotations and curation box in the West

void Viewer::SetAnnotationAndCurationBox(Wt::WContainerWidget * w) {
    displaySwitches switches;
    userstatus_.annotationbox = new AnnotationGroupBox("Annotations");
    userstatus_.curationbox = new CurationGroupBox(session_, "Curation", parent_);
    userstatus_.curationbox->CancelAllSnipletsSignal().connect(this, &Viewer::CancelAllSnipletsSignalReceived);
    userstatus_.curationbox->CancelSingleSnipletSignal().connect(this, &Viewer::CancelSingleSnipletSignalReceived);
    userstatus_.curationbox->ScrollIntoViewSignal().connect(this, &Viewer::HighlightSentenceAndScrollIntoView);
    userstatus_.curationbox->CBClicked().connect(this, &Viewer::CBButtonClicked);
    userstatus_.curationbox->SetLoginState();
    w->addWidget(userstatus_.annotationbox);
    if (switches.isNotSuppressed("login")) w->addWidget(userstatus_.curationbox);
}

void Viewer::SetAnnotationText(Wt::WString s) {

    userstatus_.annotationbox->SetAnnotationText(s);
    userstatus_.annotationbox->ShowAnnotationText();
}

void Viewer::HideAnnotationText() {

    userstatus_.annotationbox->HideAnnotationText();
}

void Viewer::CancelAllSnipletsSignalReceived(bool b) {
    // b is currently not used
    std::vector<Wt::WText*>::iterator it;
    for (it = userstatus_.curationB.begin(); it != userstatus_.curationB.end(); it++) {
        std::map<Wt::WText*, Wt::WText*>::iterator itm;
        itm = userstatus_.curationE.find(*it);
        if (itm != userstatus_.curationE.end())
            TextUnderline(*it, (*itm).second, wtpt_.Terms, (Wt::WCssDecorationStyle::TextDecoration) 0);

        else
            TextUnderline(*it, *it, wtpt_.Terms, (Wt::WCssDecorationStyle::TextDecoration) 0);
    }
    userstatus_.curationB.clear();
    userstatus_.curationE.clear();
    userstatus_.laststart = NULL;
    UpdateViewerInfoContent();
}

void Viewer::CancelSingleSnipletSignalReceived(Wt::WText * s, Wt::WText * e) {
    std::vector<Wt::WText*>::iterator it;
    it = std::find(userstatus_.curationB.begin(), userstatus_.curationB.end(), s);
    if (it != userstatus_.curationB.end())
        if (userstatus_.laststart == NULL) {
            std::map<Wt::WText*, Wt::WText*>::iterator it = userstatus_.curationE.find(s);
            if (it != userstatus_.curationE.end())
                if ((*it).second == e)
                    RemoveCurationEntry(s, e);
        } else {

            RemoveCurationEntry(s, NULL);
        }
    UpdateViewerInfoContent();
}

void Viewer::RemoveCurationEntry(Wt::WText * s, Wt::WText * e) {
    if (e == NULL) {
        TextUnderline(s, s, wtpt_.Terms, (Wt::WCssDecorationStyle::TextDecoration) 0);
        std::vector<Wt::WText*>::iterator it;
        it = std::find(userstatus_.curationB.begin(), userstatus_.curationB.end(), s);
        userstatus_.curationB.erase(it);
        userstatus_.laststart = NULL;
    } else {
        TextUnderline(s, e, wtpt_.Terms, (Wt::WCssDecorationStyle::TextDecoration) 0);
        std::map<Wt::WText*, Wt::WText*>::iterator it;
        it = userstatus_.curationE.find(s);

        if (it != userstatus_.curationE.end())
            userstatus_.curationE.erase(it);
        std::vector<Wt::WText*>::iterator it2;
        it2 = std::find(userstatus_.curationB.begin(), userstatus_.curationB.end(), s);
        userstatus_.curationB.erase(it2);
    }
}

void Viewer::CBButtonClicked() {

    userstatus_.curationbox->DisableCurateButton();
    //
    UpdateViewerInfoContent();
    annotatorContainer_->PopulateFieldsContent(userstatus_.curationbox->GetFormName());
    //
    curationpanel_->centralWidget()->show();
    curationpanel_->setCollapsed(false);
}

std::vector<Wt::WText*> Viewer::GetCurB() {

    return userstatus_.curationB;
}

std::vector<Wt::WText*> Viewer::GetCurE() {
    std::vector<Wt::WText*> eaux;
    std::vector<Wt::WText*>::iterator it;
    for (it = userstatus_.curationB.begin(); it != userstatus_.curationB.end(); it++) {
        std::map<Wt::WText*, Wt::WText*>::iterator itm;
        itm = userstatus_.curationE.find(*it);

        if (itm != userstatus_.curationE.end())
            eaux.push_back((*itm).second);
    }
    return eaux;
}

std::vector<Wt::WText*> Viewer::GetTerms() {

    return wtpt_.Terms;
}

void Viewer::UpdateViewerInfoContent() {
    std::vector<Wt::WText*> eaux;
    std::vector<Wt::WText*>::iterator it;
    for (it = userstatus_.curationB.begin(); it != userstatus_.curationB.end(); it++) {
        std::map<Wt::WText*, Wt::WText*>::iterator itm;
        itm = userstatus_.curationE.find(*it);
        if (itm != userstatus_.curationE.end())
            eaux.push_back((*itm).second);
    }
    //
    std::set<std::string> seen;
    std::string additionalannotations("");
    for (int i = 0; i < userstatus_.curationB.size(); i++) {
        Wt::WText * wp = userstatus_.curationB[i];
        int32_t b = wtpt_.Pointer2B[wp];
        int32_t e = (i < eaux.size()) ? wtpt_.Pointer2E[eaux[i]] : wtpt_.Pointer2E[wp];
        std::pair<std::multimap<int32_t, uima::ANIterator>::iterator,
                std::multimap<int32_t, uima::ANIterator>::iterator> range;
        if (b > e) std::swap(b, e);
        for (int32_t i = b; i != e; i++) {
            range = sortedLexAnnotationIds_.equal_range(i);
            for (std::multimap<int32_t, uima::ANIterator>::iterator it
                    = range.first; it != range.second; it++) {
                uima::ANIterator lexannit = (*it).second;
                std::stringstream s;
                uima::Type t = lexannit.get().getType();
                uima::Feature catterm = t.getFeatureByBaseName("term");
                uima::Feature cat = t.getFeatureByBaseName("category");
                std::string aux(lexannit.get().getStringValue(cat).asUTF8());
                if (aux.substr(0, 5) != "PTCAT") {
                    s << lexannit.get().getStringValue(catterm) << "|";
                    s << lexannit.get().getStringValue(cat);
                    if (seen.find(s.str()) == seen.end()) {
                        if (!additionalannotations.empty()) additionalannotations += " ";
                        std::stringstream finals;
                        finals << "[" << (*it).first << "|" << lexannit.get().getEndPosition() << "|";
                        finals << s.str() << "]";
                        additionalannotations += finals.str();
                        seen.insert(s.str());
                    }
                }
            }

        }
    }
    //
    ConfigurableAnnotatorContainer::viewerinfo inp = {
        userstatus_.curationB, eaux,
        wtpt_.Terms, wtpt_.Pointer2B, wtpt_.Pointer2E, filename_.c_str(),
        session_->login().user().identity("loginname"), accession_,
        additionalannotations
    };
    annotatorContainer_->PopulateViewerInfoContent(inp);
}

void Viewer::TextClicked(Wt::WText * t) {
    if (session_->login().state() != 0) {
        userstatus_.curationbox->SubmitNewWtextEntry(t);
        if (userstatus_.laststart == NULL) {
            TextUnderline(t, t, wtpt_.Terms, Wt::WCssDecorationStyle::Underline);
            userstatus_.curationB.push_back(t);
            userstatus_.laststart = t;
        } else {

            userstatus_.curationE.insert(std::make_pair(userstatus_.laststart, t));
            TextUnderline(userstatus_.laststart, t, wtpt_.Terms, Wt::WCssDecorationStyle::Underline);
            userstatus_.laststart = NULL;
        }
        UpdateViewerInfoContent();
    }
}
//] Annotations and curation box in the West

void Viewer::SetCursorHand(Wt::WWidget * w) {

    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}

Wt::WColor Viewer::NextCatBgColor() {
    userstatus_.currentCatBgcolor++;
    return colors.getCategoryBgColor(userstatus_.currentCatBgcolor - 1);
}

Wt::WColor Viewer::NextKeyColor() {
    userstatus_.currentKeycolor++;
    return colors.getKeywordColor(userstatus_.currentKeycolor - 1);
}

void Viewer::readColors() {
    // load preferences for colors
    std::string username("default");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname").toUTF8();
    colors.loadColorsFromDB(username);
}