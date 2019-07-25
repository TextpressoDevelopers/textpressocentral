/* 
 * File:   AboutUs.cpp
 * Author: mueller
 * 
 * Created on April 26, 2016, 3:30 PM
 */

#include "AboutUs.h"
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WBreak>
#include <Wt/WFont>
#include <Wt/WCssDecorationStyle>

#define VERSION TPC_VERSION
#define RELEASE_NAME TPC_RELEASE_NAME

AboutUs::AboutUs() {

    setMaximumSize(Wt::WLength("100%"), Wt::WLength::Auto);
    setPadding(Wt::WLength(2, Wt::WLength::Percentage));
    std::string date(__DATE__);
    std::string year(date.substr(date.length() - 4, 4));
    Wt::WString copyright = Wt::WString::fromUTF8(
            "© " + year + " Textpresso, California Institute of Technology. Site build date: " + date + ", " +
            __TIME__ + ". Textpressocentral " + VERSION + " (" + RELEASE_NAME + ")");
    Wt::WText *ct = new Wt::WText(copyright);
    ct->decorationStyle().font().setSize(Wt::WFont::Large);
    ct->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    //
    Wt::WString logonotice("Banner and logo were designed by April Jauhal. All rights reserved.");
    Wt::WText *lnt = new Wt::WText(logonotice);
    lnt->decorationStyle().font().setSize(Wt::WFont::Large);
    lnt->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    //
    Wt::WString infringement("Any results and output obtained from the "
            "Textpresso server shall not be used for any purpose other than "
            "private study, scholarship or academic research. Anybody using "
            "Textpresso in excess of \"fair use\" may be liable for copyright "
            "infringement. "
            "All materials appearing on this server may not be reproduced "
            "or stored in a retrieval system without prior written permission "
            "of the publisher and in no case for profit. Documents from this "
            "server are provided \"AS-IS\" without any warranty, "
            "expressed or implied. ");
    Wt::WText *ift = new Wt::WText(infringement);
    ift->decorationStyle().font().setSize(Wt::WFont::Large);
    ift->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    //
    Wt::WString claim = Wt::WString::fromUTF8("Hans-Michael Müller and Paul "
            "W. Sternberg claim "
            "all rights in the word \"Textpresso\" as a trademark.");
    Wt::WText *clt = new Wt::WText(claim);
    clt->decorationStyle().font().setSize(Wt::WFont::Large);
    clt->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    //
    Wt::WString lntitle("L e g a l   N o t i c e s");
    Wt::WText *lnti = new Wt::WText(lntitle);
    lnti->decorationStyle().font().setSize(Wt::WFont::Large);
    lnti->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    lnti->decorationStyle().setTextDecoration(Wt::WCssDecorationStyle::Underline);
    //
    Wt::WString abstract("Manual, full-text literature curation underlies much of the "
            "annotation found in biological databases, but the process of finding "
            "relevant papers and specific supporting evidence for annotations can "
            "be very time-consuming.  To ameliorate the cost and tedium of manual "
            "curation, we have re-implemented, from scratch, the Textpresso text mining "
            "system to allow for direct annotation of full text and integration of that "
            "annotation into diverse curation systems.  The new system, "
            "Textpresso Central, preserves the key features of the original system "
            "by allowing for sophisticated queries using keywords and semantically "
            "related categories, but enhances the user experience by providing search "
            "results in the context of full text.  Resulting full text annotations can "
            "readily be integrated into any user-defined curation platform and also "
            "used to develop training sets to further improve text mining performance.  "
            "The current implementation of TextpressoCentral includes articles "
            "from the PMC Open Archive and uses the Unstructured Information Management "
            "Architecture (UIMA) framework as well as Lucene indexing. To maximize "
            "the utility of the system, Textpresso Central also allows for processing "
            "and displaying of results from third-party text mining and "
            "natural language processing algorithms.");
    Wt::WText * at = new Wt::WText(abstract);
    at->decorationStyle().font().setSize(Wt::WFont::Large);
    at->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    // 
    Wt::WString attitle("A b o u t   T e x t p r e s s o");
    Wt::WText *atti = new Wt::WText(attitle);
    atti->decorationStyle().font().setSize(Wt::WFont::Large);
    atti->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
    atti->decorationStyle().setTextDecoration(Wt::WCssDecorationStyle::Underline);
    //
    setContentAlignment(Wt::AlignJustify);
    addWidget(atti);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(at);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(lnti);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(ift);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(lnt);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(clt);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(ct);

}

AboutUs::AboutUs(const AboutUs &orig) {
}

AboutUs::~AboutUs() {
}

