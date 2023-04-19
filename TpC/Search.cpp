/* 
 * File:   Search.cpp
 * Author: liyuling with contributions from valerio and mueller
 * 
 * Created on April 4, 2013, 10:20 AM
 */

#include "displaySwitches.h"
#include "Search.h"
#include "TCNavWeb.h"
#include "Preference.h"
#include "ColorSet.h"
#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WItemDelegate>
#include "MyAuthWidget.h"
#include "TextpressoCentralGlobalDefinitions.h"
#include <Wt/Auth/PasswordService>
#include <bits/stl_pair.h>
#include <boost/filesystem.hpp>
#include <boost/regex.h>
#include <chrono>
#include <thread>

#include <Wt/WServer>
#include <algorithm>
#include <string>
#include <vector>
#include <unordered_map>

#include <lucene++/LuceneHeaders.h>
#include <lucene++/CompressionTools.h>
#include "lucene/CaseSensitiveAnalyzer.h"
#include "lucene/LazySelector.h"
#include "IndexManager.h"
#include <boost/algorithm/string_regex.hpp>
#include <regex>
#include <Wt/WRadioButton>

using namespace Wt;
using namespace Lucene;
using namespace std;
using namespace tpc::index;

namespace {

    bool is_number(const std::wstring& s) {
        std::wstring::const_iterator it = s.begin();
        while (it != s.end() && std::isdigit(*it)) it++;
        return !s.empty() && it == s.end();
    }

    std::string wstring2string(const std::wstring& wstring) {
        string str(wstring.begin(), wstring.end());
        return (str);
    }

    std::string WtString2string(const Wt::WString& wstring) {
        return wstring.toUTF8();
    }

    std::string LString2string(const Lucene::String& lstring) {
        wstring w_str = lstring.c_str();
        return (wstring2string(w_str));
    }

    std::wstring LString2wstring(const Lucene::String& lstring) {
        wstring w_str = lstring.c_str();
        return w_str;
    }

    Wt::WString LString2WtString(const Lucene::String& lstring) {
        return Wt::WString(lstring.c_str());
    }

    vector<pair<int, int> > searchregex(string regex, string text) {
        vector<pair<int, int> > matches;
        boost::regex regex_to_match(regex.c_str(), boost::regex::icase);
        boost::smatch author_matches;
        int sum_length = 0;
        while (boost::regex_search(text, author_matches, regex_to_match)) {
            int pos = author_matches.position();
            int len = author_matches.length();
            const unsigned int param = 0;
            pair<int, int> pos_len = make_pair(pos + sum_length, len);
            matches.push_back(pos_len);
            text = author_matches.suffix().str();
            sum_length += pos;
            sum_length += len;
        }
        return matches;
    }

    std::vector<std::string> RemovePhrases(std::string & s) {

        std::vector<std::string> ret;
        ret.clear();
        std::string quote = "\"";
        size_t spos(0);
        while (spos != std::string::npos) {
            spos = s.find(quote, spos);
            size_t epos = s.find(quote, spos + 1);
            if (epos > spos) {
                std::string extract(s.substr(spos + 1, epos - spos - 1));
                ret.push_back(extract);
                spos = epos + 1;
            }
        }
        for (auto x : ret) {
            size_t p = s.find(quote + x + quote, 0);
            s.erase(p, x.length() + 2);
        }
        return ret;
    }

    std::vector<std::wstring> RemovePhrases(std::wstring & s) {
        std::vector<std::wstring> ret;
        ret.clear();
        std::wstring quote = L"\"";
        size_t spos(0);
        while (spos != std::wstring::npos) {
            spos = s.find(quote, spos);
            size_t epos = s.find(quote, spos + 1);
            if (epos > spos) {
                std::wstring extract(s.substr(spos + 1, epos - spos - 1));
                ret.push_back(extract);
                spos = epos + 1;
            }
        }
        for (auto x : ret) {
            size_t p = s.find(quote + x + quote, 0);
            s.erase(p, x.length() + 2);
        }
        return ret;
    }

    bool keyword_is_match(std::wstring w_word, std::wstring keyword) {
        bool ret(false);
        if (boost::iequals(w_word, keyword) && (keyword != "AND") && (keyword != "OR") && !is_number(keyword))
            ret = true;
        else {
            boost::replace_all(keyword, "*", ".+");
            boost::replace_all(keyword, "?", ".");
            if ((searchregex('^' + wstring2string(keyword) + '$', wstring2string(w_word)).size() > 0)
                    && (keyword != "AND") && (keyword != "OR") && !is_number(keyword))
                ret = true;
            else if ((searchregex("\\W" + wstring2string(keyword) + '$', wstring2string(w_word)).size() > 0)
                    && (keyword != "AND") && (keyword != "OR") && !is_number(keyword))
                ret = true;
            else if ((searchregex('^' + wstring2string(keyword) + "\\W$", wstring2string(w_word)).size() > 0)
                    && (keyword != "AND") && (keyword != "OR") && !is_number(keyword))
                ret = true;
            else if ((searchregex("\\W" + wstring2string(keyword) + "\\W", wstring2string(w_word)).size() > 0)
                    && (keyword != "AND") && (keyword != "OR") && !is_number(keyword))
                ret = true;
            else if ((searchregex("^" + wstring2string(keyword) + "\\W", wstring2string(w_word)).size() > 0)
                    && (keyword != "AND") && (keyword != "OR") && !is_number(keyword))
                ret = true;
        }
        return ret;
    }

    bool phrase_is_match(const vector<std::wstring> & subwstrings,
            const vector<std::wstring> & phrasewords, int i) {
        bool ret(true);
        for (int j = i; j < i + phrasewords.size(); j++) {
            if (!keyword_is_match(subwstrings[j], phrasewords[j - i])) {
                ret = false;
                break;
            }
        }
        return ret;
    }

}

Search::Search(UrlParameters * urlparams, Session & session, Wt::WContainerWidget * parent) :
parent_(parent),
urlparameters_(urlparams),
Wt::WContainerWidget(parent) {
    session_ = &session;
    literaturestatus_ = new Wt::WText();
    literaturestatus_->setTextFormat(TextFormat::XHTMLText);
    containerforupdates_ = new Wt::WContainerWidget();
    updatetext_ = new Wt::WText();
    containerforupdates_->addWidget(updatetext_);
    table_ = new Wt::WTable();
    table_->setHeaderCount(1);
    table_->setWidth("100%");
    table_->addStyleClass("table form-inline");
    table_->setStyleClass("table-striped");
    table_->hide();

    next_button_ = new WPushButton("Next Page");
    next_button_->setMinimumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));
    next_button_->setMaximumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));

    previous_button_ = new WPushButton("Previous Page");
    previous_button_->setMinimumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));
    previous_button_->setMaximumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));

    first_button_ = new WPushButton("First page");
    first_button_->setMinimumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));
    first_button_->setMaximumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));

    last_button_ = new WPushButton("Last page");
    last_button_->setMinimumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));
    last_button_->setMaximumSize(Wt::WLength(16, Wt::WLength::FontEx), Wt::WLength(5, Wt::WLength::FontEx));

    next_button_->clicked().connect(boost::bind(&Search::changePage, this, 1));
    previous_button_->clicked().connect(boost::bind(&Search::changePage, this, -1));
    first_button_->clicked().connect(boost::bind(&Search::changePage, this, 0));
    last_button_->clicked().connect(boost::bind(&Search::changePage, this, 999999));

    currentpage_ = 0;
    totalresults_ = 0;
    current_start_ = 0;
    current_end_ = 0;

    mainlayout_ = new Wt::WBorderLayout();

    size_text_ = new WText();
    size_text_->decorationStyle().font().setSize(Wt::WFont::Large);

    //
    // N O R T H
    north_ = new Wt::WContainerWidget();
    statusline_ = new WText("");
    north_->addWidget(statusline_);
    northInner_ = new Wt::WContainerWidget();
    northInnerLiterature = new Wt::WContainerWidget();
    northInnerAdditional = new Wt::WContainerWidget();
    northInnerButtons_ = new Wt::WContainerWidget();
    Wt::WHBoxLayout* hbox_north = new Wt::WHBoxLayout();
    Wt::WHBoxLayout* hbox_north_literature = new Wt::WHBoxLayout();
    Wt::WHBoxLayout* hbox_north_additional = new Wt::WHBoxLayout();
    Wt::WHBoxLayout* hbox_north_buttons = new Wt::WHBoxLayout();
    northInner_->setLayout(hbox_north);
    CreateSearchInterface(hbox_north);
    northInnerLiterature->setLayout(hbox_north_literature);
    northInnerAdditional->setLayout(hbox_north_additional);
    northInnerButtons_->setLayout(hbox_north_buttons);
    createLiteratureRow(hbox_north_literature);
    createAdditionalOptionsRow(hbox_north_additional);
    createSearchButtonsRow(hbox_north_buttons);
    north_->addWidget(northInner_);
    north_->addWidget(northInnerLiterature);
    north_->addWidget(northInnerAdditional);
    north_->addWidget(northInnerButtons_);
    //
    // C E N T E R
    Wt::WContainerWidget * center = new Wt::WContainerWidget();
    center->addWidget(containerforupdates_);
    center->addWidget(size_text_);
    center->addWidget(new Wt::WBreak());
    center->addWidget(table_);
    cs_ = new Wt::WScrollArea();
    cs_->setWidget(center);
    cs_->setScrollBarPolicy(Wt::WScrollArea::ScrollBarAsNeeded);
    Wt::WLength p100 = Wt::WLength(100, Wt::WLength::Percentage);
    Wt::WLength p80 = Wt::WLength(80, Wt::WLength::Percentage);
    cs_->setHeight(p80);
    cs_->setWidth(p100);

    //
    // S O U T H
    south_ = new Wt::WContainerWidget();
    Wt::WHBoxLayout* hbox_south = new Wt::WHBoxLayout();

    page_number_text_ = new WText();
    page_number_text_->decorationStyle().font().setSize(Wt::WFont::Large);

    previous_button_->hide();
    next_button_->hide();
    first_button_->hide();
    last_button_->hide();

    hbox_south->addWidget(page_number_text_);
    hbox_south->addWidget(first_button_);
    hbox_south->addWidget(previous_button_);
    hbox_south->addWidget(next_button_);
    hbox_south->addWidget(last_button_);
    south_->setLayout(hbox_south);

    //
    // W E S T 
    //  currently not used

    mainlayout_->addWidget(north_, Wt::WBorderLayout::North);
    mainlayout_->addWidget(cs_, Wt::WBorderLayout::Center);
    mainlayout_->addWidget(south_, Wt::WBorderLayout::South);
    this->setLayout(mainlayout_);
    // handle search via url parameters
    if (!urlparameters_->IsEmpty()) SearchViaUrlParameters();

    statusline_->setText("");
    statusline_->decorationStyle().setBackgroundColor(Wt::white);
    northInner_->show();
    cs_->show();
    south_->show();

    // reload colors when color palette is changed from customization form
    TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent);
    tcnw->GetCustomizationInstance()->getCustomizeColorsInstance()->signalSaveClicked().connect(
            this, &Search::updateSearchColors);
    indexManager_ = IndexManager("/usr/local/textpresso/luceneindex", "/usr/local/textpresso/tpcas");
    show_list_of_corpora_before_search_ = true;
    try {
        if (WApplication::instance()->environment().getCookie("show_list_of_corpora_before_search") == "false") {
            show_list_of_corpora_before_search_ = false;
        }
    } catch (const exception& e) {

    }
    return_to_sentence_search_ = false;
    session_->login().changed().connect(boost::bind(&Search::SessionLoginChanged, this));
    SessionLoginChanged();
}

void Search::SearchViaUrlParameters() {
    bool triggersearch = false;
    Wt::Http::ParameterValues::iterator it;
    Wt::Http::ParameterValues keywords = urlparameters_->GetParameterValues("keyword");
    if (!keywords.empty()) {
        std::string kwt(*keywords.begin());
        for (it = ++keywords.begin(); it != keywords.end(); it++)
            kwt += " " + *it;
        keywordtext_->setText(kwt);
        stored_keyword_ = keywordtext_->text().value();
        triggersearch = true;
    }
    Wt::Http::ParameterValues categories = urlparameters_->GetParameterValues("category");
    if (!categories.empty()) {
        pickedcat_.clear();
        for (it = categories.begin(); it != categories.end(); it++) {
            pickedcat_.insert(*it);
        }
        UpdatePickedCatCont();
        triggersearch = true;
    }
    Wt::Http::ParameterValues scope = urlparameters_->GetParameterValues("scope");
    if (!scope.empty())
        keywordfield_combo_->setCurrentIndex(keywordfield_combo_->findText(scope[0]));
    Wt::Http::ParameterValues literatures = urlparameters_->GetParameterValues("literature");
    if (!literatures.empty())
        CheckAgainstPickedLiteratureMap(literatures);
    Wt::Http::ParameterValues excludes = urlparameters_->GetParameterValues("exclude");
    if (!excludes.empty()) {
        std::string ext(*excludes.begin());
        for (it = ++excludes.begin(); it != excludes.end(); it++)
            ext += " " + *it;
        keywordnottext_->setText(ext);
    }
    Wt::Http::ParameterValues authors = urlparameters_->GetParameterValues("author");
    if (!authors.empty()) {
        std::string aut(*authors.begin());
        for (it = ++authors.begin(); it != authors.end(); it++)
            aut += " " + *it;
        author_filter_->setText(aut);
    }
    Wt::Http::ParameterValues journals = urlparameters_->GetParameterValues("journal");
    if (!journals.empty()) {
        std::string jot(*journals.begin());
        for (it = ++journals.begin(); it != journals.end(); it++)
            jot += " " + *it;
        journal_filter_->setText(jot);
    }
    Wt::Http::ParameterValues years = urlparameters_->GetParameterValues("year");
    if (!years.empty()) {
        std::string yet(*years.begin());
        for (it = ++years.begin(); it != years.end(); it++)
            yet += " " + *it;
        year_filter_->setText(yet);
    }
    Wt::Http::ParameterValues accessions = urlparameters_->GetParameterValues("accession");
    if (!accessions.empty()) {
        std::string act(*accessions.begin());
        for (it = ++accessions.begin(); it != accessions.end(); it++)
            act += " " + *it;
        accession_filter_->setText(act);
    }
    Wt::Http::ParameterValues types = urlparameters_->GetParameterValues("type");
    string act = boost::join(types, " ");
    type_filter_->setText(act);
    Wt::Http::ParameterValues casesensitive = urlparameters_->GetParameterValues("casesensitive");
    if (!casesensitive.empty())
        if (casesensitive[0].compare("true") == 0)
            cb_casesens_->setChecked();
    if (triggersearch)
        doSearch();
}

