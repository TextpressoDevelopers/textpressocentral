/* 
 * File:   Papers.cpp
 * Author: mueller
 * 
 * Created on April 4, 2013, 10:22 AM
 */

#include "Papers.h"
#include "UploadManager.h"
#include "PaperOrganizer.h"
#include "LiteratureProcessProgress.h"
#include <Wt/WCssDecorationStyle>
#include <Wt/WText>
#include <Wt/WMenuItem>
#include <Wt/WBreak>
#include <Wt/WTable>
#include <Wt/WAnchor>
#include <Wt/WImage>

Papers::Papers(Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    alreadyloaded_ = false;
}

void Papers::LoadContent(Session & session) {
    if (!alreadyloaded_) {
        session_ = &session;
        session_->login().changed().connect(boost::bind(&Papers::SessionLoginChanged, this));
        clear();
        statusline_ = new Wt::WText("", this);
        tabwidget_ = new Wt::WTabWidget(this);
        Wt::WContainerWidget * uploadcontainer = new Wt::WContainerWidget();
        UploadManager * ulm = new UploadManager(session);
        uploadcontainer->addWidget(ulm);
        //
        Wt::WContainerWidget * organizecontainer = new Wt::WContainerWidget();
        PaperOrganizer * poz = new PaperOrganizer(session);
        organizecontainer->addWidget(poz);
        //
        Wt::WContainerWidget * processcontainer = new Wt::WContainerWidget();
        LiteratureProcessProgress * lpp = new LiteratureProcessProgress(session);
        processcontainer->addWidget(lpp);
        //
        Wt::WContainerWidget * helpcontainer = new Wt::WContainerWidget();
        FillContainerWithHelpText(helpcontainer);
        //
        tabwidget_->addTab(uploadcontainer, "Upload");
        Wt::WMenuItem * organizemenuitem =
                tabwidget_->addTab(organizecontainer, "Organize");
        organizemenuitem->triggered().connect(std::bind([ = ] (){
            poz->PopulateUploadTable();
        }));
        Wt::WMenuItem * progressmenuitem =
                tabwidget_->addTab(processcontainer, "Progress");
        progressmenuitem->triggered().connect(std::bind([ = ] (){
            lpp->PopulateLiteratureTable();
        }));
        tabwidget_->addTab(helpcontainer, "Help");
        tabwidget_->setCurrentIndex(0);
        SessionLoginChanged();
        alreadyloaded_ = true;
    }
}

Papers::~Papers() {
}

void Papers::SessionLoginChanged() {
    if (session_->login().state() == 0) {
        statusline_->setText("You must be logged in to use this feature.");
        statusline_->decorationStyle().setBackgroundColor(Wt::yellow);
        tabwidget_->hide();
    } else {
        statusline_->setText("");
        statusline_->decorationStyle().setBackgroundColor(Wt::white);
        tabwidget_->show();
    }
}

