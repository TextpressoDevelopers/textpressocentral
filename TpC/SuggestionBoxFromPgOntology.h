/* 
 * File:   SuggestionBoxFromPgOntology.h
 * Author: mueller
 *
 * Created on March 5, 2014, 1:39 PM
 */

#ifndef SUGGESTIONBOXFROMPGONTOLOGY_H
#define	SUGGESTIONBOXFROMPGONTOLOGY_H

#include <Wt/WSuggestionPopup>
#include <Wt/WStringListModel>
#include <Wt/WString>

const Wt::WSuggestionPopup::Options contactOptions = {
    /* highlightBeginTag */ "<b>",
    /* highlightEndTag */ "</b>",
    /* listSeparator */ '\n',
    /* whitespace */ " \n",
    /* wordSeparators */ "-., \"@\n;",
//  /* appendReplacedText */ ", "
    /* appendReplacedText */ ""
};

class SuggestionBoxFromPgOntology : public Wt::WSuggestionPopup {
public:
    SuggestionBoxFromPgOntology(std::string col, std::string where,
            std::string tablename, std::string databasename, Wt::WObject * parent = NULL);
private:
    void BasicInit(Wt::WObject * parent);
    void PopulateModel(const Wt::WString & s);
    Wt::WStringListModel * slm_;
    std::string col_;
    std::string where_;
    std::string tablename_;
    std::string databasename_;
};

#endif	/* SUGGESTIONBOXFROMPGONTOLOGY_H */

