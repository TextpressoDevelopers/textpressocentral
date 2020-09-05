/* 
 * File:   TCNavWeb.h
 * Author: mueller
 *
 * Created on April 2, 2013, 2:24 PM
 */

#ifndef TCNAVWEB_H
#define	TCNAVWEB_H

#include "Session.h"
#include "UrlParameters.h"
#include "Home.h"
#include "Search.h"
#include "Curation.h"
#include "CurationDatabase.h"
#include "NLP.h"
#include "Ontology.h"
#include "Lists.h"
#include "Papers.h"
#include "Workflow.h"
#include "Login.h"
#include "Browsers.h"
#include "Customization.h"
#include "AboutUs.h"
#include "Download.h"
#include "Help.h"
#include "Permissions.h"
#include "AuthWidget.h"
#include <Wt/WContainerWidget>
#include <Wt/WMenu>
#include <Wt/WMenuItem>

class Curation;

struct PaperAddress {
    std::string rootdir;
    std::string paperdir;
    std::string paperfile;
    std::string title;
    std::string author;
    std::string journal;
    std::string year;
    std::string bestring;
    std::string accession;
    bool case_sensitive{false};
    std::vector<std::string> keywords;
    std::vector<std::string> categories;
};

class TCNavWeb : public Wt::WContainerWidget {
public:
    TCNavWeb(UrlParameters * urlparameters, Wt::WContainerWidget * parent = NULL);
    void DeleteSinglePaperItem();
    void SetSinglePaperItem(const PaperAddress& x);
    PaperAddress * GetSinglePaperItem() { return singlepaperitem_; }
    bool SinglePaperItemIsEmpty() { return (singlepaperitem_ == NULL); }
    Customization * GetCustomizationInstance() { return customization_; }
    Wt::WMenu * GetLeftMenu() { return leftMenu_; }
    Wt::WMenuItem * GetSearchMenuItem() { return searchmenuitem_; }
    bool paperHasChanged();
    void setNoChange();
private:
    Session session_;
    UrlParameters * urlparameters_;
    Wt::WText* loganchor_;
    Wt::WAnchor* verifieduserlabel_;
    Wt::WNavigationBar * navigation_;
    Wt::WStackedWidget * contentsStack_;
    PaperAddress * singlepaperitem_;
    //
    Home * home_;
    Curation * curation_;
    Search * search_;
    Customization * customization_;
    AboutUs * aboutus_;
    Download * download_;
    Help * help_;
    Permissions * permissions_;
    Papers * papers_;
    Browsers * browsers_;
    Ontology * ontology_;
    Lists * lists_;
    NLP * nlp_;
    CurationDatabase * curationdatabase_;
    Workflow * workflow_;
    //
    Wt::WMenu * leftMenu_;
    Wt::WMenuItem * homemenuitem_;
    Wt::WMenuItem * curationmenuitem_;
    Wt::WMenuItem * searchmenuitem_;
    Wt::WMenuItem * browsersmenuitem_;
    Wt::WMenuItem * papersmenuitem_;
    Wt::WMenuItem * customizationmenuitem_;
    void AuthEvent();
    void MenuItemTriggered(Wt::WMenuItem * x);
    void SearchEntered(Wt::WMenu * m, Wt::WLineEdit * edit);

    bool isUserVerified();

    void verifyEmailAddress();

    void SetCursorHand(WWidget *w);

    void displayLogin();
    AuthWidget *authWidget_;
    WDialog *dialog_;
    bool changed_;
};

#endif	/* TCNAVWEB_H */
