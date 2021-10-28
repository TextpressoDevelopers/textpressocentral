/* 
 * File:   Home.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 11:46 AM
 */

#include "Home.h"
#include "TCNavWeb.h"
#include "Session.h"

namespace {

    Wt::WContainerWidget * textFromFile(std::string filename, Wt::WContainerWidget * parent = NULL) {
        Wt::WContainerWidget * ret = new Wt::WContainerWidget(parent);
        std::ifstream f(filename.c_str());
        std::string in("");
        std::string txt("");
        while (getline(f, in)) txt += in;
        Wt::WText * text = new Wt::WText(txt);
        text->decorationStyle().font().setSize(Wt::WFont::XLarge);
        text->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        f.close();
        ret->addWidget(text);
        return ret;
    }

    Wt::WGroupBox * DisplayGroupBox(const Wt::WString & title, const Wt::WString & description) {
        Wt::WGroupBox * ret = new Wt::WGroupBox(title);
        ret->setContentAlignment(Wt::AlignJustify);
        Wt::WText * descriptiontext = new Wt::WText(description);
        descriptiontext->decorationStyle().font().setSize(Wt::WFont::Large);
        descriptiontext->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        ret->addWidget(descriptiontext);
        return ret;
    }

    Wt::WContainerWidget * Banner(Wt::WContainerWidget * parent = NULL) {
        Wt::WContainerWidget * ret = new Wt::WContainerWidget(parent);

        Wt::WImage * banner = new Wt::WImage("resources/other_images/textpresso_banner_aj.png");
        banner->setWidth(Wt::WLength(20, Wt::WLength::Percentage));

        //        Wt::WText * banner = new Wt::WText("Textpresso Central");
        //        banner->decorationStyle().setForegroundColor(Wt::WColor(70, 90, 180));
        //        banner->decorationStyle().font().setWeight(Wt::WFont::Bold);
        //        banner->decorationStyle().font().setSize(Wt::WFont::XXLarge);
        //        banner->decorationStyle().font().setStyle(Wt::WFont::Italic);
        //        banner->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
        //        banner->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);

        ret->addWidget(banner);
        //
        ret->addWidget(new Wt::WBreak());
        Wt::WText * subtitle = new Wt::WText("Knowledge discovery through "
                "full text mining, classification and searching");
        subtitle->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        ret->addWidget(subtitle);
        //
        return ret;
    }

    Wt::WContainerWidget * Info(Wt::WContainerWidget * parent = NULL) {

    }

    Wt::WContainerWidget * ExternalLinks(std::string filename, Wt::WContainerWidget * parent = NULL) {
        Wt::WContainerWidget * ret = new Wt::WContainerWidget();

        std::ifstream f(filename.c_str());
        if (f.is_open()) {
            Wt::WGroupBox * gb = new Wt::WGroupBox("External Links");
            gb->setContentAlignment(Wt::AlignJustify);
            std::string in("");
            while (getline(f, in)) {
                std::vector<std::string> info;
                boost::split(info, in, boost::is_any_of("\t"));
                if (info.size() > 1) {
                    Wt::WAnchor * site = new Wt::WAnchor(info[0]);
                    site->setTarget(Wt::TargetNewWindow);
                    site->setText(info[1]);
                    site->decorationStyle().font().setSize(Wt::WFont::Large);
                    site->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
                    int r(100), g(100), b(255);
                    if (info.size() > 2) r = std::stoi(info[2]);
                    if (info.size() > 3) g = std::stoi(info[3]);
                    if (info.size() > 4) b = std::stoi(info[4]);
                    site->decorationStyle().setForegroundColor(Wt::WColor(r, g, b));
                    gb->addWidget(site);
                    gb->addWidget(new Wt::WBreak());
                }
            }
            ret->addWidget(gb);
        }
        f.close();
        return ret;
    }

    Wt::WContainerWidget * Main(Wt::WContainerWidget * parent = NULL) {
        Wt::WContainerWidget * ret = new Wt::WContainerWidget(parent);
        //
        Wt::WGroupBox * sft(DisplayGroupBox("Search Full Text",
                "Textpresso Central contains "
                "full text articles that can be searched "
                "by keywords and categories. "
                "The number of articles is continuously growing."));
        ret->addWidget(sft);
        ret->addWidget(new Wt::WBreak());
        Wt::WGroupBox * frt(DisplayGroupBox("Find Relevant Facts",
                "Perform simple keyword searches or more advanced searches "
                "to answer specific biological questions. Search results "
                "are presented within the context of the full text for "
                "rapid assessment of relevancy."));
        ret->addWidget(frt);
        ret->addWidget(new Wt::WBreak());
        Wt::WGroupBox * cur(DisplayGroupBox("Curate",
                "Add annotations to papers and "
                "send them to any external curation database."));
        ret->addWidget(cur);
        //
        return ret;
    }
}

Home::Home(Session& session, Wt::WContainerWidget * parent) : parent_(parent), Wt::WContainerWidget(parent) {
    session_ = &session;
}

Wt::WContainerWidget * Home::SearchBox(Search * search) {
    Wt::WContainerWidget * ret = new Wt::WContainerWidget();
    if (search != NULL) {
        ret->decorationStyle().setBackgroundColor(Wt::WColor(220, 220, 255));
        ret->setContentAlignment(Wt::AlignMiddle);
        Wt::WText * searchlabel = new Wt::WText("Search corpus: ");
        searchlabel->decorationStyle().font().setSize(Wt::WFont::Large);
        searchlabel->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
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
        Wt::WAnchor * advsearch = new Wt::WAnchor(Wt::WLink(Wt::WLink::InternalPath, "/search"),
                "Advanced Search");
        advsearch->decorationStyle().font().setSize(Wt::WFont::Large);
        advsearch->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        ret->addWidget(searchlabel);
        ret->addWidget(keyword);
        ret->addWidget(searchicon);
        ret->addWidget(advsearch);
        Wt::WContainerWidget * litcont = new Wt::WContainerWidget();
        litcont->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
        search->SetLiteratureDescription(litcont);
        ret->addWidget(litcont);
        ret->setContentAlignment(Wt::AlignCenter);
    }
    return ret;
}

void Home::LoadContent(Search * search) {
    Wt::WBorderLayout * mainlayout = new Wt::WBorderLayout();
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
    //
    north->setContentAlignment(Wt::AlignCenter);
    north->addWidget(Banner());
    north->addWidget(new Wt::WBreak());
    north->addWidget(textFromFile("/data/textpresso/etc/subline.txt"));
    south->addWidget(SearchBox(search));
    //
    center->addWidget(Main());
    center->setPadding(Wt::WLength("1%"));
    //
    east->addWidget(ExternalLinks("/data/textpresso/etc/externallinks.txt"));
    east->setPadding(Wt::WLength("1%"));
    //
}

void Home::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}
