/* 
 * File:   Home.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 11:46 AM
 */

#include "Home.h"
#include "TCNavWeb.h"
#include "PgList.h"
#include "Session.h"

#include <Wt/WBreak>
#include <Wt/WText>
#include <Wt/WLabel>

Home::Home(Session& session, Wt::WContainerWidget * parent) : parent_(parent), Wt::WContainerWidget(parent) {
    session_ = &session;
}

void Home::LoadContent(Search * search) {
    //    Wt::WBorderLayout * mainlayout = new Wt::WBorderLayout();
    Wt::WVBoxLayout * mainlayout = new Wt::WVBoxLayout();
    setLayout(mainlayout);
    Wt::WContainerWidget * center = new Wt::WContainerWidget();
    Wt::WContainerWidget * north = new Wt::WContainerWidget();
    Wt::WContainerWidget * south = new Wt::WContainerWidget();
    Wt::WContainerWidget * west = new Wt::WContainerWidget();
    Wt::WContainerWidget * east = new Wt::WContainerWidget();
    mainlayout->addWidget(center, Wt::WBorderLayout::Center);
    mainlayout->addWidget(north, Wt::WBorderLayout::North);
    mainlayout->addWidget(south, Wt::WBorderLayout::South);
    mainlayout->addWidget(west, Wt::WBorderLayout::West);
    mainlayout->addWidget(east, Wt::WBorderLayout::East);
    center->resize(Wt::WLength("50%"), Wt::WLength(80, Wt::WLength::Percentage));
    south->resize(Wt::WLength("50%"), Wt::WLength(20, Wt::WLength::Percentage));
    south->setMinimumSize(Wt::WLength("50%"), Wt::WLength(20, Wt::WLength::Percentage));
    //south->setMaximumSize(Wt::WLength("50%"), Wt::WLength(20, Wt::WLength::Percentage));
    //south->setWidth(WLength(50, WLength::Percentage));
    //
    center->setContentAlignment(Wt::AlignCenter);
    Wt::WText * banner = new Wt::WText("Textpresso Central");
    banner->decorationStyle().setForegroundColor(Wt::WColor(70, 90, 180));
    banner->decorationStyle().font().setWeight(Wt::WFont::Bold);
    banner->decorationStyle().font().setSize(Wt::WFont::XXLarge);
    banner->decorationStyle().font().setStyle(Wt::WFont::Italic);
    banner->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    center->addWidget(banner);
    //
    center->addWidget(new Wt::WBreak());
    //
    Wt::WText * subtitle = new Wt::WText("Knowledge discovery through full text mining, classification and searching");
    //    subtitle->decorationStyle().font().setSize(Wt::WFont::Default);
    subtitle->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    center->addWidget(subtitle);
    //
    center->addWidget(new Wt::WBreak());
    center->addWidget(new Wt::WBreak());
    //
    if (search != NULL) {
        querycont = new Wt::WContainerWidget();
        querycont->decorationStyle().setBackgroundColor(Wt::WColor(220, 220, 255));
        querycont->setWidth(Wt::WLength("50%"));
        center->addWidget(querycont);
        Wt::WText * searchlabel = new Wt::WText("Search corpus: ");
        searchlabel->decorationStyle().font().setSize(Wt::WFont::Large);
        searchlabel->resize(Wt::WLength(20, Wt::WLength::FontEx), Wt::WLength(3, Wt::WLength::FontEx));
        Wt::WLineEdit * keyword = new Wt::WLineEdit();
        keyword->resize(Wt::WLength(20, Wt::WLength::FontEx), Wt::WLength(3, Wt::WLength::FontEx));
        Wt::WImage * searchicon = new Wt::WImage("resources/icons/search-icon.png");
        searchicon->resize(Wt::WLength(24, Wt::WLength::Pixel), Wt::WLength(24, Wt::WLength::Pixel));
        searchicon->mouseWentOver().connect(boost::bind(&Home::SetCursorHand, this, searchicon));
        searchicon->clicked().connect(std::bind([ = ] (){
            if (!keyword->text().empty()) {
                TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent_);
                        tcnw->GetLeftMenu()->select(tcnw->GetSearchMenuItem());
                        search->SimpleKeywordSearchApi(keyword->text());
            }
        }));
        keyword->enterPressed().connect(std::bind([ = ](){
            if (!keyword->text().empty()) {
                TCNavWeb * tcnw = dynamic_cast<TCNavWeb*> (parent_);
                        tcnw->GetLeftMenu()->select(tcnw->GetSearchMenuItem());
                        search->SimpleKeywordSearchApi(keyword->text());
            }
        }));
        Wt::WAnchor * advsearch = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/search"), "Advanced Search");
        querycont->addWidget(searchlabel);
        querycont->addWidget(keyword);
        querycont->addWidget(searchicon);
        querycont->addWidget(advsearch);
        Wt::WContainerWidget * litcont = new Wt::WContainerWidget();
        search->SetLiteratureDescription(litcont);
        querycont->addWidget(litcont);
    }
    //
    center->addWidget(new Wt::WBreak());
    //center->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * notice = new Wt::WContainerWidget();
    notice = new Wt::WContainerWidget();
    notice->setContentAlignment(Wt::AlignLeft);
    notice->decorationStyle().setBackgroundColor(Wt::WColor(255, 64, 129));
    notice->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    notice->setWidth(Wt::WLength("50%"));
    center->addWidget(notice);
    notice->addWidget(new Wt::WText("This is the new Textpresso site. You can "
            "still access the "));
    Wt::WAnchor * oldsite = new Wt::WAnchor("http://textpresso.org/index_old.html");
    oldsite->setTarget(Wt::TargetNewWindow);
    oldsite->setText("old site");
    oldsite->decorationStyle().setForegroundColor(Wt::WColor(200, 200, 255));
    notice->addWidget(oldsite);
    notice->addWidget(new Wt::WText(", but it will be retired, probably by the "
            "end of the year 2018. This new site is described in more detail "
            "in our "));
    Wt::WAnchor * paper = new Wt::WAnchor("https://bmcbioinformatics.biomedcentral.com/articles/10.1186/s12859-018-2103-8");
    paper->decorationStyle().setForegroundColor(Wt::WColor(200, 200, 255));
    paper->setTarget(Wt::TargetNewWindow);
    paper->setText("paper");
    notice->addWidget(paper);
    notice->addWidget(new Wt::WText(", published in March 2018."));
    Wt::WContainerWidget * sftcont = new Wt::WContainerWidget();
    sftcont->setWidth(Wt::WLength("50%"));
    center->addWidget(sftcont);
    Wt::WGroupBox * sftgroup = new Wt::WGroupBox("Search Full Text");
    sftgroup->setContentAlignment(Wt::AlignJustify);
    Wt::WText * sfttext1 = new Wt::WText("Textpresso Central contains "
            "(roughly 1.5 million) PMCOA full text articles. Number of "
            "articles are continuously growing.");
    sfttext1->decorationStyle().font().setSize(Wt::WFont::Large);
    sfttext1->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    sftgroup->addWidget(sfttext1);
    sftcont->addWidget(sftgroup);
    center->addWidget(sftcont);
    //
    center->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * frfcont = new Wt::WContainerWidget();
    frfcont->setWidth(Wt::WLength("50%"));
    center->addWidget(frfcont);
    Wt::WGroupBox * frfgroup = new Wt::WGroupBox("Find Relevant Facts");
    frfgroup->setContentAlignment(Wt::AlignJustify);
    Wt::WText * frftext1 = new Wt::WText("Perform simple keyword searches or more advanced searches to answer specific "
            "biological questions. Search results are presented within the "
            "context of the full text for rapid assessment of relevancy.");
    frftext1->decorationStyle().font().setSize(Wt::WFont::Large);
    frftext1->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    frfgroup->addWidget(frftext1);
    frfgroup->addWidget(new Wt::WBreak());
    //frfgroup->addWidget(new Wt::WBreak());
    frfcont->addWidget(frfgroup);
    center->addWidget(frfcont);
    //
    center->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * curcont = new Wt::WContainerWidget();
    curcont->setWidth(Wt::WLength("50%"));
    center->addWidget(curcont);
    Wt::WGroupBox * curgroup = new Wt::WGroupBox("Curate");
    curgroup->setContentAlignment(Wt::AlignJustify);
    Wt::WText * curtext1 = new Wt::WText("Add annotations to papers and "
            "send them to any external curation database.");
    curtext1->decorationStyle().font().setSize(Wt::WFont::Large);
    curtext1->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    curgroup->addWidget(curtext1);
    curcont->addWidget(curgroup);
    //
    center->addWidget(new Wt::WBreak());
    center->addWidget(new Wt::WBreak());
    center->addWidget(new Wt::WBreak());
    //
    Wt::WColor tipcolor(Wt::WColor(255, 215, 90));
    Wt::WColor newscolor(Wt::WColor(255, 60, 60));
    Wt::WColor introcolor(Wt::WColor(100, 180, 100));
    Wt::WColor nextstepcolor(Wt::WColor(180, 100, 180));
    Wt::WColor contactcolor(Wt::WColor(0, 0, 0));
    //
    Wt::WAnchor * facebookanchor = new Wt::WAnchor("https://facebook.com/textpressocentral");
    Wt::WImage * im = new Wt::WImage("resources/icons/find.us.on.facebook.lg.png");
    im->setVerticalAlignment(Wt::AlignBottom);
    im->setWidth(WLength(15, Wt::WLength::FontEx));
    facebookanchor->setImage(im);
    facebookanchor->setTarget(Wt::TargetNewWindow);
    facebookanchor->decorationStyle().setBackgroundColor(WColor(220, 220, 220));
    //
    //Wt::WContainerWidget * sw = new Wt::WContainerWidget();
    //Wt::WContainerWidget * se = new Wt::WContainerWidget();
    //sw->setMaximumSize(Wt::WLength(20, Wt::WLength::Percentage), Wt::WLength(36, Wt::WLength::FontEx));
    //sw->setContentAlignment(Wt::AlignCenter);
    //se->setMinimumSize(Wt::WLength(80, Wt::WLength::Percentage), Wt::WLength(36, Wt::WLength::FontEx));
    Wt::WVBoxLayout * vbox = new Wt::WVBoxLayout();
    south->setLayout(vbox);
    auto southButtons = new Wt::WContainerWidget();
    auto hbox = new Wt::WHBoxLayout();
    southButtons->setLayout(hbox);
    vbox->addWidget(southButtons);
    auto southText = new Wt::WContainerWidget();
    auto vboxText = new Wt::WVBoxLayout();
    southText->setLayout(vboxText);
    vbox->addWidget(southText);
    //
    Wt::WImage * tipimage = new Wt::WImage("resources/other_images/didyouknow.png");
    tipimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    tipimage->setInline(true);
    Wt::WText * tt = new Wt::WText(" Tip of the Day");
    tt->setInline(true);
    Wt::WContainerWidget * tipcontainer = new Wt::WContainerWidget();
    tipcontainer->decorationStyle().setBackgroundColor(tipcolor);
    tipcontainer->addWidget(tipimage);
    tipcontainer->addWidget(tt);
    //
    /*
    Wt::WImage * newsimage = new Wt::WImage("resources/other_images/news.png");
    newsimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    newsimage->setInline(true);
    Wt::WText * tn = new Wt::WText(" News");
    tn->setInline(true);
    Wt::WContainerWidget * newscontainer = new Wt::WContainerWidget();
    newscontainer->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    newscontainer->decorationStyle().setBackgroundColor(newscolor);
    newscontainer->addWidget(newsimage);
    newscontainer->addWidget(tn);
    */
    //
    Wt::WImage * introimage = new Wt::WImage("resources/other_images/glowingbulb.png");
    introimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    introimage->setInline(true);
    Wt::WText * ti = new Wt::WText(" Introduction");
    ti->setInline(true);
    Wt::WContainerWidget * introcontainer = new Wt::WContainerWidget();
    introcontainer->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    introcontainer->decorationStyle().setBackgroundColor(introcolor);
    introcontainer->addWidget(introimage);
    introcontainer->addWidget(ti);
    //
    Wt::WImage * nextstepimage = new Wt::WImage("resources/other_images/footprints.png");
    nextstepimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    nextstepimage->setInline(true);
    Wt::WText * tns = new Wt::WText(" First Steps");
    tns->setInline(true);
    Wt::WContainerWidget * nextstepcontainer = new Wt::WContainerWidget();
    nextstepcontainer->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    nextstepcontainer->decorationStyle().setBackgroundColor(nextstepcolor);
    nextstepcontainer->addWidget(nextstepimage);
    nextstepcontainer->addWidget(tns);
    //
    Wt::WImage * contactimage = new Wt::WImage("resources/other_images/handshake.png");
    contactimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    contactimage->setInline(true);
    Wt::WText * tc = new Wt::WText(" Contact Us");
    tc->setInline(true);
    auto contactanchor = new WAnchor("mailto:textpresso@caltech.edu");
    contactanchor->addWidget(contactimage);
    contactanchor->decorationStyle().setForegroundColor(WColor(255, 255, 255));
    contactanchor->decorationStyle().setBackgroundColor(WColor(50, 50, 200));
    contactanchor->addWidget(tc);

    hbox->addWidget(tipcontainer);
    //hbox->addWidget(newscontainer);
    hbox->addWidget(introcontainer);
    hbox->addWidget(nextstepcontainer);
    hbox->addWidget(contactanchor);
    hbox->addWidget(facebookanchor);

    southText->setWidth(WLength("80%"));
    south->setWidth(WLength("80%"));
    southText->setContentAlignment(AlignCenter);

    Wt::WTimer * timer = new Wt::WTimer();
    timer->setInterval(15000);
    timer->start();
    timer->timeout().connect(boost::bind(&Home::DisplayRandomItem, this, southText));
    //
    tipcontainer->clicked().connect(std::bind([ = ] (){
        southText->clear();
        timer->stop();
        PgList * tips = new PgList(PGTIPOFDAY, PGTIPOFDAYTABLENAME);
        Wt::WText * tiptext = new Wt::WText(tips->GetRandomItem());
        delete tips;
        southText->addWidget(tiptext);
    }));
    /*
    newscontainer->clicked().connect(std::bind([ = ] (){
        southText->clear();
        timer->stop();
        std::string news("/usr/local/textpresso/resources/resources-web/home.news.txt");
        DisplayTextAndLinksFromFile(news, newscolor, southText);
    }));
    */
    introcontainer->clicked().connect(std::bind([ = ] (){
        southText->clear();
        timer->stop();
        std::string intro("/usr/local/textpresso/resources/resources-web/home.introduction.txt");
        DisplayTextAndLinksFromFile(intro, introcolor, southText);
    }));
    nextstepcontainer->clicked().connect(std::bind([ = ] (){
        southText->clear();
        timer->stop();
        std::string nextstep("/usr/local/textpresso/resources/resources-web/home.firststeps.txt");
        DisplayTextAndLinksFromFile(nextstep, nextstepcolor, southText);
    }));
    //contactcontainer->clicked().connect(std::bind([ = ] (){
    //    southText->clear();
    //    timer->stop();
    //    std::string contact("/usr/local/textpresso/resources/resources-web/home.contactus.txt");
    //    DisplayTextAndLinksFromFile(contact, contactcolor, southText);
    //}));
    tipcontainer->mouseWentOver().connect(boost::bind(&Home::SetCursorHand, this, tipcontainer));
    //newscontainer->mouseWentOver().connect(boost::bind(&Home::SetCursorHand, this, newscontainer));
    introcontainer->mouseWentOver().connect(boost::bind(&Home::SetCursorHand, this, introcontainer));
    nextstepcontainer->mouseWentOver().connect(boost::bind(&Home::SetCursorHand, this, nextstepcontainer));
    //contactcontainer->mouseWentOver().connect(boost::bind(&Home::SetCursorHand, this, contactcontainer));
}

