/* 
 * File:   Search.h
 * Author: mueller
 *
 * Created on April 4, 2013, 10:20 AM
 */

#ifndef SEARCH_H
#define SEARCH_H

#include "Session.h"

#include <Wt/WContainerWidget>
#include <Wt/WEnvironment>
#include <Wt/WApplication>
#include <Wt/WBreak>
#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>
#include <Wt/WPushButton>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WComboBox>
#include <Wt/WAnimation>
#include <Wt/WPanel>
#include <Wt/WScrollArea>
#include <Wt/WTable>
#include <Wt/WTableCell>
#include <Wt/WStandardItemModel>
#include <Wt/WTableView>
#include <Wt/WSuggestionPopup>
#include <Wt/WTimer>

#include "math.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <ctime>
#include <boost/algorithm/string.hpp>
#include <boost/bind.hpp>
#include <boost/regex.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/thread.hpp>
#include <Wt/WBorderLayout>
#include <Wt/WBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WVBoxLayout>
#include <Wt/WGroupBox>
#include <Wt/WMessageBox>
#include <boost/regex/v4/match_results.hpp>
#include "lucene++/LuceneHeaders.h"
#include "lucene++/FilterIndexReader.h"
#include "lucene++/MiscUtils.h"
#include "lucene++/Highlighter.h"
#include "lucene++/LoadFirstFieldSelector.h"
#include "lucene++/FieldCache.h"
#include <postgresql/libpq-fe.h>
#include <chrono>
#include <unordered_map>
#include "IndexManager.h"
#include "PickCategory.h"
#include "PickLiterature.h"
#include "UrlParameters.h"
#include "ColorSet.h"
#include "lucene/CaseSensitiveAnalyzer.h"


using namespace Wt;
using namespace Lucene;
using namespace std;


#define TEXTPRESSOSEARCHDB "textpressocentralsearch"
#define CATEGORYTABLE "category"

const String LUCENE_INDEX_LOCATION = L"/usr/local/textpresso/luceneindex/";
const int RECORDS_PER_PAGE = 20;
const int NUM_SENTENCES_HITS_WARN_THRESHOLD = 10000;
const string CAS_ROOTDIR = "/usr/local/textpresso/tpcas/";

static const set<string> urlparamset = {"accession", "author", "category", "exclude", "journal", "keyword",
    "literature", "scope", "tpcrootpasswd", "type", "year"};

class Search : public Wt::WContainerWidget {
public:
    Search(UrlParameters *urlparams, Session & session, Wt::WContainerWidget *parent = NULL);
    virtual ~Search();
    void SimpleKeywordSearchApi(Wt::WString text);
    void SetLiteratureDescription(Wt::WContainerWidget *w);
private:
    UrlParameters *urlparameters_;
    Session *session_;
    Wt::WContainerWidget *parent_;
    Wt::WBorderLayout *mainlayout_;
    Wt::WLineEdit *keywordtext_;
    Wt::WLineEdit *keywordnottext_;
    Wt::WComboBox *keywordfield_combo_;
    Wt::WComboBox *searchlocation_;
    Wt::WText *size_text_;
    Wt::WText *page_number_text_;

    WPushButton *addcat_button_;
    WPushButton *button_search_;
    WPushButton *button_reset_;
    WPushButton *next_button_;
    WPushButton *previous_button_;
    WPushButton *first_button_;
    WPushButton *last_button_;
    WLineEdit *author_filter_;
    WLineEdit *journal_filter_;
    WLineEdit *year_filter_;
    WLineEdit *accession_filter_;
    WLineEdit *type_filter_;
    WPushButton *corpus_button_;
    Wt::WCheckBox *author_exact_match;
    Wt::WCheckBox *journal_exact_match;
    Wt::WCheckBox *cb_year_;
    Wt::WCheckBox *cb_casesens_;
    Wt::WText  *literaturestatus_;

    Wt::WContainerWidget  *container_west_;
    Wt::WTable *table_;
    WLineEdit *username_;
    WLineEdit *password_;

    int currentpage_;
    int totalresults_;
    int current_start_;
    int current_end_;

    std::string allResultsAsTSVfilename_;

    std::vector<Wt::WGroupBox*> tableTextGroupBoxes_;
    std::set<int> expandedPanelIndexes_;
    colors::ColorSet colorSet_;
    map<string, string> filters_;
    std::map<std::string, bool> pickedliterature_;
    std::set<std::string> preloadedcategories_;

    tpc::index::SearchResults searchResults_;
    double min_score_;
    double max_score_;
    double max_min_;

    std::unordered_map<std::string, int> keywordColorsMap_;

