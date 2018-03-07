/* 
 * File:   AboutUs.cpp
 * Author: mueller
 * 
 * Created on April 26, 2016, 3:30 PM
 */

#include "AboutUs.h"
#include <Wt/WText>
#include <Wt/WBreak>
#include <Wt/WFont>
#include <Wt/WCssDecorationStyle>

#define VERSION TPC_VERSION
#define RELEASE_NAME TPC_RELEASE_NAME

AboutUs::AboutUs() {

    setContentAlignment(Wt::AlignLeft);
    setMaximumSize(Wt::WLength("90%"), Wt::WLength::Auto);
    std::string date(__DATE__);
    std::string year(date.substr(date.length() - 4, 4));
    Wt::WString copyright = Wt::WString::fromUTF8(
            "© " + year + " Textpresso, California Institute of Technology. Site build date: " + date + ", " +
            __TIME__ + ". Textpressocentral " + VERSION + " (" + RELEASE_NAME + ")");
    Wt::WText *ct = new Wt::WText(copyright);
    ct->decorationStyle().font().setSize(Wt::WFont::Large);
    ct->decorationStyle().font().setVariant(Wt::WFont::SmallCaps);
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
    Wt::WString licenses("This sites uses the following third-party libraries and software. "
                                 "Please follow the links to see the license and copyright notices.<br/>"
                                 "<br/><a href=\"https://www.webtoolkit.eu/wt\" target=\"_blank\">Wt. A C++ Web Toolkit</a>."
                                 "<br/><a href=\"https://github.com/luceneplusplus/LucenePlusPlus\" target=\"_blank\">Lucene++</a>."
                                 "<br/><a href=\"http://uima.apache.org\" target=\"_blank\">Unstructured Information Management Architecture</a>."
                                 "<br/><a href=\"http://podofo.sourceforge.net/\" target=\"_blank\">PoDoFo</a>."
                                 "<br/><a href=\"http://pqxx.org/development/libpqxx/\" target=\"_blank\">libpqxx</a>."
    );
    Wt::WText *at = new Wt::WText(abstract);
    at->decorationStyle().font().setSize(Wt::WFont::Large);
    addWidget(at);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    Wt::WText *li = new Wt::WText(licenses);
    li->decorationStyle().font().setSize(Wt::WFont::Large);
    addWidget(li);
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(new Wt::WBreak());
    addWidget(ct);

}

AboutUs::AboutUs(const AboutUs &orig) {
}

AboutUs::~AboutUs() {
}