void Home::DisplayRandomItem(Wt::WContainerWidget * se) {
    int pick = rand() % 5;
    se->clear();
    switch (pick) {
        case 0:
        {
            // Code
            PgList * tips = new PgList(PGTIPOFDAY, PGTIPOFDAYTABLENAME);
            Wt::WText * tiptext = new Wt::WText(tips->GetRandomItem());
            delete tips;
            se->addWidget(tiptext);
            break;
        }
        case 1:
        {
            // Code
            std::string news("/usr/local/textpresso/resources/resources-web/home.news.txt");
            DisplayTextAndLinksFromFile(news, Wt::blue, se);
            break;
        }
        case 2:
        {
            // Code
            std::string intro("/usr/local/textpresso/resources/resources-web/home.introduction.txt");
            DisplayTextAndLinksFromFile(intro, Wt::blue, se);
            break;
        }
        case 3:
        {
            // Code
            std::string nextstep("/usr/local/textpresso/resources/resources-web/home.firststeps.txt");
            DisplayTextAndLinksFromFile(nextstep, Wt::blue, se);
            break;
        }
        case 4:
        {
            std::string contact("/usr/local/textpresso/resources/resources-web/home.contactus.txt");
            DisplayTextAndLinksFromFile(contact, Wt::blue, se);
            break;
        }
    }


}

