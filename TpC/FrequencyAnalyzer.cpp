/* 
 * File:   FrequencyAnalyzer.cpp
 * Author: mueller
 * 
 * Created on April 28, 2014, 2:34 PM
 */

#include "FrequencyAnalyzer.h"
#include "StopWords.h"
#include "SpiralPainting.h"
#include "xercesc/util/XMLString.hpp"
#include "uima/xmideserializer.hpp"
#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string.hpp>
#include <bits/stl_vector.h>
#include <Wt/WPushButton>
#include <Wt/WSpinBox>

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
}

FrequencyAnalyzer::FrequencyAnalyzer(std::vector<const char*> & paperlist) {
    (void) uima::ResourceManager::createInstance("TPCAS2TPCENTRALAE");
    CreateUimaEngine(TPCAS2TPCENTRALDESCRIPTOR);
    std::vector<const char*>::iterator fit;
    for (fit = paperlist.begin(); fit != paperlist.end(); fit++) {
        std::string tmpfl = uncompressGzip2(*fit);
        pCases_.push_back(GetCas(tmpfl.c_str()));
        boost::filesystem::remove(tmpfl);
    }
    PopulateCounts();
    std::map<std::string, int>::iterator siit;
    std::multimap<int, std::string>::iterator isit;
    int catmax = (*CategoryRank_.begin()).first;
    int catmin = (*CategoryRank_.begin()).first;
    for (isit = CategoryRank_.begin(); isit != CategoryRank_.end(); isit++) {
        catmax = ((*isit).first > catmax) ? (*isit).first : catmax;
        catmin = ((*isit).first < catmin) ? (*isit).first : catmin;
    }
    for (isit = CategoryRank_.begin(); isit != CategoryRank_.end(); isit++) {
        int index = 100 * ((*isit).first - catmin) / catmax;
        std::string aux = (*isit).second + " (category)";
        NormalizedOverallRank_.insert(std::make_pair(index, aux));
    }
    int keymax = (*KeywordRank_.begin()).first;
    int keymin = (*KeywordRank_.begin()).first;
    for (isit = KeywordRank_.begin(); isit != KeywordRank_.end(); isit++) {
        keymax = ((*isit).first > keymax) ? (*isit).first : keymax;
        keymin = ((*isit).first < keymin) ? (*isit).first : keymin;
    }
    for (isit = KeywordRank_.begin(); isit != KeywordRank_.end(); isit++) {
        int index = 100 * ((*isit).first - keymin) / keymax;
        NormalizedOverallRank_.insert(std::make_pair(index, (*isit).second));
    }
}

//[ Uima related

