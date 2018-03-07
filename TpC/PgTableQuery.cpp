/* 
 * File:   PgTableQuery.cpp
 * Author: mueller
 * 
 * Created on March 24, 2014, 12:55 PM
 */

#include "PgTableQuery.h"

#include <Wt/WGridLayout>
#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WCssDecorationStyle>
#include <Wt/WColor>
#include <Wt/WLineEdit>
#include <Wt/WAnchor>
#include <Wt/WImage>
#include <Wt/WComboBox>
#include <pqxx/pqxx>
#include <boost/algorithm/string.hpp>

PgTableQuery::PgTableQuery(std::string databasename, std::string tablename,
        Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    clear();
    Wt::WGridLayout * l = new Wt::WGridLayout();
    setLayout(l);
    databasename_ = databasename;
    tablename_ = tablename;
    suggestionbox4lineedit_ = NULL;
    Wt::WText * title1 = new Wt::WText("Pick a column name and enter a term: ");
    title1->decorationStyle().font().setWeight(Wt::WFont::Bold);
    title1->decorationStyle().setForegroundColor(Wt::WColor(0, 68, 204));
    title1->setVerticalAlignment(Wt::AlignMiddle);
    l->addWidget(title1, 0, 0);
    std::vector<std::string> headers = GetColumnHeaders();
    lineedit_ = new Wt::WLineEdit();
    lineedit_->setMaximumSize(Wt::WLength(30, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    lineedit_->hide();
    lineedit_->setVerticalAlignment(Wt::AlignMiddle);
    lineedit_->enterPressed().connect(boost::bind(&PgTableQuery::LineEditEnterPressed, this));
    lineedit_->setEmptyText("Enter a term");
    suggestionbox4lineedit_ = new SuggestionBoxFromPg("", "", tablename_, databasename_);
    suggestionbox4lineedit_->forEdit(lineedit_);
    headerscombobox_ = new Wt::WComboBox();
    headerscombobox_->setMaximumSize(Wt::WLength(30, Wt::WLength::FontEx),
            Wt::WLength(4, Wt::WLength::FontEx));
    headerscombobox_->setVerticalAlignment(Wt::AlignMiddle);
    headerscombobox_->addItem(Wt::WString());
    std::sort(headers.begin(), headers.end(), std::greater<std::string > ());
    while (!headers.empty()) {
        headerscombobox_->addItem(headers.back());
        headers.pop_back();
    }
    headerscombobox_->changed().connect(boost::bind(&PgTableQuery::ComboBoxValueChanged, this));
    Wt::WContainerWidget * aux = new Wt::WContainerWidget();
    aux->addWidget(headerscombobox_);
    aux->addWidget(lineedit_);
    headerscombobox_->setInline(true);
    lineedit_->setInline(true);
    l->addWidget(aux, 1, 0);
    Wt::WText * title2 = new Wt::WText("or formulate the full where clause yourself: ");
    title2->decorationStyle().font().setWeight(Wt::WFont::Bold);
    title2->decorationStyle().setForegroundColor(Wt::WColor(0, 68, 204));
    title2->setVerticalAlignment(Wt::AlignMiddle);
//    l->addWidget(title2, 2, 0);
//    l->addWidget(new Wt::WText(" "), 2, 0);
    Wt::WAnchor * a = new Wt::WAnchor("http://www.postgresql.org/docs/9.3/static/queries-table-expressions.html#QUERIES-WHERE");
    Wt::WImage * im = new Wt::WImage("resources/icons/qmark15.png");
    im->setVerticalAlignment(Wt::AlignTop);
    a->setImage(im);
    a->setTarget(Wt::TargetNewWindow);
    a->setVerticalAlignment(Wt::AlignSuper);
    Wt::WContainerWidget * aux2 = new Wt::WContainerWidget();
    title2->setInline(true);
    a->setInline(true);
    aux2->addWidget(title2);
    aux2->addWidget(new Wt::WText());
    aux2->addWidget(a);
    l->addWidget(aux2, 0, 2);
    where_ = new Wt::WLineEdit();
    where_->setMaximumSize(Wt::WLength(30, Wt::WLength::FontEx),
            Wt::WLength(3, Wt::WLength::FontEx));
//    where_->setWidth(Wt::WLength(20,Wt::WLength::FontEx));
    where_->setText(Wt::WString("where "));
    where_->setVerticalAlignment(Wt::AlignMiddle);
    where_->enterPressed().connect(boost::bind(&PgTableQuery::WhereFieldEnterPressed, this));
    l->addWidget(where_, 1, 2);
    setMaximumSize(Wt::WLength::Auto, Wt::WLength(30, Wt::WLength::FontEx));
}

std::vector<std::string> PgTableQuery::GetColumnHeaders() {
    pqxx::connection cn(databasename_);
    std::vector<std::string> ret;
    try {
        pqxx::work w(cn);
        pqxx::result r;
        std::stringstream pc;
        pc << "select column_name from information_schema.columns where table_name ='";
        pc << tablename_ << "'";
        r = w.exec(pc.str());
        for (pqxx::result::size_type i = 0; i != r.size(); i++) {
            std::string saux;
            r[i]["column_name"].to(saux);
            ret.push_back(saux);
        }
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.
        std::cerr << e.what() << std::endl;
    }
    cn.disconnect();
    return ret;
}

void PgTableQuery::ComboBoxValueChanged() {
    suggestionbox4lineedit_->SetPgColumn(headerscombobox_->currentText().toUTF8());
    lineedit_->setText("");
    lineedit_->show();
}

void PgTableQuery::LineEditEnterPressed() {
    if (!(lineedit_->text()).empty()) {
        condition_ = " where " + headerscombobox_->currentText().toUTF8();
        condition_ += " ~ '" + lineedit_->text().toUTF8() + "'";
        boost::replace_all(condition_, "(", "\\(");
        boost::replace_all(condition_, ")", "\\)");
        boost::replace_all(condition_, "[", "\\[");
        boost::replace_all(condition_, "]", "\\]");
        boost::replace_all(condition_, "{", "\\{");
        boost::replace_all(condition_, "}", "\\}");
        std::cerr << "COND: " << condition_ << std::endl;
        SearchPg();
        std::cerr << "SIZE: " << pgresult_.size() << std::endl;
        done_.emit(pgresult_.size());
    }
}

void PgTableQuery::WhereFieldEnterPressed() {
    condition_ = where_->text().toUTF8();
    SearchPg();
    done_.emit(pgresult_.size());
}

void PgTableQuery::SearchPg() {
    pqxx::connection cn(databasename_);
    try {
        pqxx::work w(cn);
        std::stringstream pc;
        pc << "select * from ";
        pc << tablename_ << " ";
        pc << condition_;
        pgresult_ = w.exec(pc.str());
        w.commit();
    } catch (const std::exception &e) {
        // There's no need to check our database calls for errors.  If
        // any of them fails, it will throw a normal C++ exception.
        std::cerr << e.what() << std::endl;
    }
    cn.disconnect();
}
