/* 
 * File:   SuggestionBoxFromPgOrWeb.cpp
 * Author: mueller
 * 
 * Created on March 23, 2016, 1:45 PM
 */

#include <curl/curl.h>
#include "SuggestionBoxFromPgOrWeb.h"
#include <pqxx/pqxx>
#include <boost/algorithm/string.hpp>
#include <Wt/WApplication>

namespace {
    // callback function writes data to a std::ostream

    static size_t data_write(void* buf, size_t size, size_t nmemb, void* userp) {
        if (userp) {
            std::ostream& os = *static_cast<std::ostream*> (userp);
            std::streamsize len = size * nmemb;
            if (os.write(static_cast<char*> (buf), len))
                return len;
        }
        return 0;
    }

    /**
     * timeout is in seconds
     **/
    CURLcode curl_read(const std::string& url, std::ostream& os, long timeout = 30) {
        CURLcode code(CURLE_FAILED_INIT);
        CURL* curl = curl_easy_init();
        if (curl) {
            if (CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, &data_write))
                    && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 1L))
                    && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L))
                    && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_FILE, &os))
                    && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout))
                    && CURLE_OK == (code = curl_easy_setopt(curl, CURLOPT_URL, url.c_str()))) {
                code = curl_easy_perform(curl);
            }
            curl_easy_cleanup(curl);
        }
        return code;
    }

    std::string getwebpage(std::string url) {
        curl_global_init(CURL_GLOBAL_ALL);
        std::ostringstream oss;
        if (CURLE_OK == curl_read(url, oss)) {
            std::cerr << "Web page successfully downloaded to string." << std::endl;
        } else {
            std::cerr << "Downloading of web page failed." << std::endl;
        }
        curl_global_cleanup();
        return oss.str();
    }
}

SuggestionBoxFromPgOrWeb::SuggestionBoxFromPgOrWeb(std::string col, std::string where,
        std::string tablename, std::string databasename, Wt::WObject * parent)
: Wt::WSuggestionPopup(Wt::WSuggestionPopup::generateMatcherJS(contactOptions1),
Wt::WSuggestionPopup::generateReplacerJS(contactOptions1), parent) {
    col_ = col;
    where_ = where;
    tablename_ = tablename;
    databasename_ = databasename;
    if ((tablename_.empty()) || (col_.empty())) {
        ispg_ = false;
        url_ = databasename;
        boost::replace_first(url_, "host=", "");
        boost::trim(url_);
    } else
        ispg_ = true;
    BasicInit(parent);
}

void SuggestionBoxFromPgOrWeb::BasicInit(Wt::WObject * parent) {
    slm_ = new Wt::WStringListModel();
    setFilterLength(3);
    setModel(slm_);
    setMaximumSize(Wt::WLength(60, Wt::WLength::FontEx), Wt::WLength(15, Wt::WLength::FontEx));
    filterModel().connect(this, &SuggestionBoxFromPgOrWeb::PopulateModel);
}

SuggestionBoxFromPgOrWeb::~SuggestionBoxFromPgOrWeb() {
    delete slm_;
}

void SuggestionBoxFromPgOrWeb::PopulateModel(const Wt::WString & s) {
    clearSuggestions();
    if (ispg_) {
        pqxx::connection cn(databasename_);
        std::string stoutf8 = s.toUTF8();
        boost::replace_all(stoutf8, "(", "\\(");
        boost::replace_all(stoutf8, ")", "\\)");
        boost::replace_all(stoutf8, "[", "\\[");
        boost::replace_all(stoutf8, "]", "\\]");
        boost::replace_all(stoutf8, "{", "\\{");
        boost::replace_all(stoutf8, "}", "\\}");
        std::string search = "(^| )" + stoutf8;
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);
        pqxx::work w(cn);
        pqxx::result r;
        std::stringstream pc;
        pc << "select " << col_ << " from ";
        pc << tablename_ << " where ";
        if (!where_.empty()) pc << "(";
        pc << "lower(" << col_ << ") ~ '" << search << "'";
        if (!where_.empty()) pc << ") AND (" << where_ << ")";
        std::cout << pc.str() << std::endl;
        r = w.exec(pc.str());
        if (r.size() != 0) {
            std::cerr << r.size() << " results returned from postgres." << std::endl;
            std::vector<Wt::WString> v;
            for (pqxx::result::size_type i = 0; i != r.size(); i++) {
                std::string suggested;
                r[i][col_].to(suggested);
                if (i < LISTLIMIT)
                    if (!suggested.empty())
                        v.push_back(Wt::WString(suggested));
            }
            slm_->setStringList(v);
            w.commit();
        } else {
            std::cerr << "No matches in " << tablename_ << std::endl;
        }
        cn.disconnect();
    } else {
        std::string aux(getwebpage(url_ + s.toUTF8()));
        std::vector<std::string> splits;
        boost::split(splits, aux, boost::is_any_of("\n"));
        lengthcompare c;
        std::sort(splits.begin(), splits.end(), c);
        std::vector<Wt::WString> v;
        std::vector<std::string>::iterator it;
        for (it = splits.begin(); it != splits.end(); it++)
            if (std::distance(splits.begin(), it) < LISTLIMIT)
                if (!(*it).empty())
                    v.push_back(Wt::WString(*it));
        slm_->setStringList(v);
    }
}