void FrequencyAnalyzer::CreateUimaEngine(const char * descriptor) {
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

uima::CAS * FrequencyAnalyzer::GetCas(const char * pszInputFile) {
    uima::CAS * ret = pEngine_->newCAS();
    if (ret == NULL) {
        std::cerr << "pEngine_->newCAS() failed." << std::endl;
    } else {
        try {
            /* initialize from an xmicas */
            XMLCh * native = XMLString::transcode(pszInputFile);
            LocalFileInputSource fileIS(native);
            XMLString::release(&native);
            uima::XmiDeserializer::deserialize(fileIS, * ret, true);
        } catch (uima::Exception e) {
            uima::ErrorInfo errInfo = e.getErrorInfo();
            std::cerr << "Error " << errInfo.getErrorId() << " " << errInfo.getMessage() << std::endl;
            std::cerr << errInfo << std::endl;
        }
    }
    return ret;
}
//] Uima related

void FrequencyAnalyzer::PopulateCounts() {
    std::vector<uima::CAS*>::iterator casit;
    for (casit = pCases_.begin(); casit != pCases_.end(); casit++) {
        Add2CategoryCount(*casit);
        Add2KeywordCount(*casit);
    }
}

void FrequencyAnalyzer::Add2CategoryCount(uima::CAS * cas) {
    uima::ANIndex allannindex = cas->getAnnotationIndex();
    uima::ANIterator aait = allannindex.iterator();
    aait.moveToFirst();
    while (aait.isValid()) {
        uima::Type currentType = aait.get().getType();
        uima::UnicodeStringRef tnameref = currentType.getName();
        if (tnameref.compare("org.apache.uima.textpresso.lexicalannotation") == 0) {
            uima::Type t = aait.get().getType();
            uima::Feature f = t.getFeatureByBaseName("category");
            uima::UnicodeStringRef catname = aait.get().getStringValue(f);
            CategoryCount_[catname.asUTF8()]++;
        }
        aait.moveToNext();
    }
    std::map<std::string, int>::iterator siit;
    for (siit = CategoryCount_.begin(); siit != CategoryCount_.end(); siit++)
        CategoryRank_.insert(std::make_pair(siit->second, siit->first));
}

void FrequencyAnalyzer::Add2KeywordCount(uima::CAS * cas) {
    uima::ANIndex allannindex = cas->getAnnotationIndex();
    uima::ANIterator aait = allannindex.iterator();
    StopWords stopwords;
    aait.moveToFirst();
    while (aait.isValid()) {
        uima::Type currentType = aait.get().getType();
        uima::UnicodeStringRef tnameref = currentType.getName();
        if (tnameref.compare("org.apache.uima.textpresso.rawsource") == 0) {
            uima::Feature fvalue = currentType.getFeatureByBaseName("value");
            if (fvalue.isValid()) {
                uima::UnicodeStringRef uvalue = aait.get().getStringValue(fvalue);
                if (uvalue.compare("nxml") == 0)
                    rawsource_ = nxml;
                else if (uvalue.compare("pdf") == 0)
                    rawsource_ = pdf;
            }
            break;
        }
        aait.moveToNext();
    }
    aait.moveToFirst();
    while (aait.isValid()) {
        uima::Type currentType = aait.get().getType();
        uima::UnicodeStringRef tnameref = currentType.getName();
        if (tnameref.compare("org.apache.uima.textpresso.xmltag") == 0) {
            uima::Type t = aait.get().getType();
            uima::Feature f = t.getFeatureByBaseName("value");
            std::string value = aait.get().getStringValue(f).asUTF8();
            f = t.getFeatureByBaseName("term");
            uima::UnicodeStringRef term = aait.get().getStringValue(f);
            if (value.compare("pcdata") == 0)
                if (term.length() > 0) {
                    int32_t curr = 0;
                    int32_t old = 0;
                    curr = term.indexOf(' ', old);
                    while ((curr > 0) && (curr < term.length())) {
                        UnicodeString extract;
                        term.extract(old, curr - old, extract);
                        std::string txt = uima::UnicodeStringRef(extract).asUTF8();
                        boost::trim(txt);
                        txt.erase(txt.find_last_not_of(".,!?;") + 1);
                        if (txt.length() > 0)
                            if (!stopwords.isStopword(txt))
                                KeywordCount_[txt]++;
                        old = curr + 1;
                        curr = term.indexOf(' ', old);
                    }
                    // take care of last element
                    UnicodeString extract;
                    term.extract(old, term.length() - old, extract);
                    std::string txt = uima::UnicodeStringRef(extract).asUTF8();
                    boost::trim(txt);
                    txt.erase(txt.find_last_not_of(".,!?;") + 1);
                    if (txt.length() > 0)
                        if (!stopwords.isStopword(txt))
                            KeywordCount_[txt]++;
                }
        } else if (rawsource_ == pdf) {
            if (tnameref.compare("org.apache.uima.textpresso.token") == 0) {
                uima::Type t = aait.get().getType();
                uima::Feature f = t.getFeatureByBaseName("content");
                std::string content = aait.get().getStringValue(f).asUTF8();
                if (content.find("_") != 0) {
                    content.erase(content.find_last_not_of(".,!?;") + 1);
                    if (content.length() > 0)
                        if (!stopwords.isStopword(content))
                            KeywordCount_[content]++;
                }
            }
        }
        aait.moveToNext();
    }
    std::map<std::string, int>::iterator siit;
    for (siit = KeywordCount_.begin(); siit != KeywordCount_.end(); siit++)
        KeywordRank_.insert(std::make_pair(siit->second, siit->first));
}

void FrequencyAnalyzer::ShowTagSpiral(Wt::WContainerWidget * parent) {
    paintingWidget_ = new Wt::WContainerWidget;
    std::vector<std::string> aux;
    std::multimap<int, std::string>::iterator isit = NormalizedOverallRank_.end();
    while (--isit != NormalizedOverallRank_.begin()) {
        if (((*isit).second).length() > 1) {
            aux.push_back((*isit).second);
        }
    }
    if (((*isit).second).length() > 1) {
        aux.push_back((*NormalizedOverallRank_.begin()).second);
    }
    SpiralPainting * painting = new SpiralPainting(900, 900, aux, 50, paintingWidget_);
    dialog_ = new Wt::WDialog("Tag Cloud", parent);
    dialog_->contents()->addWidget(paintingWidget_);
    dialog_->setModal(false);
    dialog_->setResizable(true);
    dialog_->setClosable(true);
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", dialog_->contents());
    ok->clicked().connect(dialog_, &Wt::WDialog::accept);
    dialog_->finished().connect(this, &FrequencyAnalyzer::DialogDone);
    dialog_->show();
}

void FrequencyAnalyzer::DialogDone(Wt::WDialog::DialogCode code) {
    if (code != Wt::WDialog::Accepted) {

    } else {

    }
    dialog_->contents()->removeWidget(paintingWidget_);
    delete dialog_;
}