    PickCategory *pc_;
    PickLiterature *pl_;
    bool categoriesanded_;
    Wt::WContainerWidget *pickedcatcont_;
    std::set<std::string> pickedcat_;
    Wt::WDialog *listallcatsdialog_;
    Wt::WDialog *helplucenedialog_;
    Wt::WDialog *helpscopedialog_;
    Wt::WDialog *helpcurationcheckboxdialog_;
    Wt::WContainerWidget *containerforupdates_;
    Wt::WText *updatetext_;
    Wt::WTimer *updatetimer_;
    int searchstatus_;
    std::chrono::time_point<std::chrono::system_clock> bt_, et_;
    std::vector<Wt::WCheckBox*> all_vp_cbxes_;

    Wt::WContainerWidget *north_;
    WContainerWidget *northInner_;
    WContainerWidget *northInnerButtons_;
    WContainerWidget *northInnerAdditional;
    WContainerWidget *northInnerLiterature;
    Wt::WScrollArea *cs_;
    Wt::WContainerWidget *south_;
    Wt::WText *statusline_;
    WPushButton *setDefTypeBtn_;

    tpc::index::IndexManager indexManager_;
    tpc::index::Query query_;

    double partial_search_time_;
    bool show_list_of_corpora_before_search_;
    bool return_to_sentence_search_;
    std::wstring stored_keyword_;
    TpCategoryBrowser *tcp_;

    void SearchViaUrlParameters();
    void DoSearchUpdates();
    void UpdatePickedCatCont();
    void ListAllCats();
    void ListAllCatsDone(Wt::WDialog::DialogCode code);
    void HelpCurationCheckBoxDialog();
    void HelpCurationCheckBoxDialogDone(Wt::WDialog::DialogCode code);
    void HelpLuceneDialog();
    void HelpLuceneDialogDone(Wt::WDialog::DialogCode code);
    void HelpScopeDialog();
    void SetCursorHand(Wt::WWidget *w);
    void AddCatButtonPressed(Session *session);
    void SelectLiteraturePressed();
    void PickCategoryDialogDone(Wt::WDialog::DialogCode code);
    void PickLiteratureDialogDone(Wt::WDialog::DialogCode code);
    void ReadPreloadedCategories();
    void readPreloadedColors();
    void CreateSearchInterface(Wt::WHBoxLayout *hbox);
    void createSearchButtonsRow(Wt::WHBoxLayout *hbox);
    void doSearch();
    tpc::index::Query getSearchQuery();
    void ResetSearch();
    void WriteTsvFile(const std::vector< std::vector < std::wstring> > &contents,
            const std::string tmpfilename);
    void getAllResults(std::vector<std::vector<std::wstring>>& contents);
    void displayTable(int start, int end, int shift);
    void ViewPaperClicked(Wt::WCheckBox *cb, const std::string& papertitle,
            const std::string& paperauthor, const std::string& paperjournal, const std::string& paperyear,
            const std::string& filepath, int index, const std::string& accession);
    std::string RetrieveBEString(int index);
    void changePage(int shift);
    void deleteMessagebox(Wt::WMessageBox *messageBox);
    map<string, string> getFilters();
    void PanelTitleClick(WPanel *panel, int index, Wt::WGroupBox *textGroupBox);
    void setHitText(int index, Wt::WGroupBox *textGroupBox);
    Wt::WContainerWidget *setHitCatSentence(wstring w_cat, vector<wstring > subwstrings, int sentence_length, Wt::WGroupBox *textGroupBox);
    void showMessagebox(WString message);
    void UpdateLiteraturePreferences(bool checkpermissions);
    void ReadIndexPrefix();
    void UpdateLiteratureStatus();
    void CheckAgainstPickedLiteratureMap(Wt::Http::ParameterValues literatures);
    vector<int> getKeywordPositions(vector<wstring> subwstrings, wstring keyword);
    wstring RemoveTags(wstring w_cleantext);
    void SetLiteratureContainer(Wt::WContainerWidget *literaturecontainer);

    // added by valerio
    Wt::WContainerWidget *getSingleSentenceHighlightedWidgetFromText(std::wstring, std::wstring,
            std::vector<std::wstring>, int, Wt::WGroupBox*);
    std::vector<std::wstring> getCleanKeywords();
    void ShowLongSentenceInDialog(std::string, std::wstring, std::wstring);
    void eraseAllOccurrencesOfStr(string &str, const string &pattern);
    void HelpLongSentenceDialog();
    bool isSearchFormValid();

    static bool pairCompare(const pair<String, int32_t>& firstElem, const pair<String, int32_t>& secondElem) {
        return firstElem.second < secondElem.second;
    }

    void SessionLoginChanged();
    void updateSearchColors();
    void prepareKeywordColorsForSearch();
    vector<string> getSelectedLiteratures();
    void startSearchProcess();
    void readDialogPreferences();

    static bool sentence_position_lt(const tpc::index::SentenceDetails &a, const tpc::index::SentenceDetails &b) {
        return a.doc_position_begin < b.doc_position_begin;
    }
    void createAdditionalOptionsRow(Wt::WHBoxLayout *hbox);
    void createLiteratureRow(WHBoxLayout *hbox);
    void hideTable();
};

#endif /* SEARCH_H */