void Search::SetLiteratureDescription(Wt::WContainerWidget * w) {
    //
    Wt::WText * plusoption = new Wt::WText("+ Available Literature Info");
    Wt::WText * minusoption = new Wt::WText("- Available Literature Info");
    minusoption->decorationStyle().font().setSize(Wt::WFont::XSmall);
    minusoption->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, minusoption));
    minusoption->decorationStyle().setBackgroundColor(Wt::WColor(255, 200, 200));
    plusoption->decorationStyle().font().setSize(Wt::WFont::XSmall);
    plusoption->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, plusoption));
    plusoption->decorationStyle().setBackgroundColor(Wt::WColor(200, 255, 200));
    w->addWidget(plusoption);
    w->addWidget(minusoption);
    w->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * literaturecontainer = new Wt::WContainerWidget();
    literaturecontainer->decorationStyle().font().setSize(Wt::WFont::Small);
    literaturecontainer->setContentAlignment(Wt::AlignMiddle);
    literaturecontainer->setInline(true);
    w->addWidget(literaturecontainer);
    //
    plusoption->clicked().connect(std::bind([ = ] (){
        plusoption->hide();
        minusoption->show();
        literaturecontainer->show();
        SetLiteratureContainer(literaturecontainer);
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
        literaturecontainer->hide();
        Wt::WBorder bx;
        w->decorationStyle().setBorder(bx);
        w->decorationStyle().setBackgroundColor(Wt::WColor(255, 255, 255));
    }));
    plusoption->show();
    minusoption->hide();
    literaturecontainer->hide();
    //
}

void Search::SetLiteratureContainer(Wt::WContainerWidget * literaturecontainer) {
    vector<string> corpus_vec = IndexManager::get_available_corpora(CAS_ROOT_LOCATION.c_str());
    vector<string> additional_corpora;
    if (session_->login().state() != 0 && indexManager_.has_external_index()) {
        additional_corpora = indexManager_.get_external_corpora();
    }
    literaturecontainer->clear();
    literaturecontainer->addWidget(new Wt::WText(Wt::WString(
            "Current site contains " + to_string(corpus_vec.size() + additional_corpora.size()) + " literature")));
    if (corpus_vec.size() + additional_corpora.size() > 1)
        literaturecontainer->addWidget(new Wt::WText(Wt::WString("s")));
    literaturecontainer->addWidget(new Wt::WText(Wt::WString(":")));
    literaturecontainer->addWidget(new Wt::WBreak());
    for (const auto& lit : corpus_vec) {
        literaturecontainer->addWidget(new Wt::WText(Wt::WString(
                (lit) + " (" + std::to_string(indexManager_.get_num_articles_in_corpus(lit)) + " papers)")));
        literaturecontainer->addWidget(new Wt::WBreak());
    }
    for (const auto& lit : additional_corpora) {
        literaturecontainer->addWidget(new Wt::WText(Wt::WString(
                (lit) + " (" + std::to_string(indexManager_.get_num_articles_in_corpus(lit, true)) + " papers)")));
        literaturecontainer->addWidget(new Wt::WBreak());
    }
}

void Search::CreateSearchInterface(Wt::WHBoxLayout* hbox) {
    Wt::WContainerWidget * litcont = new Wt::WContainerWidget();
    SetLiteratureDescription(litcont);
    //
    Wt::WContainerWidget * scopeLocationContainer = new Wt::WContainerWidget();
    Wt::WText * label_scope = new WText("Search Scope: ");
    label_scope->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_scope->decorationStyle().font().setSize(Wt::WFont::Medium);
    scopeLocationContainer->addWidget(label_scope);
    scopeLocationContainer->addWidget(new Wt::WBreak());
    keywordfield_combo_ = new WComboBox();
    keywordfield_combo_->addItem("sentence");
    keywordfield_combo_->addItem("document");
    keywordfield_combo_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    keywordfield_combo_->decorationStyle().font().setSize(Wt::WFont::Medium);
    keywordfield_combo_->setInline(true);
    keywordfield_combo_->setWidth(WLength("50%"));
    keywordfield_combo_->setCurrentIndex(keywordfield_combo_->findText("document"));
    scopeLocationContainer->addWidget(keywordfield_combo_);
    Wt::WImage * imssc = new Wt::WImage("resources/icons/qmark15.png");
    imssc->setVerticalAlignment(Wt::AlignTop);
    imssc->setInline(true);
    imssc->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, imssc));
    imssc->clicked().connect(this, &Search::HelpScopeDialog);
    scopeLocationContainer->addWidget(imssc);
    //
    setDefTypeBtn_ = new Wt::WPushButton("set as default");
    setDefTypeBtn_->setStyleClass("btn-small");
    setDefTypeBtn_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    setDefTypeBtn_->clicked().connect(std::bind([ = ](){
        if (session_->login().state() != 0) {
            Preference *pref = new Preference(PGSEARCHPREFERENCES, PGSEARCHPREFERENCESTABLENAME,
                    session_->login().user().identity("loginname").toUTF8());
                    vector<string> prefVec = pref->GetPreferencesVec();
                    set<string> prefSet;
                    copy(prefVec.begin(), prefVec.end(), inserter(prefSet, prefSet.begin()));
            if (keywordfield_combo_->currentText().value() == "sentence") {
                prefSet.insert("search_type_sentences");
            } else {
                prefSet.erase("search_type_sentences");
            }
            prefVec.clear();
                    copy(prefSet.begin(), prefSet.end(), back_inserter(prefVec));
                    pref->SavePreferencesVector(session_->login().user().identity("loginname").toUTF8(), prefVec);
                    statusline_->setText("Preferences have been saved.");
                    statusline_->decorationStyle().setForegroundColor(Wt::green);
                    Wt::WTimer * timer = new Wt::WTimer();
                    timer->setInterval(2000);
                    timer->setSingleShot(true);
                    timer->timeout().connect(std::bind([ = ] (Wt::WTimer * timer){
                timer->stop();
                delete timer;
                statusline_->setText("");
            }, timer));
                    timer->start();
        }
    }));
    setDefTypeBtn_->hide();
    if (session_->login().state() != 0) {
        setDefTypeBtn_->show();
    }
    scopeLocationContainer->addWidget(setDefTypeBtn_);
    scopeLocationContainer->addWidget(new WBreak());
    //
    Wt::WText * label_location = new WText("Search Location: ");
    label_location->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_location->decorationStyle().font().setSize(Wt::WFont::Medium);
    scopeLocationContainer->addWidget(label_location);
    scopeLocationContainer->addWidget(new Wt::WBreak());
    searchlocation_ = new WComboBox();
    searchlocation_->addItem("abstract");
    searchlocation_->addItem("acknowledgments");
    searchlocation_->addItem("background");
    searchlocation_->addItem("beginning of article");
    searchlocation_->addItem("conclusion");
    searchlocation_->addItem("design");
    searchlocation_->addItem("discussion");
    searchlocation_->addItem("document");
    searchlocation_->addItem("introduction");
    searchlocation_->addItem("materials and methods");
    searchlocation_->addItem("references");
    searchlocation_->addItem("result");
    searchlocation_->addItem("title");
    searchlocation_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    searchlocation_->decorationStyle().font().setSize(Wt::WFont::Medium);
    searchlocation_->setInline(true);
    searchlocation_->setWidth(WLength("50%"));
    searchlocation_->setCurrentIndex(searchlocation_->findText("document"));
    scopeLocationContainer->addWidget(searchlocation_);
    keywordfield_combo_->changed().connect(std::bind([ = ](){
        if (keywordfield_combo_->currentText().value() == "sentence") {
            searchlocation_->setCurrentIndex(searchlocation_->findText("document"));
                    searchlocation_->disable();
        } else {
            searchlocation_->enable();
        }
    }));
    //
    Wt::WContainerWidget * combinedlitscope = new Wt::WContainerWidget();
    combinedlitscope->addWidget(scopeLocationContainer);
    combinedlitscope->addWidget(new Wt::WBreak());
    combinedlitscope->addWidget(litcont);
    //
    Wt::WContainerWidget * keywordcont = new Wt::WContainerWidget();
    Wt::WText * label_keyword_ = new WText("Keywords: ");
    label_keyword_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_keyword_->decorationStyle().font().setSize(Wt::WFont::Medium);
    keywordcont->addWidget(label_keyword_);
    keywordcont->addWidget(new Wt::WBreak());
    keywordtext_ = new WLineEdit(); // allow text input
    keywordtext_->setFocus();
    keywordtext_->enterPressed().connect(this, &Search::doSearch);
    keywordtext_->setWidth(WLength("70%"));
    Wt::WImage * im = new Wt::WImage("resources/icons/qmark15.png");
    im->setVerticalAlignment(Wt::AlignTop);
    keywordtext_->setInline(true);
    im->setInline(true);
    im->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, im));
    im->clicked().connect(this, &Search::HelpLuceneDialog);
    keywordcont->addWidget(keywordtext_);
    keywordcont->addWidget(im);
    //
    WText* label_casesens = new WText("Case Sensitive Search ");
    label_casesens->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_casesens->decorationStyle().font().setSize(Wt::WFont::Medium);
    cb_casesens_ = new WCheckBox();
    keywordcont->addWidget(new WBreak());
    keywordcont->addWidget(label_casesens);
    keywordcont->addWidget(cb_casesens_);
    //
    Wt::WContainerWidget * catcont = new Wt::WContainerWidget();
    Wt::WText * label_category_ = new WText("Category: ");
    label_category_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_category_->setInline(true);
    label_category_->decorationStyle().font().setSize(Wt::WFont::Medium);
    catcont->addWidget(label_category_);
    catcont->addWidget(new Wt::WBreak());
    addcat_button_ = new WPushButton("Add a Category");
    addcat_button_->setStyleClass("btn-small");
    addcat_button_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    addcat_button_->clicked().connect(boost::bind(&Search::AddCatButtonPressed, this, session_));
    catcont->addWidget(addcat_button_);
    pickedcatcont_ = new Wt::WContainerWidget();
    UpdatePickedCatCont();
    catcont->addWidget(pickedcatcont_);
    //
    //    combinedlitscope->setWidth(Wt::WLength(10,Wt::WLength::Percentage));
    //    catcont->setWidth(Wt::WLength(50,Wt::WLength::Percentage));
    //
    hbox->addWidget(combinedlitscope);
    hbox->addWidget(keywordcont);
    hbox->addWidget(catcont);
}

void Search::createLiteratureRow(Wt::WHBoxLayout* hbox) {
    corpus_button_ = new Wt::WPushButton("Select Literature");
    corpus_button_->setStyleClass("btn-small");
    corpus_button_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    corpus_button_->clicked().connect(this, &Search::SelectLiteraturePressed);
    hbox->addWidget(new Wt::WBreak());
    hbox->addWidget(corpus_button_);
    hbox->addWidget(new Wt::WBreak());
    hbox->addWidget(literaturestatus_, 1);
}

