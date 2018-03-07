/* 
 * File:   OntologyTermQuery.h
 * Author: mueller
 *
 * Created on July 19, 2017, 10:09 PM
 */

#ifndef ONTOLOGYTERMQUERY_H
#define	ONTOLOGYTERMQUERY_H

#include <Wt/WContainerWidget>
#include <Wt/WLineEdit>

class OntologyTermQuery : public Wt::WContainerWidget {
public:
    OntologyTermQuery();
    OntologyTermQuery(const OntologyTermQuery& orig);
    virtual ~OntologyTermQuery();
    Wt::Signal<void> & TermEntered() { return termentered_; }
    Wt::WString GetTerm() { return term_->text(); }
private:
    Wt::Signal<void> termentered_;
    Wt::WLineEdit *term_;
};

#endif	/* ONTOLOGYTERMQUERY_H */

