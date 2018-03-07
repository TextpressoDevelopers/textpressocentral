/* 
 * File:   FetchList.cpp
 * Author: mueller
 * 
 * Created on March 22, 2016, 2:28 PM
 */

#include "FetchList.h"
#include <iostream>
#include <Wt/WApplication>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>

namespace {

    std::string urlencode(const std::string &s) {
        //RFC 3986 section 2.3 Unreserved Characters (January 2005)
        //const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
        //modify this so witty will not complain
        const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~:/?=&";

        std::string escaped = "";
        for (size_t i = 0; i < s.length(); i++) {
            if (unreserved.find_first_of(s[i]) != std::string::npos) {
                escaped.push_back(s[i]);
            } else {
                escaped.append("%");
                char buf[3];
                sprintf(buf, "%.2X", (unsigned char) s[i]);
                escaped.append(buf);
            }
        }
        return escaped;
    }
}

FetchList::FetchList(std::string location, std::string tablename,
        std::string columnname, std::string whereclause) {
    if (tablename.empty() || columnname.empty()) {
        url_ = location;
        boost::replace_first(url_, "host=", "");
        boost::trim(url_);
        ispg_ = false;
    } else {
        // postgres database
        cn_ = new pqxx::connection(location);
        ispg_ = true;
    }
    tablename_ = tablename;
    columnname_ = columnname;
    whereclause_ = whereclause;
}

FetchList::~FetchList() {
    cn_->disconnect();
}

bool FetchList::IsInList(std::string s) {
    if (list_.find(s) == list_.end()) LoadList(s);
    return (list_.find(s) != list_.end());
}

bool FetchList::HasList() {
    return !list_.empty();
}

void FetchList::LoadList(std::string s) {
    if (ispg_) {
        boost::replace_all(s, "(", "\\(");
        boost::replace_all(s, ")", "\\)");
        boost::replace_all(s, "[", "\\[");
        boost::replace_all(s, "]", "\\]");
        boost::replace_all(s, "{", "\\{");
        boost::replace_all(s, "}", "\\}");
        std::string search = "(^| )" + s;
        std::transform(search.begin(), search.end(), search.begin(), ::tolower);
        pqxx::work w(*cn_);
        pqxx::result r;
        std::stringstream pc;
        pc << "select " << columnname_ << " from ";
        pc << tablename_ << " where ";
        if (!whereclause_.empty()) pc << "(";
        pc << "lower(" << columnname_ << ") ~ '" << search << "'";
        if (!whereclause_.empty()) pc << ") AND (" << whereclause_ << ")";
        std::cout << pc.str() << std::endl;
        r = w.exec(pc.str());
        if (r.size() != 0) {
            std::cerr << r.size() << " results returned from postgres." << std::endl;
            for (pqxx::result::size_type i = 0; i != r.size(); i++) {
                std::string suggested;
                r[i][columnname_].to(suggested);
                list_.insert(suggested);
            }
        } else {
            std::cerr << "No matches in " << tablename_ << std::endl;
        }
        w.commit();
    } else {
        client_ = new Wt::Http::Client();
        client_->setTimeout(10);
        client_->done().connect(boost::bind(&FetchList::HandleHttpResponse, this, _1, _2));
        std::cerr << "url_+ s: " << url_ + s << std::endl;
        std::cerr << "urlencode(url_ + s): " << urlencode(url_ + s) << std::endl;
        if (!client_->get(urlencode(url_ + s)))
            std::cerr << "Post request in LoadList could not be scheduled." << std::endl;
        else
            Wt::WApplication::instance()->deferRendering();
    }
}

void FetchList::HandleHttpResponse(boost::system::error_code err, const Wt::Http::Message & response) {
    Wt::WApplication::instance()->resumeRendering();
    if (response.status() == 200) {
        std::vector<std::string> splits;
        std::string aux(response.body());
        std::cerr << "AUX: " << aux << std::endl;
        boost::split(splits, aux, boost::is_any_of("\n"));
        while (!splits.empty()) {
            if (!splits.back().empty()) {
                list_.insert(splits.back());
                std::cerr << "SB: [" << splits.back() << "]" << std::endl;
            }
            splits.pop_back();
        }
    } else {
        std::stringstream resp;
        resp << response.status() << " ";
        resp << response.body() << std::endl;
    }
    delete client_;
    usleep(500);
}