void Search::createAdditionalOptionsRow(Wt::WHBoxLayout* hbox) {
    Wt::WPanel * filterpanel = new Wt::WPanel();
    Wt::WText * fpt = new Wt::WText("Additional Search Options");
    fpt->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, fpt));
    fpt->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    fpt->decorationStyle().font().setSize(Wt::WFont::Medium);
    filterpanel->setTitleBar(true);
    filterpanel->titleBarWidget()->addWidget(fpt);
    filterpanel->titleBarWidget()->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    filterpanel->titleBarWidget()->decorationStyle().setBackgroundColor(Wt::WColor(200, 200, 200));
    filterpanel->titleBarWidget()->setHeight(Wt::WLength(5, Wt::WLength::FontEx));

    fpt->setVerticalAlignment(Wt::AlignBottom);
    Wt::WContainerWidget * filters = new Wt::WContainerWidget();
    filterpanel->setCentralWidget(filters);
    filterpanel->setCollapsible(true);
    filterpanel->setCollapsed(true);

    //
    Wt::WText * label_keywordnot_ = new WText("Exclude Keywords:  ");
    label_keywordnot_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_keywordnot_->decorationStyle().font().setSize(Wt::WFont::Medium);
    keywordnottext_ = new WLineEdit(); // allow text input
    keywordnottext_->setWidth(WLength("97%"));
    keywordnottext_->enterPressed().connect(this, &Search::doSearch);
    //
    WText* label_author_filter = new WText("Author: ");
    label_author_filter->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_author_filter->decorationStyle().font().setSize(Wt::WFont::Medium);
    author_filter_ = new WLineEdit();
    author_filter_->setWidth(WLength("97%"));
    author_filter_->enterPressed().connect(this, &Search::doSearch);
    //
    WText* label_journal_filter = new WText("Journal: ");
    label_journal_filter->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_journal_filter->decorationStyle().font().setSize(Wt::WFont::Medium);
    journal_filter_ = new WLineEdit();
    journal_filter_->setWidth(WLength("97%"));
    journal_filter_->enterPressed().connect(this, &Search::doSearch);
    //
    WText* label_year_filter = new WText("Year: ");
    label_year_filter->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_year_filter->decorationStyle().font().setSize(Wt::WFont::Medium);
    year_filter_ = new WLineEdit();
    year_filter_->setWidth(WLength("97%"));
    year_filter_->enterPressed().connect(this, &Search::doSearch);
    //
    WText* label_accession_filter = new WText("Accession: ");
    label_accession_filter->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_accession_filter->decorationStyle().font().setSize(Wt::WFont::Medium);
    Wt::WImage * imAcc = new Wt::WImage("resources/icons/qmark15.png");
    imAcc->setVerticalAlignment(Wt::AlignTop);
    imAcc->setInline(true);
    imAcc->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, imAcc));
    imAcc->clicked().connect(std::bind([ = ] (){
        Wt::WDialog *dialog = new Wt::WDialog("Info");
        dialog->contents()->addWidget(new Wt::WText("<p>Search the field containing the identifier(s) for th paper. "
        "For example, for Pubmed ids, insert \"PMID:xxxxxxx\". "
        "Other possible fields are DOI and WBPaper id.</p>"));
        dialog->contents()->addStyleClass("form-group");
        dialog->setWidth(Wt::WLength("70%"));
        Wt::WPushButton *ok = new Wt::WPushButton("Close");
        dialog->footer()->addWidget(ok);
        ok->setDefault(true);
        dialog->rejectWhenEscapePressed();
        ok->clicked().connect(std::bind([ = ]()
        {
            dialog->accept();
        }));
        dialog->show();
    }));
    accession_filter_ = new WLineEdit();
    accession_filter_->setWidth(WLength("97%"));
    accession_filter_->enterPressed().connect(this, &Search::doSearch);
    //
    WText* label_type_filter = new WText("Type of Paper: ");
    label_type_filter->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_type_filter->decorationStyle().font().setSize(Wt::WFont::Medium);
    type_filter_ = new WLineEdit();
    type_filter_->setWidth(WLength("97%"));
    type_filter_->enterPressed().connect(this, &Search::doSearch);
    //
    WText* label_note = new WText("Note: wildcard * is accepted (but not being the first letter).");
    label_note->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_note->decorationStyle().font().setSize(Wt::WFont::Medium);
    //
    WText* label_sortbyyear = new WText("Sort by Year: ");
    label_sortbyyear->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_sortbyyear->decorationStyle().font().setSize(Wt::WFont::Medium);
    cb_year_ = new Wt::WCheckBox();

    //
    filters->addWidget(label_keywordnot_);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(keywordnottext_);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_author_filter);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(author_filter_);
    filters->addWidget(new Wt::WBreak());
    WText* label_author_exact_match = new WText("  Exact Match ");
    label_author_exact_match->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_author_exact_match->decorationStyle().font().setSize(Wt::WFont::Medium);
    filters->addWidget(label_author_exact_match);
    author_exact_match = new WCheckBox();
    filters->addWidget(author_exact_match);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_journal_filter);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(journal_filter_);
    filters->addWidget(new Wt::WBreak());
    WText* label_journal_exact_match = new WText("  Exact Match ");
    label_journal_exact_match->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    label_journal_exact_match->decorationStyle().font().setSize(Wt::WFont::Medium);
    filters->addWidget(label_journal_exact_match);
    journal_exact_match = new WCheckBox();
    filters->addWidget(journal_exact_match);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_year_filter);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(year_filter_);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_accession_filter);
    filters->addWidget(imAcc);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(accession_filter_);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_type_filter);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(type_filter_);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_note);
    filters->addWidget(new Wt::WBreak());
    filters->addWidget(label_sortbyyear);
    filters->addWidget(cb_year_);
    hbox->addWidget(filterpanel);
    hbox->addWidget(new WText(""));
    hbox->addWidget(new WText(""));
    hbox->addWidget(new WText(""));
}

void Search::createSearchButtonsRow(Wt::WHBoxLayout* hbox) {
    Wt::WContainerWidget * butcont = new Wt::WContainerWidget();
    button_search_ = new WPushButton("Search!");
    button_search_->setStyleClass("btn-small");
    button_search_->clicked().connect(this, &Search::doSearch);
    button_search_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    button_search_->setInline(true);
    butcont->addWidget(button_search_);
    butcont->addWidget(new WText(" "));
    button_reset_ = new WPushButton("Reset Query!");
    button_reset_->setStyleClass("btn-small");
    button_reset_->clicked().connect(this, &Search::ResetSearch);
    button_reset_->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    button_reset_->setInline(true);
    butcont->addWidget(button_reset_);
    hbox->addWidget(butcont);
}

void Search::ResetSearch() {
    bool skip_resetui(false);
    for (auto& param : urlparameters_->GetMap()) {
        if (!param.second.empty() && urlparamset.find(param.first) != urlparamset.end()) {
            skip_resetui = true;
        }
    }
    if (!skip_resetui) {
        keywordfield_combo_->setCurrentIndex(keywordfield_combo_->findText("document"));
        searchlocation_->enable();
        if (session_->login().state() != 0) {
            std::string username("");
            username = session_->login().user().identity("loginname").toUTF8();
            Preference * pref = new Preference(PGSEARCHPREFERENCES, PGSEARCHPREFERENCESTABLENAME, username);
            if (pref->HasPreferences())
                if (pref->IsPreference("search_type_sentences")) {
                    keywordfield_combo_->setCurrentIndex(keywordfield_combo_->findText("sentence"));
                    searchlocation_->disable();
                }
        }
        searchlocation_->setCurrentIndex(searchlocation_->findText("document"));
        keywordtext_->setText("");
        stored_keyword_ = L"";
        keywordnottext_->setText("");
        accession_filter_->setText("");
        type_filter_->setText("");
        author_filter_->setText("");
        journal_filter_->setText("");
        year_filter_->setText("");
        pickedcat_.clear();
    }
    hideTable();
    UpdatePickedCatCont();
}

void Search::SelectLiteraturePressed() {
    corpus_button_->disable();
    pl_ = new PickLiterature(session_, pickedliterature_);
    pl_->finished().connect(this, &Search::PickLiteratureDialogDone);
    pl_->show();
}

void Search::PickLiteratureDialogDone(Wt::WDialog::DialogCode code) {
    if (code == Wt::WDialog::Accepted) {
        std::map < std::string, bool>::iterator it;
        for (auto& lit : pickedliterature_)
            lit.second = pl_->GetCheckState(lit.first);
    }
    if (pl_->SaveBoxIsChecked()) {
        if (session_->login().state() != 0) {
            std::string username("");
            username = session_->login().user().identity("loginname").toUTF8();
            Preference * pref = new Preference(PGLITERATURE, PGLITPREFTABLENAME, username);
            std::set<std::string> lits;
            for (const auto& lit : pickedliterature_) {
                if (lit.second) {
                    lits.insert(lit.first);
                }
            }
            pref->SavePreferences(username, lits);
            delete pref;
        }
    }
    delete pl_;
    UpdateLiteratureStatus();
    corpus_button_->enable();
}

void Search::UpdatePickedCatCont() {
    pickedcatcont_->clear();
    pickedcatcont_->setInline(true);
    pickedcatcont_->setContentAlignment(Wt::AlignMiddle);
    if (!pickedcat_.empty()) {
        Wt::WText * labelpcc = new Wt::WText("Picked Categories:");
        labelpcc->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        labelpcc->decorationStyle().font().setSize(Wt::WFont::Medium);
        pickedcatcont_->addWidget(labelpcc);
        std::set<std::string>::iterator it;
        int count = 0;
        int maxl(0);
        for (it = pickedcat_.begin(); (it != pickedcat_.end()) && (count++ < 3); it++) {
            maxl = ((*it).length() > maxl) ? (*it).length() : maxl;
            pickedcatcont_->addWidget(new Wt::WBreak());
            std::string aux = (*it);
            string aux_title(aux);
            if (aux.substr(0, 5) == "PTCAT") aux_title += " (incl. children)";
            boost::replace_first(aux_title, "PTCAT", "");
            //
            Wt::WImage * im = new Wt::WImage("resources/icons/cancel.png");
            im->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, im));
            im->clicked().connect(std::bind([ = ](){
                std::set<std::string>::iterator fit(pickedcat_.find(aux));
                pickedcat_.erase(fit);
                UpdatePickedCatCont();
            }));
            pickedcatcont_->addWidget(im);
            //
            Wt::WText * t = new Wt::WText(aux_title);
            t->decorationStyle().font().setSize(Wt::WFont::Medium);
            t->setWordWrap(false);
            pickedcatcont_->addWidget(t);
        }
        if (count > 3) {
            Wt::WText * t = new Wt::WText("...and more.");
            t->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
            t->decorationStyle().font().setSize(Wt::WFont::Medium);
            t->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 150));
            t->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, t));
            t->clicked().connect(this, &Search::ListAllCats);
            pickedcatcont_->addWidget(new Wt::WBreak());
            pickedcatcont_->addWidget(t);
        }
        if (pickedcat_.size() > 1) {
            Wt::WText * catmatch = new Wt::WText();
            catmatch->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
            catmatch->decorationStyle().font().setSize(Wt::WFont::Small);
            catmatch->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 0));
            Wt::WString aux;
            if (categoriesanded_)
                aux = "ALL categories have ";

            else
                aux = "AT LEAST ONE category has ";
            aux += "to be matched. ";
            catmatch->setText(aux);
            pickedcatcont_->addWidget(new Wt::WBreak());
            pickedcatcont_->addWidget(new Wt::WBreak());
            pickedcatcont_->addWidget(catmatch);
            Wt::WText * toggle = new Wt::WText("Toggle");
            toggle->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, toggle));
            toggle->clicked().connect(std::bind([ = ](){
                categoriesanded_ = !categoriesanded_;
                UpdatePickedCatCont();
            }));
            //            toggle->setInline(true);
            toggle->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
            toggle->decorationStyle().font().setSize(Wt::WFont::Small);
            toggle->decorationStyle().setBackgroundColor(Wt::WColor(220, 220, 220));
            toggle->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 0));
            pickedcatcont_->addWidget(toggle);
        }
        maxl = maxl + maxl / 10;
        pickedcatcont_->setWidth(Wt::WLength(maxl, Wt::WLength::FontEx));
    }
}

void Search::HelpCurationCheckBoxDialog() {
    helpcurationcheckboxdialog_ = new Wt::WDialog("How to View a Paper");
    new Wt::WText("Check the paper you want to view. ", helpcurationcheckboxdialog_->contents());
    new Wt::WText("Then hit the 'View selected paper!' button in the column header. ", helpcurationcheckboxdialog_->contents());
    new Wt::WText("Uncheck a selection first before selecting a different choice ", helpcurationcheckboxdialog_->contents());
    new Wt::WText("as only one paper at a time can be selected. ", helpcurationcheckboxdialog_->contents());
    helpcurationcheckboxdialog_->contents()->addWidget(new Wt::WBreak());
    helpcurationcheckboxdialog_->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", helpcurationcheckboxdialog_->contents());
    ok->clicked().connect(helpcurationcheckboxdialog_, &Wt::WDialog::accept);
    helpcurationcheckboxdialog_->finished().connect(this, &Search::HelpCurationCheckBoxDialogDone);
    helpcurationcheckboxdialog_->show();
}

void Search::HelpCurationCheckBoxDialogDone(Wt::WDialog::DialogCode code) {
    delete helpcurationcheckboxdialog_;
}

void Search::HelpLuceneDialog() {
    helplucenedialog_ = new Wt::WDialog("Lucene Query Language");
    helplucenedialog_->setClosable(true);
    new Wt::WText("Textpresso search engine is based on Lucene and "
            "follows its query language as well as the scoring method for "
            "the search results. In this example, let\'s assume the user is "
            "interested in plant defense. She could type the words plant "
            "and defense in the keyword text field and choose the document scope. "
            "That means Textpresso searches the words plant or defense in each "
            "document. The scoring method is tf*idf (see ", helplucenedialog_->contents());
    Wt::WAnchor * aw = new Wt::WAnchor("https://en.wikipedia.org/wiki/Tf%E2%80%93idf", helplucenedialog_->contents());
    aw->setText("Wikipedia ");
    aw->setTarget(Wt::TargetNewWindow);
    new Wt::WText("for an explanation how it works). The word plant is probably "
            "a very common word in the biomedical literature, while the word "
            "defense probably not so much. This affects how the documents are scored. "
            "One probably gets better results by searching for the phrase plant "
            "defense by typing \"plant defense\" (with quotes) in the keyword "
            "text box. This search guarantees the phrase to be present somewhere in "
            "the document. Another possibility is typing", helplucenedialog_->contents());
    new Wt::WBreak(helplucenedialog_->contents());
    new Wt::WBreak(helplucenedialog_->contents());
    new Wt::WText("plant AND defense", helplucenedialog_->contents());
    new Wt::WBreak(helplucenedialog_->contents());
    new Wt::WBreak(helplucenedialog_->contents());
    new Wt::WText("in the box (AND capitalized). This requires Lucene to find "
            "both words to be in the document, just not right next to "
            "each other. Finally, one could also do all these searches on a"
            " sentence scope level. It requires the matches to be present in a "
            "sentence, and not in a document. In this case, unless one "
            "restricts the number of literatures to be searched significantly, "
            "these searches take a considerably longer time. ", helplucenedialog_->contents());
    new Wt::WBreak(helplucenedialog_->contents());
    new Wt::WBreak(helplucenedialog_->contents());
    new Wt::WText("More information can be found in a ", helplucenedialog_->contents());
    Wt::WAnchor * at = new Wt::WAnchor("https://www.drupal.org/node/375446", helplucenedialog_->contents());
    at->setText("tutorial ");
    at->setTarget(Wt::TargetNewWindow);
    new Wt::WText("or by searching for a preferred documentation via ", helplucenedialog_->contents());
    Wt::WAnchor * ag = new Wt::WAnchor("https://www.google.com/?gws_rd=ssl#q=lucene+query+language", helplucenedialog_->contents());
    ag->setText("Google");
    ag->setTarget(Wt::TargetNewWindow);
    new Wt::WText(".", helplucenedialog_->contents());
    helplucenedialog_->contents()->addWidget(new Wt::WBreak());
    helplucenedialog_->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", helplucenedialog_->contents());
    ok->clicked().connect(helplucenedialog_, &Wt::WDialog::accept);
    helplucenedialog_->finished().connect(this, &Search::HelpLuceneDialogDone);
    helplucenedialog_->show();
}

