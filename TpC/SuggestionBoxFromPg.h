/* 
 * File:   SuggestionBoxFromPg.h
 * Author: mueller
 *
 * Created on March 25, 2014, 4:24 PM
 */

#ifndef SUGGESTIONBOXFROMPG_H
#define	SUGGESTIONBOXFROMPG_H

#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WString>

const Wt::WSuggestionPopup::Options contactOptions1 = {   
    /* highlightBeginTag */ "<b>",
    /* highlightEndTag */ "</b>",
    /* listSeparator */ '\n',
    /* whitespace */ " \n",
    /* wordSeparators */ "-., \"@\n;",
//  /* appendReplacedText */ ", "
    /* appendReplacedText */ ""
};

class SuggestionBoxFromPg : public Wt::WSuggestionPopup {
public:
    SuggestionBoxFromPg(std::string col,  std::string where,
            std::string tablename, std::string databasename, Wt::WObject * parent = NULL);
    void SetPgColumn(std::string s) { col_ = s; };
    virtual ~SuggestionBoxFromPg();
private:
    void BasicInit(Wt::WObject * parent);
    void PopulateModel(const Wt::WString & s);
    Wt::WStringListModel * slm_;
    std::string col_;
    std::string where_;
    std::string tablename_;
    std::string databasename_;
};

#endif	/* SUGGESTIONBOXFROMPG_H */
