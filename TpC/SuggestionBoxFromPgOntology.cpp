/* 
 * File:   SuggestionBoxFromPgOntology.cpp
 * Author: mueller
 * 
 * Created on March 5, 2014, 1:39 PM
 */

#include "SuggestionBoxFromPgOntology.h"
#include <pqxx/pqxx>
#include <boost/algorithm/string.hpp>

SuggestionBoxFromPgOntology::SuggestionBoxFromPgOntology(std::string col,
        std::string where, std::string tablename, std::string databasename, Wt::WObject * parent)
: Wt::WSuggestionPopup(Wt::WSuggestionPopup::generateMatcherJS(contactOptions),
Wt::WSuggestionPopup::generateReplacerJS(contactOptions), parent) {
    col_ = col;
    where_ = where;
    tablename_ = tablename;
    databasename_ = databasename;
    BasicInit(parent);
}

void SuggestionBoxFromPgOntology::BasicInit(Wt::WObject * parent) {
    slm_ = new Wt::WStringListModel();
    setFilterLength(4);
    setModel(slm_);
    setMaximumSize(Wt::WLength(60, Wt::WLength::FontEx), Wt::WLength(15, Wt::WLength::FontEx));
    filterModel().connect(this, &SuggestionBoxFromPgOntology::PopulateModel);
}

void SuggestionBoxFromPgOntology::PopulateModel(const Wt::WString & s) {
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
    std::cerr << "PopulateModel: connected to " << cn.dbname() << std::endl;
    pqxx::work w(cn);
    pqxx::result r;
    std::stringstream pc;
    pc << "select eid," << col_ << " from ";
    pc << tablename_ << " where ";
    if (!where_.empty()) pc << "(";
    pc << "lower(" << col_ << ") ~ '" << search << "'";
    if (!where_.empty()) pc << ") AND (" << where_ << ")";
    r = w.exec(pc.str());
    std::cout << pc.str() << std::endl;
    std::cerr << pc.str() << std::endl;
    if (r.size() != 0) {
        std::cerr << r.size() << " results returned from postgres." << std::endl;
        std::set<std::string> seen;
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string sauxcol;
            std::string sauxeid;
            r[i][col_].to(sauxcol);
            r[i]["eid"].to(sauxeid);
            std::string suggested = sauxcol + " (" + sauxeid + ")";
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