void Search::HelpLuceneDialogDone(Wt::WDialog::DialogCode code) {
    delete helplucenedialog_;
}

void Search::HelpScopeDialog() {
    helpscopedialog_ = new Wt::WDialog("Search Scope");
    helpscopedialog_->setWidth(Wt::WLength(80, Wt::WLength::FontEx));
    Wt::WText * t1 = new Wt::WText("For sentence scope all search items (for example, two or more keywords, or a "
            "combination of keywords and categories) need to be found in the same sentence. ", helpscopedialog_->contents());
    t1->setWordWrap(true);
    Wt::WText * t2 = new Wt::WText("For document scope all search items "
            "only need to be found in the search location specified. ", helpscopedialog_->contents());
    t2->setWordWrap(true);
    helpscopedialog_->contents()->addWidget(new Wt::WBreak());
    helpscopedialog_->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", helpscopedialog_->contents());
    ok->clicked().connect(helpscopedialog_, &Wt::WDialog::accept);
    helpscopedialog_->finished().connect(this, &Search::HelpLuceneDialogDone);
    helpscopedialog_->show();
}

void Search::HelpLongSentenceDialog() {
    WDialog* helpLongSentDialog = new Wt::WDialog("Long Sentences");
    helpLongSentDialog->setWidth(Wt::WLength(80, Wt::WLength::FontEx));
    Wt::WText *t1 = new Wt::WText("Long sentences are truncated after 50 words. Click the \"show more ..\" button at "
            "the end of the sentence to see the full text.",
            helpLongSentDialog->contents());
    t1->setWordWrap(true);
    helpLongSentDialog->contents()->addWidget(new Wt::WBreak());
    helpLongSentDialog->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton *ok = new Wt::WPushButton("Ok", helpLongSentDialog->contents());
    ok->clicked().connect(helpLongSentDialog, &Wt::WDialog::accept);
    helpLongSentDialog->finished().connect(std::bind([ = ] (){
        delete helpLongSentDialog;
    }));
    helpLongSentDialog->show();
}

/*!
 * create a new dialog widget with the provided text, with search keywords and categories highlighted
 *
 * @param title the dialog title
 * @param rawText the raw text to be highlighted with keywords and categories and inserted in the dialog
 * @param categories a string of tab separated categories for highlightling
 */
void Search::ShowLongSentenceInDialog(string title, wstring rawText, wstring categories) {
    auto wt_sentence = getSingleSentenceHighlightedWidgetFromText(rawText, categories, getCleanKeywords(),
            0, nullptr);
    WDialog* longSentenceDialog = new WDialog(title);
    longSentenceDialog->contents()->addWidget(wt_sentence);
    longSentenceDialog->contents()->addWidget(new WBreak());
    Wt::WPushButton *ok = new Wt::WPushButton("Ok", longSentenceDialog->contents());
    ok->clicked().connect(longSentenceDialog, &Wt::WDialog::accept);
    longSentenceDialog->enterPressed().connect(longSentenceDialog, &Wt::WDialog::accept);
    longSentenceDialog->finished().connect(std::bind([ = ](){delete longSentenceDialog;}));
    longSentenceDialog->setWidth(Wt::WLength(100, Wt::WLength::FontEx));
    ok->setDefault(true);
    longSentenceDialog->rejectWhenEscapePressed();
    longSentenceDialog->show();
}

void Search::ListAllCats() {
    listallcatsdialog_ = new WDialog("Selected Categories");
    std::set<std::string>::iterator it;
    int maxl(0);
    for (it = pickedcat_.begin(); it != pickedcat_.end(); it++) {
        std::string aux = (*it);
        if (aux.substr(0, 5) == "PTCAT") aux += " (incl. children)";
        boost::replace_first(aux, "PTCAT", "");
        maxl = (aux.length() > maxl) ? aux.length() : maxl;
        //
        Wt::WImage * im = new Wt::WImage("resources/icons/cancel.png");
        //im->setInline(true);
        im->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, im));
        im->clicked().connect(std::bind([ = ](){
            std::set<std::string>::iterator fit(pickedcat_.find(aux));
            pickedcat_.erase(fit);
            UpdatePickedCatCont();
            ListAllCatsDone(Wt::WDialog::DialogCode::Accepted);
            ListAllCats();
        }));
        listallcatsdialog_->contents()->addWidget(im);
        //
        Wt::WText * t = new Wt::WText(aux);
        //t->setInline(true);
        listallcatsdialog_->contents()->addWidget(t);
        new Wt::WBreak(listallcatsdialog_->contents());
    }
    Wt::WPushButton *ok = new Wt::WPushButton("Ok", listallcatsdialog_->contents());
    // these events will accept() the Dialog
    ok->clicked().connect(listallcatsdialog_, &Wt::WDialog::accept);
    listallcatsdialog_->finished().connect(this, &Search::ListAllCatsDone);
    listallcatsdialog_->show();
    maxl += maxl / 10;
    listallcatsdialog_->setWidth(Wt::WLength(maxl, Wt::WLength::FontEx));

}

void Search::ListAllCatsDone(Wt::WDialog::DialogCode code) {
    delete listallcatsdialog_;
}

void Search::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}

void Search::AddCatButtonPressed(Session * session) {
    addcat_button_->disable();
    ReadPreloadedCategories();
    pc_ = new PickCategory(session, preloadedcategories_);
    pc_->finished().connect(this, &Search::PickCategoryDialogDone);
    pc_->show();
}

void Search::PickCategoryDialogDone(Wt::WDialog::DialogCode code) {
    if (code == Wt::WDialog::Accepted) {
        std::set<Wt::WString> list(pc_->GetSelected());
        std::set<Wt::WString>::iterator it;

        for (it = list.begin(); it != list.end(); it++)
            pickedcat_.insert((*it).toUTF8());
        categoriesanded_ = pc_->CatAnded();
        UpdatePickedCatCont();
    }
    delete pc_;
    addcat_button_->enable();
}

void Search::SimpleKeywordSearchApi(Wt::WString text) {
    keywordtext_->setText(text);
    keywordfield_combo_->setCurrentIndex(keywordfield_combo_->findText("document"));
    searchlocation_->setCurrentIndex(searchlocation_->findText("document"));
    if (session_->login().state() != 0) {
        std::string username("");
        username = session_->login().user().identity("loginname").toUTF8();
        Preference * pref = new Preference(PGSEARCHPREFERENCES, PGSEARCHPREFERENCESTABLENAME, username);
        if (pref->HasPreferences())
            if (pref->IsPreference("search_type_sentences")) {
                keywordfield_combo_->setCurrentIndex(keywordfield_combo_->findText("sentence"));
                searchlocation_->disable();
            }
    }
    Wt::Http::ParameterValues literatures;
    doSearch();
}

void Search::DoSearchUpdates() {
    updatetext_->decorationStyle().font().setSize(Wt::WFont::Small);
    updatetext_->decorationStyle().setBackgroundColor(Wt::green);
    updatetimer_->stop();
    if (searchstatus_ == 0) {
        updatetext_->setText("Searching ...");
        searchstatus_++;
        updatetimer_->start();
    } else if (searchstatus_ == 1) {
        prepareKeywordColorsForSearch();
        readPreloadedColors();
        currentpage_ = 0;
        totalresults_ = 0;
        current_start_ = 0;
        current_end_ = 0;
        query_ = getSearchQuery();
        searchResults_ = indexManager_.search_documents(query_, true);
        et_ = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = et_ - bt_;
        partial_search_time_ = elapsed_seconds.count();
        updatetext_->setText("Index lookup took " + std::to_string(elapsed_seconds.count()) + " seconds.");
        searchstatus_++;
        updatetimer_->start();
    } else if (searchstatus_ == 2) {
        if (query_.type == QueryType::sentence &&
                searchResults_.partialIndexMatches.size() > NUM_SENTENCES_HITS_WARN_THRESHOLD) {
            Wt::WDialog *dialog = new Wt::WDialog("Warning");
            dialog->contents()->addWidget(new Wt::WText("<p>Your search returned a very large number of sentences.</p>"
                    "<p>Processing these sentences may take a lot of time.</p>"
                    "<p>Are you sure you want to continue?</p>"));
            dialog->contents()->addStyleClass("form-group");
            dialog->setWidth(Wt::WLength("70%"));
            Wt::WPushButton *ok = new Wt::WPushButton("Ok");
            dialog->footer()->addWidget(ok);
            ok->setDefault(true);
            Wt::WPushButton *reject = new Wt::WPushButton("Cancel");
            dialog->footer()->addWidget(reject);
            dialog->rejectWhenEscapePressed();
            ok->clicked().connect(std::bind([ = ](){
                dialog->accept();
                updatetext_->setText("Combining results ...");
                searchstatus_++;
                updatetimer_->start();
            }));
            reject->clicked().connect(std::bind([ = ](){
                dialog->reject();
                delete updatetimer_;
                searchstatus_ = 0;
                updatetext_->decorationStyle().setBackgroundColor(Wt::white);
                updatetext_->setText("Search canceled.");
            }));
            dialog->show();
        } else {
            updatetext_->setText("Combining results ...");
            searchstatus_++;
            updatetimer_->start();
        }
    } else if (searchstatus_ == 3) {
        bt_ = std::chrono::system_clock::now();
        searchResults_ = indexManager_.search_documents(query_, false,{}, searchResults_);
        et_ = std::chrono::system_clock::now();
        searchstatus_++;
        updatetimer_->start();
    } else {
        long hits = searchResults_.query.type != QueryType::sentence ? searchResults_.hit_documents.size() :
                searchResults_.total_num_sentences;
        long size = searchResults_.hit_documents.size();
        totalresults_ = size;
        int total_page = 0;
        if (totalresults_ % RECORDS_PER_PAGE == 0) {
            total_page = totalresults_ / RECORDS_PER_PAGE;
        } else {
            total_page = totalresults_ / RECORDS_PER_PAGE + 1;
        }
        std::string matchtext("There were matches in " + std::to_string(size) +
                "  documents");
        if (keywordfield_combo_->currentText().value() == "sentence") {
            if (hits < 1000000) {
                matchtext += " (" + std::to_string(hits) + " sentences)";
            } else {
                matchtext += " (results were limited to the first " + std::to_string(hits) + " sentences)";
            }
        }
        matchtext += ".";
        size_text_->setText(matchtext);
        page_number_text_->setText("Page: " + std::to_string(currentpage_ + 1) +
                " of " + std::to_string(total_page));
        displayTable(0, 0, 1);
        et_ = std::chrono::system_clock::now();
        std::chrono::duration<double> elapsed_seconds = et_ - bt_;
        std::string aux("Index lookup took " + std::to_string(partial_search_time_) + " seconds. Combining results "
                "took " +
                std::to_string(elapsed_seconds.count()) + " seconds. ");
        updatetext_->decorationStyle().setBackgroundColor(Wt::white);
        updatetext_->setText(aux);
        if (return_to_sentence_search_) {
            keywordfield_combo_->setValueText("sentence");
            return_to_sentence_search_ = false;
        }
        delete updatetimer_;
    }
}

bool Search::isSearchFormValid() {
    bool atLeastOneAdditional = getFilters()["year"] != L"" || getFilters()["author"] != L"" ||
            getFilters()["journal"] != L"" || getFilters()["accession"] != L"" || getFilters()["type"] != L"";
    bool isDocSearch = keywordfield_combo_->currentText().value() == "document";
    bool atLeastOneCat = !pickedcat_.empty();
    bool atLeastOneKeyword = !StringUtils::toString(keywordtext_->text().value()).empty();
    if (atLeastOneAdditional && !isDocSearch && !atLeastOneCat && !atLeastOneKeyword &&
            keywordfield_combo_->currentText().value() == "sentence") {
        keywordfield_combo_->setValueText("document");
        return_to_sentence_search_ = true;
        isDocSearch = true;
    }
    return (isDocSearch && (atLeastOneAdditional or atLeastOneCat or atLeastOneKeyword)) ||
            (!isDocSearch && (atLeastOneCat || atLeastOneKeyword));
}

void Search::doSearch() {
    if (isSearchFormValid()) {
        stored_keyword_ = keywordtext_->text().value();
        startSearchProcess();
    } else {
        Wt::WDialog *dialog = new Wt::WDialog("Error");
        dialog->contents()->addWidget(new Wt::WText("<p>Searches requires at least a keyword, an additional field or a "
                "category.</p>"));
        dialog->contents()->addStyleClass("form-group");
        dialog->setWidth(Wt::WLength("50%"));
        Wt::WPushButton *ok = new Wt::WPushButton("Close");
        dialog->footer()->addWidget(ok);
        ok->setDefault(true);
        dialog->rejectWhenEscapePressed();
        ok->clicked().connect(std::bind([ = ](){
            dialog->accept();
        }));
        dialog->show();
    }
}

