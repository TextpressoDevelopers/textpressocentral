/* 
 * File:   SuggestionBoxFromPg.cpp
 * Author: mueller
 * 
 * Created on March 25, 2014, 4:24 PM
 */

#include "SuggestionBoxFromPg.h"
#include <pqxx/pqxx>
#include <boost/algorithm/string.hpp>

SuggestionBoxFromPg::SuggestionBoxFromPg(std::string col, std::string where,
        std::string tablename, std::string databasename, Wt::WObject * parent)
: Wt::WSuggestionPopup(Wt::WSuggestionPopup::generateMatcherJS(contactOptions1),
Wt::WSuggestionPopup::generateReplacerJS(contactOptions1), parent) {
    col_ = col;
    where_ = where;
    tablename_ = tablename;
    databasename_ = databasename;
    BasicInit(parent);
}

void SuggestionBoxFromPg::BasicInit(Wt::WObject * parent) {
    slm_ = new Wt::WStringListModel();
    setFilterLength(4);
    setModel(slm_);
    setMaximumSize(Wt::WLength(60, Wt::WLength::FontEx), Wt::WLength(15, Wt::WLength::FontEx));
    filterModel().connect(this, &SuggestionBoxFromPg::PopulateModel);
}

SuggestionBoxFromPg::~SuggestionBoxFromPg() {
    delete slm_;
}

void SuggestionBoxFromPg::PopulateModel(const Wt::WString & s) {
    slm_->removeRows(0, slm_->rowCount());
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
        std::set<std::string> seen;
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string suggested;
            r[i][col_].to(suggested);
            std::set<std::string>::iterator it = seen.find(suggested);
            if (it == seen.end()) {
                slm_->addString(suggested);
                seen.insert(suggested);
            }
            if (seen.size() > 750) break; // suggestion box is getting too big!!
        }
        std::cerr << seen.size() << " terms suggested." << std::endl;
        w.commit();
    } else {
        std::cerr << "No matches in " << tablename_ << std::endl;
    }
    cn.disconnect();
}
