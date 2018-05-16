/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   Download.cpp
 * Author: mueller
 * 
 * Created on May 14, 2018, 9:13 PM
 */

#include "Download.h"
#include <Wt/WString>
#include <Wt/WText>
#include <Wt/WCssDecorationStyle>
#include <Wt/WBreak>

Download::Download() {

    setContentAlignment(Wt::AlignLeft);
    setMaximumSize(Wt::WLength("90%"), Wt::WLength::Auto);

    Wt::WString dltext("The software  to run this site can be downloaded at our "
            "<a href=\"https://github.com/TextpressoDevelopers\" target=\"_blank\">GitHub site</a>. "
            "All licenses are available there.");
    Wt::WText *dt = new Wt::WText(dltext);
    dt->decorationStyle().font().setSize(Wt::WFont::Large);
    addWidget(dt);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    Wt::WString outsidelicenses("This site uses the following third-party libraries and software. "
            "Please follow the links to see the license and copyright notices.<br/>"
            "<br/><a href=\"https://www.webtoolkit.eu/wt\" target=\"_blank\">Wt. A C++ Web Toolkit</a>."
            "<br/><a href=\"https://www.lighttpd.net/\" target=\"_blank\">Lighttpd</a>."
            "<br/><a href=\"https://github.com/luceneplusplus/LucenePlusPlus\" target=\"_blank\">Lucene++</a>."
            "<br/><a href=\"http://uima.apache.org\" target=\"_blank\">Unstructured Information Management Architecture</a>."
            "<br/><a href=\"http://podofo.sourceforge.net/\" target=\"_blank\">PoDoFo</a>."
            "<br/><a href=\"http://pqxx.org/development/libpqxx/\" target=\"_blank\">libpqxx</a>."
            );
    Wt::WText *oli = new Wt::WText(outsidelicenses);
    oli->decorationStyle().font().setSize(Wt::WFont::Large);
    addWidget(oli);
    addWidget(new Wt::WBreak());
}

Download::Download(const Download& orig) {
}

Download::~Download() {
}

