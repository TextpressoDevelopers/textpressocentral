/* 
 * File:   CategoryPreferences.cpp
 * Author: mueller
 * 
 * Created on March 16, 2015, 2:54 PM
 */

#include "CategoryPreferences.h"
#include "TpCategoryBrowser.h"
#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WPushButton>
#include <Wt/WGridLayout>

CategoryPreferences::CategoryPreferences(Session * session,
        std::set<std::string> & preloaded) {
    setWindowTitle("Change Tree Preferences");
    setModal(false);
    setResizable(true);
    setClosable(true);

    Wt::WContainerWidget * gridcont = new Wt::WContainerWidget();
    int x(0), y(0), count(0), squaresize(3);
    Wt::WGridLayout * grid = new Wt::WGridLayout();
    gridcont->setLayout(grid);
    std::set<std::string> plist;
    TpCategoryBrowser * tcp = new TpCategoryBrowser();
    plist = tcp->GetAllDirectChildrensName("root");
    delete tcp;
    std::set<std::string>::iterator it;
    for (it = plist.begin(); it != plist.end(); it++) {
        Wt::WCheckBox * aux = new Wt::WCheckBox(*it);
        if (preloaded.find(*it) != preloaded.end()) {
            aux->setChecked(true);
            choices_[*it] = true;
        }
        aux->changed().connect(boost::bind(&CategoryPreferences::ChangedChoices,
                this, aux, *it));
        x = count % squaresize;
        y = count / squaresize;
        grid->addWidget(aux, y, x);
        count++;
    }
    gridcont->resize(Wt::WLength(40 * squaresize, Wt::WLength::FontEx),
            Wt::WLength(5 * (y+1), Wt::WLength::FontEx));
    Wt::WString savetext("Save Preferences");
    if (session->login().state() == 0) savetext += " (disabled -- not logged in)";
    Wt::WCheckBox * savebox = new Wt::WCheckBox(savetext);
    saveclicked_ = false;
    savebox->setChecked(saveclicked_);
    savebox->changed().connect(boost::bind(&CategoryPreferences::ChangedSaveBox,
            this, savebox));
    if (session->login().state() != 0)
        savebox->enable();
    else
        savebox->disable();
    contents()->addWidget(gridcont);
    contents()->addWidget(new Wt::WBreak());
    contents()->addWidget(savebox);
    contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * okbt = new Wt::WPushButton("Ok");
    okbt->clicked().connect(this, &Wt::WDialog::accept);
    contents()->addWidget(okbt);
}

void CategoryPreferences::ChangedChoices(Wt::WCheckBox * cb, std::string s) {
    choices_[s] = cb->isChecked();
}

void CategoryPreferences::ChangedSaveBox(Wt::WCheckBox * cb) {
    saveclicked_ = cb->isChecked();
}

CategoryPreferences::~CategoryPreferences() {
}