void Home::DisplayTextAndLinksFromFile(std::string filename,
        Wt::WColor textcolor, Wt::WContainerWidget * container) {
    std::ifstream f(filename.c_str());
    std::string in;
    while (getline(f, in)) {
        std::vector<std::string> splits;
        boost::split(splits, in, boost::is_any_of("\t"));
        std::string text(splits[0]);
        if (splits.size() > 1) {
            Wt::WLink link;
            link.setUrl(splits[1]);
            Wt::WAnchor * anchor = new Wt::WAnchor(link);
            anchor->setText(text);
            if (splits.size() > 2) {
                if (splits[2].compare("this_window") == 0)
                    anchor->setTarget(Wt::TargetThisWindow);
                else if (splits[2].compare("self_window") == 0)
                    anchor->setTarget(Wt::TargetSelf);
            } else
                anchor->setTarget(Wt::TargetNewWindow);
            container->addWidget(anchor);
        } else if (splits.size() == 1) {
            if (splits[0].length() == 0) {
                container->addWidget(new Wt::WBreak());
            } else if (splits[0].length() == 1) {
                if (splits[0].compare("*") == 0) {
                    Wt::WImage * starimage = new Wt::WImage("resources/other_images/goldstar_small.jpg");
                    starimage->resize(Wt::WLength(2, Wt::WLength::FontEx), Wt::WLength(2, Wt::WLength::FontEx));
                    starimage->setInline(true);
                    starimage->setInline(true);
                    starimage->setVerticalAlignment(Wt::AlignMiddle);
                    container->addWidget(starimage);
                    container->addWidget(new Wt::WText(" "));
                } else if (splits[0].compare(" ") == 0) {
                    container->addWidget(new Wt::WText("&nbsp;"));
                }
            } else {
                Wt::WText * ttext = new Wt::WText(text);
                ttext->decorationStyle().setForegroundColor(textcolor);
                container->addWidget(ttext);
            }
        }
    }
    f.close();
}

void Home::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}