void Search::startSearchProcess() {
    if (show_list_of_corpora_before_search_) {
        Wt::WDialog *dialog = new Wt::WDialog("Info");
        dialog->contents()->addWidget(new Wt::WText("<p>Your search will be performed on the following corpora:</p>"
                "<p>" +
                boost::algorithm::join(getSelectedLiteratures(), ", ") +
                ".</p><p>To modify the search, click 'Cancel' and select the "
                "corpora from the literature option.</p>"));
        dialog->contents()->addStyleClass("form-group");
        dialog->setWidth(Wt::WLength("70%"));
        Wt::WCheckBox *cb = new Wt::WCheckBox("don't show this message again");
        cb->checked().connect(std::bind([ = ](){
            show_list_of_corpora_before_search_ = false;
            WApplication::instance()->setCookie("show_list_of_corpora_before_search", "false", 60000);
            if (session_->login().state() != 0) {
                Preference *dialogPref = new Preference(PGDIALOGPREFERENCES, PGDIALOGPREFERENCESTABLENAME,
                        session_->login().user().identity("loginname").toUTF8());
                        vector<string> prefVec = dialogPref->GetPreferencesVec();
                        set<string> prefSet;
                        copy(prefVec.begin(), prefVec.end(), inserter(prefSet, prefSet.begin()));
                        prefSet.insert("show_list_of_corpora_before_search");
                        prefVec.clear();
                        copy(prefSet.begin(), prefSet.end(), back_inserter(prefVec));
                        dialogPref->SavePreferencesVector(session_->login().user().identity("loginname").toUTF8(), prefVec);
            }
        }));
        dialog->contents()->addWidget(cb);
        Wt::WPushButton *ok = new Wt::WPushButton("Start Search");
        dialog->footer()->addWidget(ok);
        ok->setDefault(true);
        Wt::WPushButton *cancel = new Wt::WPushButton("Cancel");
        dialog->footer()->addWidget(cancel);
        dialog->rejectWhenEscapePressed();
        ok->clicked().connect(std::bind([ = ](){
            dialog->accept();
            updatetimer_ = new Wt::WTimer();
            updatetimer_->setInterval(5);
            updatetimer_->timeout().connect(this, &Search::DoSearchUpdates);
            searchstatus_ = 0;
            updatetimer_->start();
            bt_ = std::chrono::system_clock::now();
        }));
        cancel->clicked().connect(std::bind([ = ](){
            dialog->reject();
        }));
        dialog->show();
    } else {
        updatetimer_ = new Wt::WTimer();
        updatetimer_->setInterval(5);
        updatetimer_->timeout().connect(this, &Search::DoSearchUpdates);
        searchstatus_ = 0;
        updatetimer_->start();
        bt_ = std::chrono::system_clock::now();
    }
}

void Search::prepareKeywordColorsForSearch() {
    String keyword = StringUtils::toString(keywordtext_->text().value());
    vector<string> keyword_entities;
    keywordColorsMap_ = unordered_map<string, int>();
    string keyword_string(keyword.begin(), keyword.end());
    std::vector<std::string> phrases(RemovePhrases(keyword_string));
    eraseAllOccurrencesOfStr(keyword_string, "*");
    eraseAllOccurrencesOfStr(keyword_string, "+");
    eraseAllOccurrencesOfStr(keyword_string, "?");
    eraseAllOccurrencesOfStr(keyword_string, "AND");
    eraseAllOccurrencesOfStr(keyword_string, "OR");
    boost::split(keyword_entities, keyword_string, boost::is_any_of(" "));
    for (auto x : phrases) keyword_entities.push_back(x);
    keyword_entities.erase(std::remove(keyword_entities.begin(), keyword_entities.end(), ""),
            keyword_entities.end());
    int colorIndex = 0;
    for (auto entity : keyword_entities) {
        keywordColorsMap_.insert({
            {entity, colorIndex++}
        });
    }
}

vector<string> Search::getSelectedLiteratures() {
    set<string> literatures;
    vector<string> lit_vec;
    std::map < std::string, bool>::iterator itpl;
    for (auto& lit_pair : pickedliterature_) {
        if (lit_pair.second) {
            literatures.insert(lit_pair.first);
        }
    }
    copy(literatures.begin(), literatures.end(), back_inserter(lit_vec));
    return lit_vec;
}

tpc::index::Query Search::getSearchQuery() {
    tpc::index::Query query;
    query.case_sensitive = cb_casesens_->isChecked();
    query.sort_by_year = cb_year_->isChecked();
    query.literatures = getSelectedLiteratures();
    query.keyword = "\\(" + StringUtils::toUTF8(keywordtext_->text().value()) + "\\)";
    query.exclude_keyword = StringUtils::toUTF8(keywordnottext_->text().value());
    std::set<std::string>::iterator it;
    query.categories_and_ed = categoriesanded_;
    for (auto& cat : pickedcat_) {
        query.categories.push_back(cat);
    }
    std::wstring type(keywordfield_combo_->currentText().value());
    if (type == L"sentence")
        query.type = QueryType::sentence;
    else {
        type = searchlocation_->currentText().value();
        if (type == L"abstract")
            query.type = QueryType::abstract;
        else if (type == L"acknowledgments")
            query.type = QueryType::acknowledgments;
        else if (type == L"background")
            query.type = QueryType::background;
        else if (type == L"beginning of article")
            query.type = QueryType::begofart;
        else if (type == L"conclusion")
            query.type = QueryType::conclusion;
        else if (type == L"design")
            query.type = QueryType::design;
        else if (type == L"discussion")
            query.type = QueryType::discussion;
        else if (type == L"document")
            query.type = QueryType::document;
        else if (type == L"introduction")
            query.type = QueryType::introduction;
        else if (type == L"materials and methods")
            query.type = QueryType::materials;
        else if (type == L"references")
            query.type = QueryType::references;
        else if (type == L"result")
            query.type = QueryType::result;
        else if (type == L"title")
            query.type = QueryType::title;
    }



    //adding filters:
    map<string, string> filters = getFilters();
    query.author = filters["author"]; //apply filters
    //adding journal filter
    query.journal = filters["journal"]; //apply filters
    //adding year filter
    query.year = filters["year"]; //apply filters
    query.accession = filters["accession"];
    query.paper_type = filters["type"];
    query.exact_match_author = author_exact_match->isChecked();
    query.exact_match_journal = journal_exact_match->isChecked();
    return query;
}

map<string, string> Search::getFilters() {
    map<string, string> filters;
    filters["author"] = WtString2string(author_filter_->text().value());
    filters["journal"] = WtString2string(journal_filter_->text().value());
    filters["year"] = WtString2string(year_filter_->text().value());
    filters["accession"] = WtString2string(accession_filter_->text().value());
    filters["type"] = WtString2string(type_filter_->text().value());
    return filters;
}

void Search::hideTable() {
    size_text_->setText("");
    page_number_text_->setText("");
    updatetext_->setText("");
    previous_button_->hide();
    next_button_->hide();
    first_button_->hide();
    last_button_->hide();
    table_->clear();
    table_->hide();
}

void Search::WriteTsvFile(const std::vector< std::vector < std::wstring> > &contents,
        const std::string tmpfilename) {
    std::cerr << tmpfilename;
    std::ofstream out(tmpfilename);
    if (out.is_open()) {
        out << "Index\t";
        out << "Accession\t";
        out << "Literature\t";
        out << "Type\t";
        out << "Title\t";
        out << "Author\t";
        out << "Journal\t";
        out << "Year\t";
        out << "Abstract\t";
        out << "Score\t";
        out << "File location\t";
        out << std::endl;
        for (unsigned i = 0; i < contents.size(); ++i) {
            for (auto x : contents[i]) out << x << "\t";
            out << std::endl;
        }
        out.close();
    } else {
        std::cerr << "Couldn't open " << tmpfilename << std::endl;
    }
}