void Papers::FillContainerWithHelpText(Wt::WContainerWidget * p) {
    //
    Wt::WText * t0 = new Wt::WText(
            "This introduction explains how to upload papers, organize them "
            "into literatures, and monitor the progress of processing. "
            "The tabs on the left of this tab are ordered according to the "
            "sequence with which papers are handled. "
            );
    t0->decorationStyle().font().setSize(Wt::WFont::Large);
    t0->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t1 = new Wt::WText(
            "Upload"
            );
    t1->decorationStyle().font().setSize(Wt::WFont::Large);
    t1->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t1->decorationStyle().font().setWeight(Wt::WFont::Bold);
    //
    Wt::WText * t11 = new Wt::WText(
            "Uploading of papers is initiated by hitting the 'Browse' button. "
            "A file browser opens in a new window and one or more papers can be "
            "selected. Once the selection is finished, the uploading "
            "process can be started by clicking on the 'Upload' button. "
            "Note that the total upload size per upload is "
            "limited, but uploading can be repeated."
            );
    t11->decorationStyle().font().setSize(Wt::WFont::Large);
    t11->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t12 = new Wt::WText(
            "Textpresso Central accepts 4 file types; they are identified by "
            "the system through their respective file extension: "
            );
    t12->decorationStyle().font().setSize(Wt::WFont::Large);
    t12->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WText * t13a = new Wt::WText(".pdf");
    t13a->decorationStyle().font().setSize(Wt::WFont::Large);
    t13a->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t13a->decorationStyle().font().setStyle(Wt::WFont::Italic);
    t13a->setInline(true);
    Wt::WText * t13b = new Wt::WText("A pdf file.");
    t13b->decorationStyle().font().setSize(Wt::WFont::Large);
    t13b->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t13b->setInline(true);
    //
    Wt::WText * t14a = new Wt::WText(".bib");
    t14a->decorationStyle().font().setSize(Wt::WFont::Large);
    t14a->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t14a->decorationStyle().font().setStyle(Wt::WFont::Italic);
    t14a->setInline(true);
    Wt::WText * t14b = new Wt::WText(
            "A bibliography file. As a pdf file usually does not contain "
            "bibliographical information in a format that can be easily "
            "extracted, each pdf file needs to be accompanied by a bib file. "
            "A template can be found "
            );
    t14b->decorationStyle().font().setSize(Wt::WFont::Large);
    t14b->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t14b->setInline(true);
    Wt::WAnchor * a14c = new Wt::WAnchor(Wt::WLink(Wt::WLink::Url, "http://tpc.textpresso.org/template.bib"), "here");
    a14c->setTarget(Wt::TargetNewWindow);
    a14c->decorationStyle().font().setSize(Wt::WFont::Large);
    a14c->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    a14c->setInline(true);
    Wt::WText * t14d = new Wt::WText(" (replace the '<not uploaded>' text "
            "in the file with the correct information). The filename of the bib file (without "
            "extension '.bib') must be the same as the filename of the pdf file "
            "(without extension '.pdf')."
            );
    t14d->decorationStyle().font().setSize(Wt::WFont::Large);
    t14d->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t14d->setInline(true);
    //
    Wt::WText * t15a = new Wt::WText(".tar.gz");
    t15a->decorationStyle().font().setSize(Wt::WFont::Large);
    t15a->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t15a->decorationStyle().font().setStyle(Wt::WFont::Italic);
    t15a->setInline(true);
    Wt::WText * t15b = new Wt::WText("A tarzipped file from the ");
    t15b->decorationStyle().font().setSize(Wt::WFont::Large);
    t15b->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t15b->setInline(true);
    Wt::WAnchor * a15c = new Wt::WAnchor(Wt::WLink(Wt::WLink::Url,
            "https://www.ncbi.nlm.nih.gov/pmc/tools/openftlist/"),
            "PMC Open Access Subset");
    a15c->setTarget(Wt::TargetNewWindow);
    a15c->decorationStyle().font().setSize(Wt::WFont::Large);
    a15c->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    a15c->setInline(true);
    Wt::WText * t15d = new Wt::WText(". This archive file must contain an "
            "NXML file and image files belonging to it."
            );
    t15d->decorationStyle().font().setSize(Wt::WFont::Large);
    t15d->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t15d->setInline(true);
    //
    Wt::WText * t16a = new Wt::WText(".nxml");
    t16a->decorationStyle().font().setSize(Wt::WFont::Large);
    t16a->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t16a->decorationStyle().font().setStyle(Wt::WFont::Italic);
    t16a->setInline(true);
    Wt::WText * t16b = new Wt::WText("An NXML file from the ");
    t16b->decorationStyle().font().setSize(Wt::WFont::Large);
    t16b->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t16b->setInline(true);
    Wt::WAnchor * a16c = new Wt::WAnchor(Wt::WLink(Wt::WLink::Url,
            "https://www.ncbi.nlm.nih.gov/pmc/tools/openftlist/"),
            "PMC Open Access Subset");
    a16c->setTarget(Wt::TargetNewWindow);
    a16c->decorationStyle().font().setSize(Wt::WFont::Large);
    a16c->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    a16c->setInline(true);
    Wt::WText * t16d = new Wt::WText(". In this case there are no image files, "
            "so images will not be displayed when the paper is shown "
            "in the 'Curation' tab above."
            );
    t16d->decorationStyle().font().setSize(Wt::WFont::Large);
    t16d->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t16d->setInline(true);
    //
    Wt::WText * t2 = new Wt::WText(
            "Organize"
            );
    t2->decorationStyle().font().setSize(Wt::WFont::Large);
    t2->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t2->decorationStyle().font().setWeight(Wt::WFont::Bold);
    //
    Wt::WText * t21 = new Wt::WText(
            "Once papers are uploaded, they need to be organized into "
            "literatures. There has to be at least one literature for the "
            "processing pipeline to begin. It makes sense to organize them "
            "according to topics. Names of literatures can be entered by "
            "clicking on the plus sign next to the drop down menu:"
            );
    t21->decorationStyle().font().setSize(Wt::WFont::Large);
    t21->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i22 = new Wt::WImage("resources/other_images/currentliterature1.png");
    //
    Wt::WText * t23 = new Wt::WText(
            "A line edit box appears where a name can be typed in. It is "
            "accepted when 'Enter' is hit and appears in the drop-down menu: "
            );
    t23->decorationStyle().font().setSize(Wt::WFont::Large);
    t23->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i24 = new Wt::WImage("resources/other_images/currentliterature2.png");
    //
    Wt::WText * t25 = new Wt::WText(
            "To assign papers to a particular literature, click on one or more "
            "rows of the paper table (selecting multiple papers with the shift, "
            "ctrl, or any other key assigned by your browser should work). "
            "Then choose from the drop-down menu to which literature the selected "
            "papers should be assigned to. Finally, click on the green "
            "'Add selections to current literature!' button. The column "
            "'assigned literature(s)' in the paper table should change and "
            "confirm your selection. You can use the yellow "
            "'Delete selections from current literatures!' to remove erroneous "
            "entries. The red 'Delete selections from library!' "
            "button completely removes selected, uploaded files from the "
            "system. The minus sign next to the plus sign removes literature "
            "names from the drop-down menu, and if papers are already assigned "
            "the literature that is to be removed, the assignment will be deleted "
            "too. After all assignments are done, the 'Organize' page should "
            "look something like this:"
            );
    t25->decorationStyle().font().setSize(Wt::WFont::Large);
    t25->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    Wt::WImage * i26 = new Wt::WImage("resources/other_images/currentliterature3.png");
    i26->setWidth(Wt::WLength("75%"));
    //
    Wt::WText * t3 = new Wt::WText(
            "Progress"
            );
    t3->decorationStyle().font().setSize(Wt::WFont::Large);
    t3->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    t3->decorationStyle().font().setWeight(Wt::WFont::Bold);
    //
    Wt::WText * t31 = new Wt::WText(
            "A cronjob checks literatures of all users once every 15 minutes "
            "and tokenizes them, performs mark-ups, produces indices and "
            "puts processed literatures online. The 'Progress' page reports the status of the processes. "
            "When indexed, the literature is ready for searching, and papers are ready "
            "for viewing. If they cannot be found in the 'Pick Literature' "
            "popup window of the 'Search' page, the whole webpage should be "
            "restarted by hitting the 'Reload' button of the web browser."
            );
    t31->decorationStyle().font().setSize(Wt::WFont::Large);
    t31->decorationStyle().font().setFamily(Wt::WFont::SansSerif);
    //
    //
    p->addWidget(t0);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t1);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t11);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t12);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    Wt::WTable * fttable = new Wt::WTable();
    fttable->addStyleClass("table form-inline");
    fttable->addStyleClass("table-bordered");
    fttable->elementAt(0, 0)->addWidget(t13a);
    fttable->elementAt(0, 1)->addWidget(t13b);
    fttable->elementAt(1, 0)->addWidget(t14a);
    fttable->elementAt(1, 1)->addWidget(t14b);
    fttable->elementAt(1, 1)->addWidget(a14c);
    fttable->elementAt(1, 1)->addWidget(t14d);
    fttable->elementAt(2, 0)->addWidget(t15a);
    fttable->elementAt(2, 1)->addWidget(t15b);
    fttable->elementAt(2, 1)->addWidget(a15c);
    fttable->elementAt(2, 1)->addWidget(t15d);
    fttable->elementAt(3, 0)->addWidget(t16a);
    fttable->elementAt(3, 1)->addWidget(t16b);
    fttable->elementAt(3, 1)->addWidget(a16c);
    fttable->elementAt(3, 1)->addWidget(t16d);
    p->addWidget(fttable);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t2);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t21);
    p->addWidget(new Wt::WBreak());
    p->addWidget(i22);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t23);
    p->addWidget(new Wt::WBreak());
    p->addWidget(i24);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t25);
    p->addWidget(new Wt::WBreak());
    p->addWidget(i26);
    p->addWidget(new Wt::WBreak());
    p->addWidget(t3);
    p->addWidget(new Wt::WBreak());
    p->addWidget(new Wt::WBreak());
    p->addWidget(t31);
    p->addWidget(new Wt::WBreak());
}