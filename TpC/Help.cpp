/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Help.cpp
 * Author: mueller
 * 
 * Created on July 24, 2019, 6:33 PM
 */

#include "TextpressoCentralGlobalDefinitions.h"
#include "Help.h"
#include "PgList.h"
#include <Wt/WColor>
#include <Wt/WAnchor>
#include <Wt/WLength>
#include <Wt/WImage>
#include <Wt/WText>
#include <Wt/WCssStyleSheet>
#include <Wt/WLength>
#include <Wt/WVBoxLayout>
#include <Wt/WHBoxLayout>
#include <Wt/WTimer>
#include <fstream>
#include <boost/algorithm/string.hpp>

namespace {

    void SetCursorHand(Wt::WWidget * w) {
        w->decorationStyle().setCursor(Wt::PointingHandCursor);
    }
}

Help::Help() {
}

void Help::LoadContent() {
    //
    Wt::WColor tipcolor(Wt::WColor(255, 215, 90));
    Wt::WColor newscolor(Wt::WColor(255, 60, 60));
    Wt::WColor introcolor(Wt::WColor(100, 180, 100));
    Wt::WColor nextstepcolor(Wt::WColor(180, 100, 180));
    Wt::WColor contactcolor(Wt::WColor(50, 50, 200));
    //

    //
    setWidth(Wt::WLength(80, Wt::WLength::FontEx));
    Wt::WImage * introimage = new Wt::WImage("resources/other_images/glowingbulb.png");
    introimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    introimage->setInline(true);
    Wt::WText * ti = new Wt::WText(" Introduction");
    ti->setInline(true);
    Wt::WContainerWidget * introbanner = new Wt::WContainerWidget();
    introbanner->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    introbanner->decorationStyle().setBackgroundColor(introcolor);
    introbanner->addWidget(introimage);
    introbanner->addWidget(ti);
    addWidget(introbanner);
    //
    std::string intro("/usr/local/textpresso/resources/resources-web/home.introduction.txt");
    Wt::WContainerWidget * introtext = new Wt::WContainerWidget();
    DisplayTextAndLinksFromFile(intro, introcolor, introtext);
    addWidget(introtext);
    //  
    addWidget(new Wt::WBreak());
    //
    Wt::WImage * nextstepimage = new Wt::WImage("resources/other_images/footprints.png");
    nextstepimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    nextstepimage->setInline(true);
    Wt::WText * tns = new Wt::WText(" First Steps");
    tns->setInline(true);
    Wt::WContainerWidget * nextstepbanner = new Wt::WContainerWidget();
    nextstepbanner->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    nextstepbanner->decorationStyle().setBackgroundColor(nextstepcolor);
    nextstepbanner->addWidget(nextstepimage);
    nextstepbanner->addWidget(tns);
    addWidget(nextstepbanner);
    //
    std::string nextstep("/usr/local/textpresso/resources/resources-web/home.firststeps.txt");
    Wt::WContainerWidget * nextsteptext = new Wt::WContainerWidget();
    DisplayTextAndLinksFromFile(nextstep, nextstepcolor, nextsteptext);
    addWidget(nextsteptext);
    //
    addWidget(new Wt::WBreak());
    //
    Wt::WImage * contactimage = new Wt::WImage("resources/other_images/handshake.png");
    contactimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    contactimage->setInline(true);
    Wt::WText * tc = new Wt::WText(" Contact Us");
    tc->setInline(true);
    Wt::WContainerWidget * contactbanner = new Wt::WContainerWidget();
    contactbanner->decorationStyle().setForegroundColor(Wt::WColor(255, 255, 255));
    contactbanner->decorationStyle().setBackgroundColor(contactcolor);
    contactbanner->addWidget(contactimage);
    contactbanner->addWidget(tc);
    addWidget(contactbanner);
    //
    std::string contactus("/usr/local/textpresso/resources/resources-web/home.contactus.txt");
    Wt::WContainerWidget * contactustext = new Wt::WContainerWidget();
    DisplayTextAndLinksFromFile(contactus, contactcolor, contactustext);
    addWidget(contactustext);
    //
    addWidget(new Wt::WBreak());
    //
    Wt::WImage * tipimage = new Wt::WImage("resources/other_images/didyouknow.png");
    tipimage->resize(Wt::WLength(4, Wt::WLength::FontEx), Wt::WLength(4, Wt::WLength::FontEx));
    tipimage->setInline(true);
    Wt::WText * tt = new Wt::WText(" Tip of the Day");
    tt->setInline(true);
    Wt::WContainerWidget * tipbanner = new Wt::WContainerWidget();
    tipbanner->decorationStyle().setBackgroundColor(tipcolor);
    tipbanner->addWidget(tipimage);
    tipbanner->addWidget(tt);
    addWidget(tipbanner);
    //
    Wt::WContainerWidget * tipcont = new Wt::WContainerWidget();
    DisplayRandomItem(tipcont);
    Wt::WTimer * timer = new Wt::WTimer();
    timer->setInterval(7500);
    timer->start();
    timer->timeout().connect(boost::bind(&Help::DisplayRandomItem, this, tipcont));
    addWidget(tipcont);
}

Help::Help(const Help& orig) {
}

void Help::DisplayRandomItem(Wt::WContainerWidget * se) {
    se->clear();
    PgList * tips = new PgList(PGTIPOFDAY, PGTIPOFDAYTABLENAME);
    Wt::WText * tiptext = new Wt::WText(tips->GetRandomItem());
    delete tips;
    se->addWidget(tiptext);
}

void Help::DisplayTextAndLinksFromFile(std::string filename,
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
                } else {
                    Wt::WText * ttext = new Wt::WText(text);
                    ttext->decorationStyle().setForegroundColor(textcolor);
                    container->addWidget(ttext);
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

Help::~Help() {
}