void Search::displayTable(int start, int end, int direction) {
    displaySwitches switches;
    // data structure to keep track of open panels
    // create reader to retrieve paper information
    expandedPanelIndexes_.clear();
    tableTextGroupBoxes_.clear();
    vector<string> literatures = getSelectedLiteratures();
    if (totalresults_ == 0) {
        showMessagebox("Your search returned no result.");
        table_->clear();
        table_->refresh();
        return;
    }
    int total_page = totalresults_ / RECORDS_PER_PAGE + 1;
    if (totalresults_ % RECORDS_PER_PAGE == 0) //special case when total number is divisible by recordperpage
        total_page = totalresults_ / RECORDS_PER_PAGE;
    std::vector< std::vector < std::wstring> > contents;
    max_score_ = searchResults_.max_score;
    min_score_ = searchResults_.min_score;
    max_min_ = max_score_ - min_score_;
    int n_row = 0;
    int i = 0;
    if (direction == 1) { //next page
        i = end;
    } else if (direction == -1) { //previous page
        i = start;
    } else if (direction == 0) //first page
    {
        i = 0;
    } else if (direction == 999999) //last page
    {
        i = (total_page - 1)*20 - 1;
    }
    if (i == 0 || i < 0) {
        i = -1; //for first result to display
    }
    vector<String> identifiers; //collect 20 identifiers
    vector<int> indexes; //collect 20 doc ids.
    vector<DocumentSummary> doc_summaries;
    vector<double> scores;
    while (n_row < RECORDS_PER_PAGE && i < totalresults_ - 1) {
        if (direction == 1) //next page 
        {
            i++;
        } else if (direction == -1) //previous page
        {
            i--;
            if (i < 0) i = 0;
        } else if (direction == 0) {
            i++;
        } else if (direction == 999999) {
            i++;
        }
        double doc_score = searchResults_.hit_documents[i].score;
        if (totalresults_ > 1) {
            if (max_min_ > 0) {
                doc_score = (doc_score - min_score_) / max_min_;
            } else {
                doc_score = 0;
            }
        } else {
            doc_score = 1;
        }
        if (direction == -1) {
            scores.insert(scores.begin(), doc_score);
            doc_summaries.insert(doc_summaries.begin(), searchResults_.hit_documents[i]);
        } else {
            scores.push_back(doc_score);
            doc_summaries.push_back(searchResults_.hit_documents[i]);
        }
        n_row++;
    }
    set<string> fields_to_exclude;
    vector<DocumentDetails> docsDetails = indexManager_.get_documents_details(
            doc_summaries, cb_year_->isChecked(), false,
            DOCUMENTS_FIELDS_DETAILED, SENTENCE_FIELDS_DETAILED, {
                "fulltext_compressed", "fulltext_cat_compressed"
            },
    {
    });
    n_row = 0;
    bool use_identifiers = all_of(doc_summaries.begin(), doc_summaries.end(), [](DocumentSummary d) {
        return !d.identifier.empty();
    });
    map<string, DocumentDetails> docDetailsMap;
    for (DocumentDetails docDetails : docsDetails) {
        if (use_identifiers) {
            docDetailsMap[docDetails.identifier] = docDetails;

        } else {
            docDetailsMap[to_string(docDetails.lucene_internal_id)] = docDetails;
        }
    }
    for (int i = 0; i < doc_summaries.size(); ++i) {
        cout << "doc score " << scores[i] << "min score " << min_score_ << endl;
        std::wstring w_score = boost::lexical_cast<std::wstring > (100.0 * scores[i]);
        w_score = w_score.substr(0, 5); //display only 4 digit of score
        identifiers.push_back(StringUtils::toString(doc_summaries[i].identifier.c_str())); //collection 20 identifers
        indexes.push_back(i); //collect 20 doc ids
        int maxHits = 1;
        DocumentDetails docDetails;
        if (use_identifiers) {
            docDetails = docDetailsMap[doc_summaries[i].identifier];
        } else {
            docDetails = docDetailsMap[to_string(doc_summaries[i].lucene_internal_id)];
        }
        n_row++; //one row is qualified
        std::vector<std::wstring> row;
        std::wstring serial = boost::lexical_cast<std::wstring > (i + 1);
        row.push_back(serial);
        row.push_back(LString2WtString(String(docDetails.accession.begin(), docDetails.accession.end())));
        string corpora = boost::join(docDetails.corpora, ", ");
        row.push_back(LString2WtString(String(corpora.begin(), corpora.end())));
        row.push_back(LString2WtString(String(docDetails.type.begin(), docDetails.type.end())));
        if (!docDetails.title.empty()) {
            string cleanTitle = docDetails.title.substr(6, docDetails.title.size() - 10);
            row.push_back(LString2WtString(String(cleanTitle.begin(), cleanTitle.end())));
        } else {
            row.push_back(L"");
        }
        if (!docDetails.author.empty()) {
            string cleanAuthor = docDetails.author.substr(6, docDetails.author.size() - 10);
            row.push_back(LString2WtString(String(cleanAuthor.begin(), cleanAuthor.end())));
        } else {
            row.push_back(L"");
        }
        if (!docDetails.journal.empty()) {
            string cleanJournal = docDetails.journal.substr(6, docDetails.journal.size() - 10);
            row.push_back(LString2WtString(String(cleanJournal.begin(), cleanJournal.end())));
        } else {
            row.push_back(L"");
        }
        row.push_back(LString2WtString(String(docDetails.year.begin(), docDetails.year.end())));
        row.push_back(LString2WtString(String(docDetails.abstract.begin(), docDetails.abstract.end())));
        row.push_back(w_score);
        string filepath = docDetails.filepath;
        boost::replace_regex(filepath, boost::regex("^PMCOA [^\\/]*\\/"), string("PMCOA\\/"));
        row.push_back(LString2WtString(String(filepath.begin(), filepath.end())));
        contents.push_back(row);
    }
    if (direction == 1) {
        if (current_end_ != 0) {
            current_start_ = current_end_ + 1;
        }
        current_end_ = i;
    } else if (direction == -1) {
        current_end_ = current_start_ - 1;
        current_start_ = i;
        //std::reverse(contents.begin(), contents.end());
    } else if (direction == 0) {
        current_start_ = 0;
        current_end_ = RECORDS_PER_PAGE - 1;
    } else if (direction == 999999) {
        current_start_ = (total_page - 1)*20;
        current_end_ = i;
    }
    // table manipulation from here
    table_->show();
    table_->clear();
    table_->refresh();
    std::vector<std::wstring> labels = {L"", L"Accession", L"journal", L"year", L"Type", L"Paper Title", L"Doc Score",
        L"Select"};
    table_->elementAt(0, 0)
            ->addWidget(new Wt::WText(labels[0])); //serial
    table_->elementAt(0, 1)
            ->addWidget(new Wt::WText(labels[1])); //accession
    table_->elementAt(0, 2)
            ->addWidget(new Wt::WText(labels[5])); //title
    table_->elementAt(0, 3)
            ->addWidget(new Wt::WText(labels[2])); //Journal
    table_->elementAt(0, 4)
            ->addWidget(new Wt::WText(labels[3])); //Year
    table_->elementAt(0, 5)
            ->addWidget(new Wt::WText(labels[4])); //Type
    table_->elementAt(0, 6)
            ->addWidget(new Wt::WText(labels[6])); //doc score
    if (switches.isNotSuppressed("curation")) {
        table_->elementAt(0, 7)
                ->addWidget(new Wt::WText(labels[7])); //link
        Wt::WImage * im = new Wt::WImage("resources/icons/qmark15.png");
        im->setVerticalAlignment(Wt::AlignTop);
        im->setInline(true);
        im->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, im));
        im->clicked().connect(this, &Search::HelpCurationCheckBoxDialog);
        table_->elementAt(0, 7)->addWidget(im);
        Wt::WPushButton * vspbutton = new Wt::WPushButton("View selected paper!");
        vspbutton->setLink(Wt::WLink(Wt::WLink::InternalPath, "/curation"));
        vspbutton->setStyleClass("btn-mini");
        table_->elementAt(0, 7)->addWidget(new WBreak());
        table_->elementAt(0, 7)->addWidget(vspbutton);
        all_vp_cbxes_.clear();
    }
    for (unsigned i = 0; i < contents.size(); ++i) {
        vector<wstring> row = contents[i];
        table_->elementAt(i + 1, 0)->setMinimumSize(50, 0);
        table_->elementAt(i + 1, 0)->addWidget(new Wt::WText(row[0])); //serial
        string accession;
        string full_accession(row[1].begin(), row[1].end());
        std::regex rgx_pmid("(PMID[:]{0,1}[\\s+]*[0-9]+)");
        std::regex rgx_doi("(doi:[^ ]+)");
        std::smatch match;
        if (std::regex_search(full_accession, match, rgx_pmid)) {
            accession = match[1];
        } else if (std::regex_search(full_accession, match, rgx_doi)) {
            accession = match[1];
        } else {
            accession = full_accession;
        }
        table_->elementAt(i + 1, 1)->addWidget(new Wt::WText(accession)); //accession
        Wt::WPanel* panel = new Wt::WPanel();
        //add "supplemental material" based on literature name
        wstring p_title(row[4]);
        std::regex lit_sup_re(".*[S|s]upplemental.*");
        string s_title(row[2].begin(), row[2].end());
        if (std::regex_match(s_title, lit_sup_re)) {
            p_title += L" (Supplemental material)";
        }
        panel->setTitle(p_title); //title
        panel->addStyleClass("centered-example");
        panel->setCollapsible(true);
        panel->setCollapsed(true);

        panel->titleBarWidget()->decorationStyle().setBackgroundColor(Wt::WColor(190, 220, 220));
        panel->titleBarWidget()->decorationStyle().setForegroundColor(Wt::WColor(50, 50, 50));
        panel->titleBarWidget()->decorationStyle().font().setSize(Wt::WFont::Large);

        Wt::WAnimation animation(Wt::WAnimation::SlideInFromTop, Wt::WAnimation::EaseOut, 100);
        panel->setAnimation(animation);

        //add bib here 
        WGroupBox* textGroupBox = new Wt::WGroupBox();
        Wt::WText* author_line = new Wt::WText(L"<b>Author</b>: " + row[5]);
        author_line->decorationStyle().font().setSize(Wt::WFont::Large);
        textGroupBox->addWidget(author_line);
        textGroupBox->addWidget(new Wt::WBreak());
        Wt::WText* journal_line = new Wt::WText(L"<b>Journal</b>: " + row[6]);
        journal_line->decorationStyle().font().setSize(Wt::WFont::Large);
        textGroupBox->addWidget(journal_line);
        textGroupBox->addWidget(new Wt::WBreak());
        Wt::WText* year_line = new Wt::WText(L"<b>Year</b>: " + row[7]);
        year_line->decorationStyle().font().setSize(Wt::WFont::Large);
        textGroupBox->addWidget(year_line);
        textGroupBox->addWidget(new Wt::WBreak());
        Wt::WText* lit_line = new Wt::WText(L"<b>Literature</b>: " + row[2]);
        lit_line->decorationStyle().font().setSize(Wt::WFont::Large);
        textGroupBox->addWidget(lit_line);
        Wt::WImage * imLit = new Wt::WImage("resources/icons/qmark15.png");
        imLit->setVerticalAlignment(Wt::AlignTop);
        imLit->setInline(true);
        imLit->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, imLit));
        imLit->clicked().connect(std::bind([ = ] (){
            Wt::WDialog *dialog = new Wt::WDialog("Info");
            dialog->contents()->addWidget(new Wt::WText("<p>This field contains all the corpora into which the document "
            "has been classified, which might not be limited to the "
            "set of corpora selected for the search.</p>"));
            dialog->contents()->addStyleClass("form-group");
            dialog->setWidth(Wt::WLength("70%"));
            Wt::WPushButton *ok = new Wt::WPushButton("Close");
            dialog->footer()->addWidget(ok);
            ok->setDefault(true);
            dialog->rejectWhenEscapePressed();
            ok->clicked().connect(std::bind([ = ]()
            {
                dialog->accept();
            }));
            dialog->show();
        }));
        textGroupBox->addWidget(imLit);
        textGroupBox->addWidget(new Wt::WBreak());
        Wt::WText* accession_line = new Wt::WText(L"<b>Full Accession</b>: " + row[1]);
        accession_line->decorationStyle().font().setSize(Wt::WFont::Large);
        textGroupBox->addWidget(accession_line);
        Wt::WPanel* abs_panel = new Wt::WPanel();
        abs_panel->setTitle("Click for Abstract");
        abs_panel->titleBarWidget()->decorationStyle().setBackgroundColor(Wt::WColor(255, 255, 255));
        abs_panel->titleBarWidget()->decorationStyle().setForegroundColor(Wt::WColor(100, 100, 255));
        abs_panel->titleBarWidget()->decorationStyle().font().setSize(Wt::WFont::Medium);
        abs_panel->addStyleClass("centered-example");
        abs_panel->setCollapsible(true);
        abs_panel->setCollapsed(true);

        Wt::WAnimation animation_abs(Wt::WAnimation::SlideInFromTop, Wt::WAnimation::EaseOut, 100);
        abs_panel->setAnimation(animation_abs);
        abs_panel->setCentralWidget(new Wt::WText(row[8]));
        textGroupBox->addWidget(abs_panel);
        textGroupBox->addWidget(new Wt::WBreak());
        panel->setCentralWidget(textGroupBox);
        panel->expanded().connect(boost::bind(&Search::setHitText, this, indexes[i], textGroupBox));
        tableTextGroupBoxes_.push_back(textGroupBox);
        panel->titleBarWidget()->clicked().connect(boost::bind(&Search::PanelTitleClick, this, panel, indexes[i], textGroupBox));
        table_->elementAt(i + 1, 2)->addWidget(panel); //panel
        table_->elementAt(i + 1, 2)->setMaximumSize(800, 100);
        //table_->elementAt(i + 1, 3)->setMinimumSize(100, 0);
        table_->elementAt(i + 1, 3)->addWidget(new WText(row[6])); //journal
        table_->elementAt(i + 1, 3)->setContentAlignment(Wt::AlignCenter);
        table_->elementAt(i + 1, 4)->addWidget(new WText(row[7])); //year
        table_->elementAt(i + 1, 4)->setMinimumSize(100, 100);
        table_->elementAt(i + 1, 4)->setContentAlignment(Wt::AlignCenter);
        table_->elementAt(i + 1, 5)->addWidget(new Wt::WText(row[3])); //type
        table_->elementAt(i + 1, 5)->setContentAlignment(Wt::AlignCenter);
        table_->elementAt(i + 1, 6)->addWidget(new Wt::WText(row[9])); //score
        table_->elementAt(i + 1, 6)->setContentAlignment(Wt::AlignCenter);
        table_->elementAt(i + 1, 4)->setMinimumSize(100, 100);
        if (switches.isNotSuppressed("curation")) {
            std::string filepath(row[10].begin(), row[10].end());
            Wt::WCheckBox * cb = new Wt::WCheckBox();
            all_vp_cbxes_.push_back(cb);
            cb->clicked().connect(boost::bind(&Search::ViewPaperClicked, this, cb,
                    std::string(row[4].begin(), row[4].end()),
                    std::string(row[5].begin(), row[5].end()),
                    std::string(row[6].begin(), row[6].end()),
                    std::string(row[7].begin(), row[7].end()),
                    filepath, indexes[i], std::string(row[1].begin(), row[1].end())));
            table_->elementAt(i + 1, 7)->addWidget(cb);
            table_->elementAt(i + 1, 7)->setContentAlignment(Wt::AlignCenter);
        }
    }
    std::string tmpfilename = Wt::WApplication::instance()->sessionId() + ".tsv";
    std::string tmpdirname = "/usr/lib/cgi-bin/tc/downloads/";
    WriteTsvFile(contents, tmpdirname + tmpfilename);
    Wt::WAnchor* tsvAnchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::Url, "downloads/" + tmpfilename), "Download this table");
    tsvAnchor->setTarget(Wt::TargetDownload);
    table_->elementAt(contents.size() + 1, 0)
            ->addWidget(tsvAnchor);
    if (direction == 1) {
        currentpage_++;
    } else if (direction == -1) {
        currentpage_--;
    } else if (direction == 0) {
        currentpage_ = 1;
    } else if (direction == 999999) {
        currentpage_ = total_page;
    }
    page_number_text_->setText("Page: " + std::to_string(currentpage_) +
            " of " + std::to_string(total_page));
    if (currentpage_ == 1) {
        previous_button_->hide();
        next_button_->show();
        last_button_->show();
    } else if (currentpage_ == total_page) {
        previous_button_->show();
        next_button_->hide();
        last_button_->hide();
    } else {
        previous_button_->show();
        next_button_->show();
        last_button_->show();
    }
    first_button_->show();
    if (total_page == 1) // if there's only one page, disable all shifting
    {
        first_button_->hide();
        last_button_->hide();
        previous_button_->hide();
        next_button_->hide();
    }
}

void Search::ViewPaperClicked(Wt::WCheckBox * cb,
        const string& title,
        const string& author, const string& journal, const string& year,
        const string& filepath, int index, const string & accession) {
    //
    auto tcnw = dynamic_cast<TCNavWeb*> (parent_);
    if (cb->isChecked()) {
        PaperAddress pa;
        pa.rootdir = CAS_ROOTDIR;
        std::vector<std::string> splits;
        boost::split(splits, filepath, boost::is_any_of("/"));
        pa.paperdir = splits[0];
        for (int i = 1; i < splits.size() - 1; i++) pa.paperdir += "/" + splits[i];
        pa.paperfile = splits[splits.size() - 1] + ".gz";
        pa.title = title;
        pa.author = author;
        pa.journal = journal;
        pa.year = year;
        pa.bestring = RetrieveBEString(index);
        pa.accession = accession;
        pa.case_sensitive = cb_casesens_->isChecked();
        string keywords_no_logic = query_.keyword;
        if (query_.keyword != "") {
            boost::replace_all(keywords_no_logic, " AND ", " ");
            boost::replace_all(keywords_no_logic, " OR ", " ");
            boost::replace_all(keywords_no_logic, "(", "");
            boost::replace_all(keywords_no_logic, ")", "");
            boost::regex_replace(keywords_no_logic, boost::regex("\\s+"), " ");
            vector<string> temp;
            boost::split(temp, keywords_no_logic, boost::is_any_of("\t "));
            for (const auto& t : temp) {
                if (t != "") {
                    pa.keywords.push_back(t);
                }
            }
        }
        vector<string> categories;
        for (const auto& category : query_.categories) {
            string cat = category;
            if (category.substr(0, 5) == "PTCAT") {
                cat = category.substr(5, category.size());
                set<string> children_cat = tcp_->GetAllChildrensName(cat);
                for (const auto& child : children_cat) {
                    categories.push_back(child);
                }
            }
            categories.push_back(cat);
        }
        pa.categories = categories;
        tcnw->SetSinglePaperItem(pa);
        for (std::vector<Wt::WCheckBox*>::const_iterator it = all_vp_cbxes_.begin();
                it != all_vp_cbxes_.end(); it++)
            if (*it != cb) (*it)->disable();

    } else {
        tcnw->DeleteSinglePaperItem();
        for (std::vector<Wt::WCheckBox*>::const_iterator it = all_vp_cbxes_.begin();
                it != all_vp_cbxes_.end(); it++)
            (*it)->enable();
    }
}

std::string Search::RetrieveBEString(int index) {
    // search for the document again but this time get information about matching sentences
    string ret_string;
    DocumentSummary document = DocumentSummary();
    if (!(searchResults_.query.type == QueryType::sentence)) {
        tpc::index::Query q = searchResults_.query;
        boost::replace_all(q.keyword, " AND ", " OR ");
        boost::replace_all(q.keyword, "(", "");
        boost::replace_all(q.keyword, ")", "");
        boost::replace_all(q.keyword, "\\", "");
        boost::replace_all(q.exclude_keyword, " AND ", " OR ");
        boost::replace_all(q.exclude_keyword, "(", "");
        boost::replace_all(q.exclude_keyword, ")", "");
        boost::replace_all(q.exclude_keyword, "\\", "");
        boost::regex_replace(q.keyword, boost::regex("\\s+"), " ");
        boost::regex_replace(q.exclude_keyword, boost::regex("\\s+"), " ");
        q.categories.clear();
        q.categories_and_ed = false;
        q.type = QueryType::sentence;
        string docid = indexManager_.get_document_details(
                searchResults_.hit_documents[index + current_start_], false,{"doc_id", "fulltext_compressed", "fulltext_cat_compressed"},
        {
        },
        {
        },
        {
        }).identifier;
        try {
            auto search_docs = indexManager_.search_documents(q, false,{docid});
            if (search_docs.hit_documents.size() > 0) {
                document = search_docs.hit_documents[0];
            } else {
                document = searchResults_.hit_documents[index + current_start_];
            }
        } catch (tpc_exception& e) {
            document = searchResults_.hit_documents[index + current_start_];
        }
    } else {
        document = searchResults_.hit_documents[index + current_start_];
    }
    String identifier = StringUtils::toString(document.identifier.c_str());
    int beupperlimit = 110;
    vector<SentenceDetails> docSentencesDetails = indexManager_.get_document_details(
            document, true,{"doc_id"},
    {
        "sentence_id", "begin", "end"
    },
    {
    },
    {
    }).sentences_details;
    sort(docSentencesDetails.begin(), docSentencesDetails.end(), sentence_position_lt);
    vector<string> begins_ends;
    for (int i = 0; i < docSentencesDetails.size() && i < beupperlimit; i++) {
        string bg_str = to_string(docSentencesDetails[i].doc_position_begin);
        string ed_str = to_string(docSentencesDetails[i].doc_position_end);
        String bg = String(bg_str.begin(), bg_str.end());
        String ed = String(ed_str.begin(), ed_str.end());
        begins_ends.push_back("(" + LString2string(bg) + "," + LString2string(ed) + ")");
    }
    ret_string = boost::algorithm::join(begins_ends, " ");
    if (ret_string.size() == 0) {
        ret_string = "(0,0)";
    }
    return ret_string;
}

