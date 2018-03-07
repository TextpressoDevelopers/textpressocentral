/* 
 * File:   SuggestionBoxFromPgOrWeb.h
 * Author: mueller
 *
 * Created on March 23, 2016, 1:45 PM
 */

#ifndef SUGGESTIONBOXFROMPGORWEB_H
#define	SUGGESTIONBOXFROMPGORWEB_H

#define LISTLIMIT 25000

#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WString>


struct lengthcompare {
    bool operator()(const std::string & first, const std::string & second) {
        return first.size() < second.size();
    }
};

const Wt::WSuggestionPopup::Options contactOptions1 = {
    /* highlightBeginTag */ "<b>",
    /* highlightEndTag */ "</b>",
    /* listSeparator */ '\n',
    /* whitespace */ " \n",
    /* wordSeparators */ "-., \"@\n;",
    //  /* appendReplacedText */ ", "
    /* appendReplacedText */ ""
};

class SuggestionBoxFromPgOrWeb : public Wt::WSuggestionPopup {
public:
    SuggestionBoxFromPgOrWeb(std::string col, std::string where,
            std::string tablename, std::string databasename, Wt::WObject * parent = NULL);
    virtual ~SuggestionBoxFromPgOrWeb();
private:
    void BasicInit(Wt::WObject * parent);
    void PopulateModel(const Wt::WString & s);
    Wt::WStringListModel * slm_;
    std::string col_;
    std::string where_;
    std::string tablename_;
    std::string databasename_;
    bool ispg_;
    std::string url_;
};

#endif	/* SUGGESTIONBOXFROMPGORWEB_H */