vector<wstring> Search::getCleanKeywords() {
    String l_keyword = StringUtils::toString(stored_keyword_);
    wstring w_keyword = LString2wstring(l_keyword);
    boost::replace_all(w_keyword, "AND", "");
    boost::replace_all(w_keyword, "OR", "");
    boost::replace_all(w_keyword, "(", "");
    boost::replace_all(w_keyword, ")", "");
    boost::wregex re_boostfactor(L"\\^\\d+");
    w_keyword = boost::regex_replace(w_keyword, re_boostfactor, L"");
    w_keyword = boost::regex_replace(w_keyword, boost::wregex(L"\\s+"), L" ");
    vector<wstring> keywords;
    boost::split(keywords, w_keyword, boost::is_any_of(" \"\'~"));
    keywords.erase(std::remove(keywords.begin(), keywords.end(), L""), keywords.end());
    return keywords;
}

void Search::setHitText(int index, Wt::WGroupBox * textGroupBox) {
    expandedPanelIndexes_.insert(index);
    DocumentSummary currDocSummary = searchResults_.hit_documents[index + current_start_];
    vector<WWidget*> children = textGroupBox->children();
    int n_children = children.size();
    for (int i = 12; i <= n_children; i++) { // clean up junky widgets before inserting hit texts
        textGroupBox->removeWidget(children[i - 1]);
    }
    // sentence_length controls the number of words to be displayed before and after a keyword hit. Each range of text
    // around a hit is considered a separate sentence.
    int sentence_length(30);
    vector<wstring> keywords = getCleanKeywords();
    string docIdentifier = currDocSummary.identifier; //getting identifier in reverse order
    double doc_score = currDocSummary.score;
    int sentence_count = 0;
    if (searchResults_.query.type == QueryType::sentence) {
        DocumentDetails currDocDetails = indexManager_.get_document_details(
                currDocSummary, true,{"doc_id"},
        {
            "sentence_id", "sentence_compressed", "sentence_cat_compressed"
        },
        {
        },
        {
        });
        sort(currDocDetails.sentences_details.begin(),
                currDocDetails.sentences_details.end(),
                IndexManager::sentence_greater_than);
        for (const SentenceDetails& sentDetails : currDocDetails.sentences_details) {
            string sentence_text = sentDetails.sentence_text;
            string categories_string = sentDetails.categories_string;
            double score = sentDetails.score;
            String l_text = String(sentence_text.begin(), sentence_text.end());
            String l_cat = String(categories_string.begin(), categories_string.end());
            wstring w_text = LString2wstring(l_text);
            wstring w_cat = LString2wstring(l_cat);
            w_text = RemoveTags(w_text);
            w_cat = RemoveTags(w_cat);
            wstring w_cleantext = L"";
            for (int i = 0; i < w_text.length(); i++) {
                char c = w_text[i];
                if (std::isprint(c)) {
                    w_cleantext += c;
                } else {
                    w_cleantext += L" ";
                }
            }
            w_cleantext = RemoveTags(w_cleantext);
            // subwstrings contains single words (punctuation included in words)
            vector<wstring> subwstrings;
            boost::split(subwstrings, w_cleantext, boost::is_any_of(" "));

            int n_words = subwstrings.size();
            vector<int> keyword_positions = getKeywordPositions(subwstrings, stored_keyword_);
            sentence_count++;
            WContainerWidget* wt_sentence = getSingleSentenceHighlightedWidgetFromText(w_text, w_cat, keywords,
                    sentence_length, textGroupBox);
            double normFactor;
            if (min_score_ != max_score_) {
                normFactor = (doc_score - min_score_) / max_min_;
            } else {
                normFactor = 1;
            }
            double sentence_score_display = (100.0 * score / doc_score) * normFactor;
            std::stringstream ss;
            ss << "Sentence " << sentence_count << " Score: " << sentence_score_display;
            std::string sentence_number_s = ss.str();
            cout << "sentence_number_s " << sentence_number_s << " Score:" << sentence_score_display << endl;
            Wt::WString sentence_number_ws(sentence_number_s);
            Wt::WText* wt_sentence_label = new Wt::WText(sentence_number_ws);
            wt_sentence_label->decorationStyle().font().setWeight(Wt::WFont::Bold);
            wt_sentence_label->decorationStyle().font().setSize(Wt::WFont::Large);
            textGroupBox->addWidget(wt_sentence_label);

            // Truncate long sentences
            int sentenceNumWords = 0;
            int sentenceNumWidgets = wt_sentence->count();
            for (int i = 0; i < sentenceNumWidgets; ++i) {
                wstring test = ((WText*) wt_sentence->widget(i))->text();
                if (test != L"\n" && test != L"" && test != L" ") {
                    ++sentenceNumWords;
                }
            }
            Wt::WContainerWidget* wt_sentence_truncated = new Wt::WContainerWidget();
            if (sentenceNumWords > SENTENCE_SEARCH_MAX_NUM_DISPLAY_WORDS) {
                int j = 0;
                for (int i = 0; i < sentenceNumWidgets && j < sentenceNumWords &&
                        j < SENTENCE_SEARCH_MAX_NUM_DISPLAY_WORDS; ++i) {
                    wstring test = ((WText*) wt_sentence->widget(i))->text();
                    if (test != L"\n" && test != L"" && test != L" ") {
                        wt_sentence_truncated->addWidget(wt_sentence->widget(i));
                        wt_sentence_truncated->addWidget(new Wt::WText(" "));
                        ++j;
                    }
                }
                WText* l = new WText(" [Long Sentence] ");
                l->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
                l->decorationStyle().font().setSize(Wt::WFont::Medium);
                l->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 150));
                textGroupBox->addWidget(l);
                Wt::WImage * imssc = new Wt::WImage("resources/icons/qmark15.png");
                imssc->setVerticalAlignment(Wt::AlignTop);
                imssc->setInline(true);
                imssc->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, imssc));
                imssc->clicked().connect(this, &Search::HelpLongSentenceDialog);
                textGroupBox->addWidget(imssc);
                Wt::WText * t = new Wt::WText("  show more ...");
                t->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
                t->decorationStyle().font().setSize(Wt::WFont::Medium);
                t->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 150));
                t->mouseWentOver().connect(boost::bind(&Search::SetCursorHand, this, t));
                wt_sentence_truncated->addWidget(t);
                textGroupBox->addWidget(wt_sentence_truncated);
                t->clicked().connect(std::bind(&Search::ShowLongSentenceInDialog, this, sentence_number_s, w_text,
                        w_cat));
            } else {
                textGroupBox->addWidget(wt_sentence);
            }
            textGroupBox->addWidget(new Wt::WBreak());
        }
    } else {
        DocumentDetails currDocDetails = indexManager_.get_document_details(
                currDocSummary, false,{"doc_id", "fulltext_compressed", "fulltext_cat_compressed"},
        {
        },
        {
        },
        {
        });
        string fulltext = currDocDetails.fulltext;
        string categories_string = currDocDetails.categories_string;
        String l_text = String(fulltext.begin(), fulltext.end());
        String l_cat = String(categories_string.begin(), categories_string.end());
        wstring w_text = LString2wstring(l_text);
        wstring w_cat = LString2wstring(l_cat);
        w_text = RemoveTags(w_text);
        w_cat = RemoveTags(w_cat);
        wstring w_cleantext = L"";
        for (int i = 0; i < w_text.length(); i++) {
            char c = w_text[i];
            if (std::isprint(c)) {
                w_cleantext += c;
            } else {
                w_cleantext += L" ";
            }
        }
        w_cleantext = RemoveTags(w_cleantext);
        // subwstrings contains single words (punctuation included in words)
        vector<wstring> subwstrings;
        boost::split(subwstrings, w_cleantext, boost::is_any_of(" "));

        int n_words = subwstrings.size();
        vector<int> keyword_positions = getKeywordPositions(subwstrings, stored_keyword_);
        for (int i = 0; i < keyword_positions.size(); i++) {
            vector<int> hit_positions; //to collect hit positions for one sentence
            int current_position = keyword_positions[i];
            hit_positions.push_back(current_position);
            int k = 0;
            for (k = 0; k < keyword_positions.size(); k++) {
                // if the kth keyword is before 30 words before the current keyword -> continue without adding it to
                // the sentence
                if (current_position - keyword_positions[k] > sentence_length)
                    continue;
                // if the kth keyword is after 30 words after the current keyword -> stop, as the following keywords
                // will be surely out the current range
                if (keyword_positions[k] - current_position > sentence_length)
                    break;
                // add the keyword position to hit positions for the current range
                hit_positions.push_back(keyword_positions[k]);
            }
            // jump to kth keyword for next range - all keywords up to the kth have been already processed
            i = k;
            Wt::WContainerWidget* wt_sentence = new Wt::WContainerWidget();
            //wstring hit_sentence = L"";
            sentence_count++;
            int start = std::max(0, current_position - sentence_length);
            int end = std::min(n_words, current_position + sentence_length);
            for (int j = start; j < end; j++) {
                wstring w_word = subwstrings[j];
                //hit_sentence += w_word;
                Wt::WText* wt_word = new Wt::WText(w_word);
                for (int l = 0; l < hit_positions.size(); l++) //loop over all hits within sentence_length
                {
                    if (j == hit_positions[l]) {
                        wt_word->decorationStyle().font().setSize(Wt::WFont::Medium);
                        wstring word = w_word;
                        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                        if (keywordColorsMap_.find(string(word.begin(), word.end())) != keywordColorsMap_.end()) {
                            wt_word->decorationStyle().setForegroundColor(colorSet_.getKeywordColor(
                                    keywordColorsMap_.find(string(word.begin(), word.end()))->second));
                        } else {
                            wt_word->decorationStyle().setForegroundColor(colorSet_.getKeywordColor(0));
                        }
                    }
                }
                wt_sentence->addWidget(wt_word);
                wt_sentence->addWidget(new Wt::WText(" "));
            }
            std::stringstream ss;
            ss << "Sentence " << sentence_count << endl; //" Score:" << score;
            std::string sentence_number_s = ss.str();
            cout << "sentence_number_s " << sentence_number_s << endl; // << " Score:" << score << endl;
            Wt::WString sentence_number_ws(sentence_number_s);
            Wt::WText* wt_sentence_label = new Wt::WText(sentence_number_ws);
            wt_sentence_label->decorationStyle().font().setWeight(Wt::WFont::Bold);
            wt_sentence_label->decorationStyle().font().setSize(Wt::WFont::Large);
            //textGroupBox->addWidget(new Wt::WText(sentence_number));
            textGroupBox->addWidget(wt_sentence_label);
            textGroupBox->addWidget(wt_sentence);
            textGroupBox->addWidget(new Wt::WBreak());
        }
        vector<wstring> subwstrings4cat;
        boost::split(subwstrings4cat, w_text, boost::is_any_of(" \n\t'\\/()[]{}:.;,!?"));
        setHitCatSentence(w_cat, subwstrings4cat, sentence_length, textGroupBox);
    }
}

wstring Search::RemoveTags(wstring w_cleantext) {
    boost::wregex tagregex(L"\<.+?\>");
    w_cleantext = boost::regex_replace(w_cleantext, tagregex, "");
    boost::wregex tagregex2(L"\<\/.+?\>");
    w_cleantext = boost::regex_replace(w_cleantext, tagregex2, "");

    return w_cleantext;
}

void Search::showMessagebox(WString message) {
    Wt::WMessageBox *messageBox = new Wt::WMessageBox("Message", message, Wt::Information, Wt::Ok);
    messageBox->show();
    messageBox->buttonClicked().connect(boost::bind(&Search::deleteMessagebox, this, messageBox));
}

void Search::deleteMessagebox(Wt::WMessageBox * messageBox) {
    if (messageBox != NULL) {
        if (messageBox->buttonResult() == Wt::Ok) {

            delete messageBox;
        }
    }
}

void Search::changePage(int shift) {
    displayTable(current_start_, current_end_, shift);
}

void Search::PanelTitleClick(WPanel* panel, int index, Wt::WGroupBox * textGroupBox) {
    if (panel->isCollapsed()) {
        setHitText(index, textGroupBox);
    } else {
        expandedPanelIndexes_.erase(index);
    }
}

Wt::WContainerWidget * Search::setHitCatSentence(wstring w_cat, vector<wstring > subwstrings, int sentence_length,
        Wt::WGroupBox * textGroupBox) {
    vector<wstring> cats;
    std::set<std::string>::iterator it;
    for (it = pickedcat_.begin(); it != pickedcat_.end(); it++) {
        wcout << "picked cat: " << *it << endl;
        wstring w_cat_highlight((*it).begin(), (*it).end());
        cats.push_back(w_cat_highlight);
    }
    vector<wstring> category_words;
    boost::split(category_words, w_cat, boost::is_any_of("\t"));
    int n_catwords = category_words.size();
    int n_words = subwstrings.size();

    Wt::WContainerWidget* wt_sentence = new Wt::WContainerWidget();
    vector<Wt::WText*> wtexts;
    int sentence_size = subwstrings.size();
    for (int j = 0; j < sentence_size; j++) {
        wstring w_word = subwstrings[j];
        Wt::WText* wt_word = new Wt::WText(w_word);
        wtexts.push_back(wt_word);
    }
    for (int n = 0; n < cats.size(); n++) {
        wstring w_cat1 = cats[n];
        vector<int> catmatch_positions;
        for (int i = 0; i < n_catwords && w_cat1 != L""; i++) {
            wstring w_word = category_words[i];
            vector<wstring> multi_cats;
            boost::split(multi_cats, w_word, boost::is_any_of("|"));
            for (int k = 0; k < multi_cats.size(); k++) {
                if (boost::iequals(multi_cats[k], w_cat1)) {
                    catmatch_positions.push_back(i);
                }
            }
        }
        if (keywordfield_combo_->currentText().value() == "document") {
            for (int i = 0; i < catmatch_positions.size(); i++) {
                int position = catmatch_positions[i];
                vector<int> hit_positions;
                hit_positions.push_back(position);
                int k = i + 1;
                while (k < catmatch_positions.size() && (catmatch_positions[k] - position < sentence_length)) {
                    hit_positions.push_back(catmatch_positions[k]);
                    k++;
                }
                i = k - 1;
                if (position > sentence_length && position < n_words - sentence_length) {
                    for (int j = position - sentence_length; j < position + sentence_length; j++) {
                        wstring w_word = subwstrings[j];
                        Wt::WText* wt_word = new Wt::WText(w_word);
                        for (int l = 0; l < hit_positions.size(); l++) {
                            if (j == hit_positions[l]) {
                                wt_word->decorationStyle().font().setSize(Wt::WFont::Medium);
                                wt_word->decorationStyle().setForegroundColor(colorSet_.getCategoryColor(n));
                            }
                        }
                        wt_sentence->addWidget(wt_word);
                        wt_sentence->addWidget(new Wt::WText(" "));
                    }
                    textGroupBox->addWidget(wt_sentence);
                }
            }
        } else if (keywordfield_combo_->currentText().value() == "sentence" && catmatch_positions.size() > 0) {
            int sentence_size = subwstrings.size();
            for (int j = 0; j < sentence_size; j++) {
                wstring w_word = subwstrings[j];
                Wt::WText* wt_word = new Wt::WText(w_word);
                for (int l = 0; l < catmatch_positions.size(); l++) {
                    if (j == catmatch_positions[l]) {
                        wtexts[j]->decorationStyle().font().setSize(Wt::WFont::Medium);
                        wtexts[j]->decorationStyle().setForegroundColor(colorSet_.getCategoryColor(n));
                    }
                }
            }
            for (int i = 0; i < wtexts.size(); i++) {
                wt_sentence->addWidget(wtexts[i]);
                wt_sentence->addWidget(new Wt::WText(" "));
            }
        }
    }
    return wt_sentence;
}

vector<int> Search::getKeywordPositions(vector<wstring> subwstrings, wstring keyword) {
    vector<int> ret;
    ret.clear();
    boost::replace_all(keyword, "AND", "");
    boost::replace_all(keyword, "OR", "");
    boost::replace_all(keyword, "(", "\\(");
    boost::replace_all(keyword, ")", "\\)");
    boost::replace_all(keyword, "[", "\\[");
    boost::replace_all(keyword, "]", "\\]");
    boost::wregex re_boostfactor(L"\\^\\d+");
    keyword = boost::regex_replace(keyword, re_boostfactor, L"");
    keyword = boost::regex_replace(keyword, boost::wregex(L"\\s+"), L" ");
    std::vector<std::wstring> phrases = RemovePhrases(keyword);
    vector<std::wstring> keywords;
    boost::split(keywords, keyword, boost::is_any_of(" \"\'~"));
    keywords.erase(std::remove(keywords.begin(), keywords.end(), L""), keywords.end());
    // do keywords first
    for (int i = 0; i < subwstrings.size(); i++) {
        wstring w_word = subwstrings[i];
        for (int j = 0; j < keywords.size(); j++)
            if (keyword_is_match(w_word, keywords[j]))
                ret.push_back(i);
    }
    // now process phrases
    for (int i = 0; i < subwstrings.size(); i++) {
        for (int j = 0; j < phrases.size(); j++) {
            vector<std::wstring> phrasewords;
            boost::split(phrasewords, phrases[j], boost::is_any_of(" \"\'~"));
            phrasewords.erase(std::remove(phrasewords.begin(), phrasewords.end(), L""), phrasewords.end());
            if (phrase_is_match(subwstrings, phrasewords, i))
                for (int k = i; k < i + phrasewords.size(); k++) ret.push_back(k);
        }
    }
    std::sort(ret.begin(), ret.end());
    return ret;
}

void Search::ReadPreloadedCategories() {
    std::string username("default");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname").toUTF8();
    tcp_ = new TpCategoryBrowser();
    std::set<std::string> plist = tcp_->GetAllDirectChildrensName("root");
    Preference * pref = new Preference(PGPRELOADEDCATEGORIES, PGPRELOADEDCATTABLENAME, username);
    preloadedcategories_.clear();
    std::set<std::string>::iterator it;
    for (it = plist.begin(); it != plist.end(); it++) {
        if (pref->HasPreferences())

            if (pref->IsPreference(*it)) preloadedcategories_.insert(*it);
    }
    delete pref;
}

void Search::readPreloadedColors() {
    std::string username("default");
    if (session_->login().state() != 0)
        username = session_->login().user().identity("loginname").toUTF8();
    colorSet_.loadColorsFromDB(username);
}

void Search::UpdateLiteraturePreferences(bool checkpermissions) {
    std::string username("default");
    if (session_->login().state() != 0) {
        username = session_->login().user().identity("loginname").toUTF8();
    }

    Preference * pref = new Preference(PGLITERATURE, PGLITPREFTABLENAME, username);
    Preference * permissions;
    if (checkpermissions)
        permissions = new Preference(PGLITERATUREPERMISSION,
            PGLITERATUREPERMISSIONTABLENAME, username);
    Preference * dfpermissions;
    if (checkpermissions)
        dfpermissions = new Preference(PGLITERATUREPERMISSION,
            PGLITERATUREPERMISSIONTABLENAME, "default");
    for (const string& corpus : IndexManager::get_available_corpora(CAS_ROOT_LOCATION.c_str())) {
        // grant search rights based on individual and default permissions.
        if (checkpermissions) {
            if (permissions->IsPreference(corpus) || dfpermissions->IsPreference(corpus))
                if (pref->HasPreferences())
                    pickedliterature_[corpus] = (pref->IsPreference(corpus)) ? true : false;
                else
                    pickedliterature_[corpus] = true;
        } else {
            if (pref->HasPreferences())
                pickedliterature_[corpus] = (pref->IsPreference(corpus)) ? true : false;
            else
                pickedliterature_[corpus] = true;
        }
    }
    if (session_->login().state() != 0 && indexManager_.has_external_index()) {
        for (const string& external_corpus : indexManager_.get_external_corpora()) {
            pickedliterature_[external_corpus] = true;
        }
    }
    delete pref;
    if (checkpermissions) {
        delete permissions;
        delete dfpermissions;
    }
}

void Search::ReadIndexPrefix() {
    if (session_->login().state() != 0 && boost::filesystem::exists(
            USERUPLOADROOTDIR + string("/") + session_->login().user().identity("loginname").toUTF8() + "/luceneindex")) {
        indexManager_.set_external_index(USERUPLOADROOTDIR + string("/") +
                session_->login().user().identity("loginname").toUTF8()
                + "/luceneindex");
    } else {
        if (indexManager_.has_external_index()) {
            indexManager_.remove_external_index();
        }
    }
    pickedliterature_.clear();
    UpdateLiteraturePreferences(false);
    UpdateLiteratureStatus();
}

void Search::UpdateLiteratureStatus() {
    bool trigger(false);
    std::map < std::string, bool>::iterator it;
    int count(0);
    int countmax(3);
    std::set<std::string> litselect;
    for (const auto& lit : pickedliterature_) {
        if (lit.second) {
            trigger = true;
            count++;
            if (litselect.size() < countmax) {
                litselect.insert(lit.first);
            }
        }
    }
    std::string ll("");
    for (std::set<std::string>::iterator it = litselect.begin(); it != litselect.end(); it++) {
        if (!ll.empty()) ll += ", ";
        ll += *it;
    }
    if (trigger) {
        if (count > countmax) ll += " ... and more. Hit the button to see all.";
        literaturestatus_->decorationStyle().setForegroundColor(Wt::darkBlue);
        literaturestatus_->setText("Current selection: " + ll);

    } else {
        literaturestatus_->decorationStyle().setForegroundColor(Wt::darkRed);
        literaturestatus_->setText("No literature selected.");
    }
}

void Search::CheckAgainstPickedLiteratureMap(Wt::Http::ParameterValues literatures) {
    if (!literatures.empty()) {
        std::map < std::string, bool>::iterator plit;
        for (plit = pickedliterature_.begin(); plit != pickedliterature_.end(); plit++)
            if (std::find(literatures.begin(), literatures.end(), (*plit).first) != literatures.end())
                (*plit).second = true;

            else
                (*plit).second = false;
    }
    UpdateLiteratureStatus();
}

Search::~Search() {
}

/*!
 * generate a container with widgets representing a sentence, where categories and keywords matches are highlighted
 *
 * @param w_text the complete sentence text
 * @param w_cat a string of tab separated categories
 * @param keywords the keywords to highlight
 * @param sentence_length the length of the sentence
 * @param textGroupBox the parent group box, used only for document searches
 * @return the container representing the sentence
 */
WContainerWidget * Search::getSingleSentenceHighlightedWidgetFromText(std::wstring w_text, std::wstring w_cat,
        vector<std::wstring> keywords,
        int sentence_length, Wt::WGroupBox * textGroupBox) {
    vector<wstring> subwstrings4cat;
    boost::split(subwstrings4cat, w_text, boost::is_any_of(" \n\t'\\/()[]{}:.;,!?"));
    Wt::WContainerWidget* wt_sentence = setHitCatSentence(w_cat, subwstrings4cat, sentence_length, textGroupBox);
    int count = wt_sentence->count();
    vector<wstring> stringwords;
    if (count > 0) {
        for (int i = 0; i < count; i++) {

            Wt::WText* wt_word = (Wt::WText*)wt_sentence->widget(i);
            stringwords.push_back(wt_word->text());
        }
        vector<int> keyword_positions = getKeywordPositions(stringwords, stored_keyword_);
        for (int j = 0; j < count; j++) {
            for (int l = 0; l < keyword_positions.size(); l++) //loop over all hits within sentence_length
            {
                if (j == keyword_positions[l]) {
                    wt_sentence->widget(j)->decorationStyle().font().setSize(Wt::WFont::Medium);
                    wstring word = ((Wt::WText*)wt_sentence->widget(j))->text().value();
                    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                    if (keywordColorsMap_.find(string(word.begin(), word.end())) != keywordColorsMap_.end()) {
                        wt_sentence->widget(j)->decorationStyle().setForegroundColor(colorSet_.getKeywordColor(
                                keywordColorsMap_.find(string(word.begin(), word.end()))->second));
                    } else {
                        wt_sentence->widget(j)->decorationStyle().setForegroundColor(colorSet_.getKeywordColor(0));
                    }
                }
            }

        }
    } else {
        boost::split(stringwords, w_text, boost::is_any_of(" "));
        vector<int> keyword_positions = getKeywordPositions(stringwords, stored_keyword_);
        int words_count = stringwords.size();
        for (int j = 0; j < words_count; j++) {
            wstring w_word = stringwords[j];
            Wt::WText* wt_word = new Wt::WText(w_word);
            for (int l = 0; l < keyword_positions.size(); l++) //loop over all hits within sentence_length
            {
                if (j == keyword_positions[l]) {
                    wt_word->decorationStyle().font().setSize(Wt::WFont::Medium);
                    wstring word = w_word;
                    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
                    if (keywordColorsMap_.find(string(word.begin(), word.end())) != keywordColorsMap_.end()) {
                        wt_word->decorationStyle().setForegroundColor(colorSet_.getKeywordColor(
                                keywordColorsMap_.find(string(word.begin(), word.end()))->second));
                    } else {
                        wt_word->decorationStyle().setForegroundColor(colorSet_.getKeywordColor(0));
                    }

                }
            }
            wt_sentence->addWidget(wt_word);
            wt_sentence->addWidget(new Wt::WText(" "));
        }
    }
    return wt_sentence;
}

void Search::eraseAllOccurrencesOfStr(string &str, const string & pattern) {
    std::string::size_type i = str.find(pattern);
    while (i != std::string::npos) {
        str.erase(i, pattern.length());
        i = str.find(pattern, i);
    }
}

void Search::SessionLoginChanged() {
    ResetSearch();
    ReadIndexPrefix();
    readDialogPreferences();
    if (session_->login().state() != 0) {
        setDefTypeBtn_->show();
    } else {
        setDefTypeBtn_->hide();
    }
}

void Search::readDialogPreferences() {
    show_list_of_corpora_before_search_ = true;
    try {
        if (WApplication::instance()->environment().getCookie("show_list_of_corpora_before_search") == "false") {
            show_list_of_corpora_before_search_ = false;
        }
    } catch (const exception& e) {

    }
    if (session_->login().state() != 0) {
        Preference *dialogPref = new Preference(PGDIALOGPREFERENCES, PGDIALOGPREFERENCESTABLENAME,
                session_->login().user().identity("loginname").toUTF8());
        if (dialogPref->HasPreferences())
            if (dialogPref->IsPreference("show_list_of_corpora_before_search"))
                show_list_of_corpora_before_search_ = false;
    }
}

void Search::updateSearchColors() {
    readPreloadedColors();
    for (auto index : expandedPanelIndexes_) {
        setHitText(index, tableTextGroupBoxes_[index % 20]);
    }
}
