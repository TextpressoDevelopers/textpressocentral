/* 
 * File:   ConfigurableAnnotatorContainer.cpp
 * Author: mueller
 * 
 * Created on April 2, 2015, 12:44 PM
 */

#include "TextpressoCentralGlobalDefinitions.h"
#include "ConfigurableAnnotatorContainer.h"
#include "CurationFormsConfiguration.h"
#include "PgCurationDataFromViewer.h"
#include "PgPrepopulation.h"
#include "Viewer.h"
#include "SuggestionBoxFromPgOntology.h"
#include "SuggestionBoxFromPgOrWeb.h"
#include "PrePopulationDialog.h"
#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Serializer>
#include <Wt/Http/Message>
#include <Wt/WApplication>
#include <Wt/WMessageBox>
#include <Wt/WGridLayout>
#include <Wt/WLineEdit>
#include <Wt/WTextArea>
#include <Wt/WVBoxLayout>

namespace {

    long unsigned int FindIndexFromPointer(Wt::WText * x,
            std::vector<Wt::WText*> & textvec) {
        for (long unsigned int i = 0; i < textvec.size(); i++)
            if (textvec[i] == x) return i;
    }

    void SetText(Wt::WText * t, Wt::WText * l, Wt::WContainerWidget * p) {
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        p->addWidget(l);
        t->decorationStyle().setBackgroundColor(Wt::white);
        p->addWidget(t);
    }

    Wt::WLineEdit * SetLineEdit(Wt::WText * l, Wt::WImage * i, Wt::WContainerWidget * p) {
        Wt::WLineEdit * ret = new Wt::WLineEdit();
        ret->setWidth(Wt::WLength(50, Wt::WLength::FontEx));
        Wt::WImage * icancel = new Wt::WImage("resources/icons/cancel.png");
        icancel->mouseWentOver().connect(std::bind([ = ] (){
            icancel->decorationStyle().setCursor(Wt::PointingHandCursor);
        }));
        icancel->clicked().connect(std::bind([ = ] (){
            ret->setText("");
        }));
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        l->setInline(true);
        p->addWidget(l);
        i->setInline(true);
        p->addWidget(i);
        icancel->setInline(true);
        p->addWidget(icancel);
        p->addWidget(new Wt::WBreak());
        p->addWidget(ret);
        return ret;
    }

    Wt::WTextArea * SetTextArea(Wt::WText * l, Wt::WImage * i, Wt::WContainerWidget * p) {
        Wt::WTextArea * ret = new Wt::WTextArea();
        Wt::WImage * icancel = new Wt::WImage("resources/icons/cancel.png");
        icancel->mouseWentOver().connect(std::bind([ = ] (){
            icancel->decorationStyle().setCursor(Wt::PointingHandCursor);
        }));
        icancel->clicked().connect(std::bind([ = ] (){
            ret->setText("");
        }));
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        l->setInline(true);
        p->addWidget(l);
        i->setInline(true);
        p->addWidget(i);
        icancel->setInline(true);
        p->addWidget(icancel);
        p->addWidget(new Wt::WBreak());
        ret->setWidth(Wt::WLength(47, Wt::WLength::FontEx));
        ret->setHeight(Wt::WLength(10, Wt::WLength::FontEx));
        p->addWidget(ret);
        return ret;
    }

    Wt::WCheckBox * SetCheckBox(Wt::WString s, bool b, Wt::WContainerWidget * p) {
        Wt::WCheckBox * ret = new Wt::WCheckBox(s);
        ret->setChecked(b);
        p->addWidget(new Wt::WBreak());
        p->addWidget(ret);
        return ret;
    }

    Wt::WComboBox * SetComboBox(Wt::WText * l, std::string choices, Wt::WContainerWidget * p) {
        Wt::WComboBox * ret = new Wt::WComboBox();
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        p->addWidget(l);
        p->addWidget(new Wt::WBreak());
        std::vector<std::string> splits;
        boost::split(splits, choices, boost::is_any_of("|"));
        for (int i = 0; i < splits.size(); i++)
            ret->addItem(Wt::WString(splits[i]));
        p->addWidget(ret);
        return ret;
    }

    Wt::WTextArea * SetListMaker(Wt::WText * l, Wt::WImage * i, Wt::WLineEdit * le, Wt::WContainerWidget * p) {
        Wt::WTextArea * ret = new Wt::WTextArea();
        Wt::WImage * icancel = new Wt::WImage("resources/icons/cancel.png");
        icancel->mouseWentOver().connect(std::bind([ = ] (){
            icancel->decorationStyle().setCursor(Wt::PointingHandCursor);
        }));
        icancel->clicked().connect(std::bind([ = ] (){
            le->setText("");
            ret->setText("");
        }));
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        l->setInline(true);
        p->addWidget(l);
        i->setInline(true);
        p->addWidget(i);
        icancel->setInline(true);
        p->addWidget(icancel);
        p->addWidget(new Wt::WBreak());
        //
        Wt::WPushButton * add = new Wt::WPushButton(">");
        add->setToolTip("Add to list");
        add->setStyleClass("btn-mini");
        add->setInline(true);
        //
        le->setInline(true);
        //
        ret->setWidth(Wt::WLength(47, Wt::WLength::FontEx));
        ret->setHeight(Wt::WLength(10, Wt::WLength::FontEx));
        ret->setInline(true);
        //
        add->clicked().connect(std::bind([ = ] (){
            Wt::WString aux(ret->text());
            aux += (ret->text().empty()) ? le->text() : "\n" + le->text();
            ret->setText(aux);
            le->setText("");
        }));
        le->enterPressed().connect(std::bind([ = ] (){
            Wt::WString aux(ret->text());
            aux += (ret->text().empty()) ? le->text() : "\n" + le->text();
            ret->setText(aux);
            le->setText("");
        }));
        //
        p->setContentAlignment(Wt::AlignMiddle);
        p->addWidget(le);
        Wt::WText * space1 = new Wt::WText(" ");
        space1->setInline(true);
        p->addWidget(space1);
        p->addWidget(add);
        Wt::WText * space2 = new Wt::WText(" ");
        space2->setInline(true);
        p->addWidget(space2);
        p->addWidget(ret);
        return ret;
    }

    std::string CleanWhiteSpaces(std::string inp) {
        std::string out = boost::replace_all_copy(inp, "\r", " ");
        boost::replace_all(out, "\n", " ");
        boost::replace_all(out, "\t", " ");
        while (out.find("  ") != std::string::npos) boost::replace_all(out, "  ", " ");
        return out;
    }

    void ExtractTermsFromAdditionalAnnotations(std::string annotations,
            std::multimap<std::string, std::string> & map) {
        int cursor = 0;
        while (cursor != std::string::npos) {
            cursor = annotations.find_first_of("[", cursor);
            int endcursor = annotations.find_first_of("]", cursor);
            if (cursor >= 0) {
                std::string aux = annotations.substr(cursor + 1, endcursor - cursor - 1);
                std::vector<std::string> splits;
                boost::split(splits, aux, boost::is_any_of("|"));
                map.insert(std::make_pair(splits[3], splits[2]));
            }
            cursor = endcursor;
        }
    }

    std::string urlencode(const std::string &s) {
        //RFC 3986 section 2.3 Unreserved Characters (January 2005)
        //const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
        //modify this so witty will not complain
        const std::string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~:/?=&";

        std::string escaped = "";
        for (size_t i = 0; i < s.length(); i++) {
            if (unreserved.find_first_of(s[i]) != std::string::npos) {
                escaped.push_back(s[i]);
            } else {
                escaped.append("%");
                char buf[3];
                sprintf(buf, "%.2X", (unsigned char) s[i]);
                escaped.append(buf);
            }
        }
        return escaped;
    }
}

ConfigurableAnnotatorContainer::ConfigurableAnnotatorContainer(UrlParameters * urlparams,
        std::string rootfn, Wt::WContainerWidget * parent) {
    urlparams_ = urlparams;
    moreurlparams_ = urlparams_->GetParmVector();
    parent_ = parent;
    rootfn_ = rootfn;
    formname_.clear();
    subcont_.viewerinfo = NULL;
    subcont_.fields = NULL;
    subcont_.buttons = NULL;
}

void ConfigurableAnnotatorContainer::CreateEmptyViewerInfoForm() {
    subcont_.viewerinfo->setWidth(Wt::WLength(80, Wt::WLength::FontEx));
    // data to be populated:
    // iid
    // term
    // paper_id (to be queried from postgres)
    // file_location
    // position_in_sofastring
    // owner
    // last_update
    // version


    //8//
    Wt::WText * plusoption = new Wt::WText("+ Paper Info");
    Wt::WText * minusoption = new Wt::WText("- Paper Info");
    minusoption->decorationStyle().font().setSize(Wt::WFont::Small);
    minusoption->mouseWentOver().connect(boost::bind(&ConfigurableAnnotatorContainer::SetCursorHand, this, minusoption));
    minusoption->decorationStyle().setBackgroundColor(Wt::WColor(255, 200, 200));
    plusoption->decorationStyle().font().setSize(Wt::WFont::Small);
    plusoption->mouseWentOver().connect(boost::bind(&ConfigurableAnnotatorContainer::SetCursorHand, this, plusoption));
    plusoption->decorationStyle().setBackgroundColor(Wt::WColor(200, 255, 200));
    subcont_.viewerinfo->addWidget(plusoption);
    subcont_.viewerinfo->addWidget(minusoption);
    subcont_.viewerinfo->addWidget(new Wt::WBreak());
    //
    Wt::WContainerWidget * clickablecontainer = new Wt::WContainerWidget();
    clickablecontainer->decorationStyle().font().setSize(Wt::WFont::Small);
    clickablecontainer->setContentAlignment(Wt::AlignMiddle);
    clickablecontainer->setInline(true);
    subcont_.viewerinfo->addWidget(clickablecontainer);
    //
    plusoption->clicked().connect(std::bind([ = ] (){
        plusoption->hide();
        minusoption->show();
        clickablecontainer->show();
        Wt::WBorder bx;
        bx.setStyle(Wt::WBorder::Solid);
        bx.setColor(Wt::black);
        bx.setWidth(Wt::WBorder::Thin);
        subcont_.viewerinfo->decorationStyle().setBorder(bx);
        subcont_.viewerinfo->decorationStyle().setBackgroundColor(Wt::WColor(250, 250, 250));
    }));
    minusoption->clicked().connect(std::bind([ = ] (){
        plusoption->show();
        minusoption->hide();
        clickablecontainer->hide();
        Wt::WBorder bx;
        subcont_.viewerinfo->decorationStyle().setBorder(bx);
        subcont_.viewerinfo->decorationStyle().setBackgroundColor(Wt::WColor(250, 250, 250));
    }));
    plusoption->show();
    minusoption->hide();
    clickablecontainer->hide();
    //8//


    texts_.term = new Wt::WText();
    texts_.term->setWordWrap(true);
    SetText(texts_.term, new Wt::WText("Terms: "), clickablecontainer);
    clickablecontainer->addWidget(new Wt::WBreak());
    //
    texts_.bestring = new Wt::WText();
    SetText(texts_.bestring, new Wt::WText("Positions: "), clickablecontainer);
    clickablecontainer->addWidget(new Wt::WBreak());
    //
    texts_.additionalannotations = new Wt::WText();
    SetText(texts_.additionalannotations, new Wt::WText("Add. Annotations: "), clickablecontainer);
    clickablecontainer->addWidget(new Wt::WBreak());
    //
    texts_.filename = new Wt::WText();
    SetText(texts_.filename, new Wt::WText("Filename: "), clickablecontainer);
    //
    texts_.paperid = new Wt::WText();
    SetText(texts_.paperid, new Wt::WText(" Paper Id: "), clickablecontainer);
    //
    texts_.owner = new Wt::WText();
    SetText(texts_.owner, new Wt::WText(" Annotator: "), clickablecontainer);
    //
    texts_.lastupdate = new Wt::WText();
    SetText(texts_.lastupdate, new Wt::WText(" Created: "), clickablecontainer);
    //
    texts_.version = new Wt::WText();
    SetText(texts_.version, new Wt::WText(" Version: "), clickablecontainer);
    clickablecontainer->addWidget(new Wt::WBreak());
}

Wt::WString ConfigurableAnnotatorContainer::GetTextFromTerms(Wt::WText * strtptr,
        Wt::WText * endptr, std::vector<Wt::WText*> & terms) {
    Wt::WString ret;
    long unsigned int startind = FindIndexFromPointer(strtptr, terms);
    long unsigned int endind = FindIndexFromPointer(endptr, terms);
    //
    for (int j = startind; j <= endind; j++)
        ret += terms[j]->text();
    return ret;
}

Wt::WString ConfigurableAnnotatorContainer::GetCatFromTerms(Wt::WText * strtptr,
        Wt::WText * endptr, std::vector<Wt::WText*> & terms, std::multimap<Wt::WText*, Wt::WString*> & cats) {
    Wt::WString ret;
    long unsigned int startind = FindIndexFromPointer(strtptr, terms);
    long unsigned int endind = FindIndexFromPointer(endptr, terms);
    //
    for (int j = startind; j <= endind; j++) {
        std::pair<std::multimap<Wt::WText*, Wt::WString*>::iterator, std::multimap<Wt::WText*, Wt::WString*>::iterator> pbe;
        pbe = cats.equal_range(terms[j]);
        std::multimap<Wt::WText*, Wt::WString*>::iterator it;
        for (it = pbe.first; it != pbe.second; it++) {
            if (it != pbe.first) ret += Wt::WString(", ");
            ret += (*(*it).second);
        }

    }
    return ret;
}

std::string ConfigurableAnnotatorContainer::GetBEFromViewerInfo(Wt::WText * strtptr, Wt::WText * endptr, viewerinfo inp) {
    long unsigned int startind = FindIndexFromPointer(strtptr, inp.terms);
    long unsigned int endind = FindIndexFromPointer(endptr, inp.terms);
    int32_t imin = 999999999;
    if (inp.pointer2B[inp.terms[startind]] > -1)
        imin = inp.pointer2B[inp.terms[startind]];
    int32_t imax = inp.pointer2E[inp.terms[startind]];
    for (int j = startind; j <= endind; j++) {
        if (inp.pointer2B[inp.terms[j]] > -1)
            imin = (imin < inp.pointer2B[inp.terms[j]]) ? imin : inp.pointer2B[inp.terms[j]];
        imax = (imax > inp.pointer2E[inp.terms[j]]) ? imax : inp.pointer2E[inp.terms[j]];
    }
    std::stringstream retstream;
    retstream << "(" << imin << "," << imax << ")";
    return retstream.str();
}

void ConfigurableAnnotatorContainer::PopulateFieldsContent(std::string s) {
    formname_ = s;
    CreateFieldsContent();
}

void ConfigurableAnnotatorContainer::PopulateViewerInfoContent(viewerinfo inp) {
    //
    // viewer info data items
    // iid
    // term
    // paper_id (to be queried from postgres)
    // file_location
    // position_in_sofastring
    // owner
    // last_update
    // version
    //
    curationdata_.term = "";
    curationdata_.bestring = "";
    long unsigned int noofBsniplets = inp.curB.size();
    long unsigned int noofEsniplets = inp.curE.size();
    for (long unsigned int i = 0; i < noofBsniplets; i++) {
        //
        Wt::WText * strtptr = inp.curB[i];
        Wt::WText * endptr = (i < noofEsniplets) ? inp.curE[i] : inp.curB[i];
        curationdata_.term += GetTextFromTerms(strtptr, endptr, inp.terms);
        curationdata_.bestring += GetBEFromViewerInfo(strtptr, endptr, inp);
        if (i < noofBsniplets - 1) {
            curationdata_.bestring += " ";
            curationdata_.term += "|";
        }
    }
    curationdata_.additionalannotations = inp.additionalAnnotations;
    std::cout << curationdata_.additionalannotations << std::endl;
    curationdata_.filename = inp.filename;
    //get rid of root dir name
    boost::replace_all(curationdata_.filename, rootfn_, "");
    curationdata_.owner = inp.owner;
    curationdata_.paperid = inp.paperid;
    curationdata_.lastupdate = time(0);
    curationdata_.version = "1.0";
    //
    texts_.term->setText(curationdata_.term);
    texts_.bestring->setText(curationdata_.bestring);
    texts_.additionalannotations->setText(curationdata_.additionalannotations);
    texts_.filename->setText(curationdata_.filename);
    texts_.paperid->setText(curationdata_.paperid);
    texts_.owner->setText(curationdata_.owner);
    char buff[20];
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&curationdata_.lastupdate));
    texts_.lastupdate->setText(buff);
    texts_.version->setText(curationdata_.version);
}

void ConfigurableAnnotatorContainer::CreateAllEmpty() {
    decorationStyle().setBackgroundColor(Wt::WColor(250, 250, 250));
    clicknfilltext_ = new Wt::WCheckBox("Click'n Fill (text)");
    addWidget(clicknfilltext_);
    clicknfillcat_ = new Wt::WCheckBox("Click'n Fill (annotations)");
    addWidget(clicknfillcat_);
    includepaperinfo_ = new Wt::WCheckBox("Include Paper Info in Data Submission");
    includepaperinfo_->setChecked();
    addWidget(includepaperinfo_);
    includetpdbid_ = new Wt::WCheckBox("Include Textpresso Database Entry ID in Data Submission");
    addWidget(includetpdbid_);
    cont4subs_ = new Wt::WContainerWidget();
    if (!urlparams_->IsEmpty()) {
        Wt::WText * moreparameters = new Wt::WText("Configure return of submission parameters");
        moreparameters->mouseWentOver().connect(boost::bind(&ConfigurableAnnotatorContainer::SetCursorHand, this, moreparameters));
        moreparameters->decorationStyle().setBorder(Wt::WBorder(Wt::WBorder::Solid, Wt::WBorder::Thin, Wt::gray));
        moreparameters->setPadding(Wt::WLength(2, Wt::WLength::Pixel));
        moreparameters->setFloatSide(Wt::Right);
        moreparameters->decorationStyle().setBackgroundColor(Wt::WColor(242, 242, 242));
        moreparameters->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::MoreParametersClicked, this));
        addWidget(moreparameters);
    }
    //
    addWidget(cont4subs_);
    //
    subcont_.viewerinfo = new Wt::WContainerWidget();
    CreateEmptyViewerInfoForm();
    subcont_.fields = new Wt::WContainerWidget();
    CreateFieldsContent();
    subcont_.buttons = new Wt::WContainerWidget();
    CreateButtonsContent();
    Wt::WVBoxLayout * vbox = new Wt::WVBoxLayout();
    cont4subs_->setLayout(vbox);
    vbox->addWidget(subcont_.viewerinfo);
    vbox->addWidget(subcont_.fields);
    vbox->addWidget(subcont_.buttons);
}

void ConfigurableAnnotatorContainer::CreateViewerInfoContent(viewerinfo inp) {
    CreateEmptyViewerInfoForm();
    PopulateViewerInfoContent(inp);
}

void ConfigurableAnnotatorContainer::CreateFieldsContent() {
    // Using Pg to read field configuration and then displays them.
    subcont_.fields->clear();
    if (!formname_.empty()) {
        Wt::WGridLayout * gridbox = new Wt::WGridLayout();
        CurationFieldConfiguration * cfconf
                = new CurationFieldConfiguration(PGCURATIONFIELDS,
                PGCURATIONFIELDSTABLENAME, formname_);
        fielddata_.clear();
        maxx_ = 0;
        maxy_ = 0;
        for (int i = 0; i < cfconf->NumberOfFields(); i++) {
            Wt::WContainerWidget * fieldcontainer = new Wt::WContainerWidget();
            fielddata fd;
            // populate field data
            fd.label = cfconf->GetFieldName(i);
            fd.fieldtype = cfconf->GetFieldType(i);
            if (cfconf->GetFieldType(i) == CurationFieldConfiguration::lineedit) {
                //
                int ppid = cfconf->GetPrePopId(i);
                Wt::WString infotext;
                infotext = "The suggestions are retrieved from a database. Very common words ";
                infotext += "such as 'protein' would trigger 10000s of matches, so the suggestion ";
                infotext += "list is cut off at 750 entries. You can still retrieve hidden ";
                infotext += "suggestions by typing more specific terms. The list of suggestions ";
                infotext += "is continuously updated as you type.";
                Wt::WImage * i1 = new Wt::WImage("resources/icons/information16.png");
                i1->mouseWentOver().connect(boost::bind(&ConfigurableAnnotatorContainer::SetCursorHand, this, i1));
                i1->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::InfoIconClicked, this, infotext));
                //
                std::string addinfo = (ppid >= 0) ? " (prepop. field) " : " ";
                Wt::WString s(cfconf->GetFieldName(i) + addinfo);
                Wt::WLineEdit * pGenericLineEdit =
                        SetLineEdit(new Wt::WText(s), i1, fieldcontainer);
                pGenericLineEdit->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditClicked,
                        this, pGenericLineEdit));
                int x = cfconf->GetX(i);
                int y = cfconf->GetY(i);
                maxx_ = (x > maxx_) ? x : maxx_;
                maxy_ = (y > maxy_) ? y : maxy_;
                gridbox->addWidget(fieldcontainer, x, y);
                // add autocomplete
                if (cfconf->HasSuggestionBoxInfo(i)) {
                    CurationFieldConfiguration::pginfo sgbinfo(cfconf->GetSuggestionBoxInfo(i));
                    if ((sgbinfo.tablename.compare(PGONTOLOGYTABLENAME) == 0)
                            && (sgbinfo.database.compare("www-data") == 0)) {
                        SuggestionBoxFromPgOntology * sbpo = new SuggestionBoxFromPgOntology(
                                sgbinfo.col, sgbinfo.whereclause,
                                sgbinfo.tablename, sgbinfo.database);
                        sbpo->forEdit(pGenericLineEdit);
                    } else {
                        SuggestionBoxFromPgOrWeb * sbpo = new SuggestionBoxFromPgOrWeb(
                                sgbinfo.col, sgbinfo.whereclause,
                                sgbinfo.tablename, sgbinfo.database);
                        sbpo->forEdit(pGenericLineEdit);
                    }
                }
                // add customized validation, only for line edit because 
                // otherwise it's getting too cumbersome for curation.
                if (cfconf->HasValidationInfo(i)) {
                    CurationFieldConfiguration::pginfo valinfo(cfconf->GetValidationInfo(i));
                    FetchList * feli = new FetchList(valinfo.database, valinfo.tablename,
                            valinfo.col, valinfo.whereclause);
                    pGenericLineEdit->changed().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditChanged, this,
                            pGenericLineEdit, feli));
                    // for newer version of wt library, use textInput()
                    pGenericLineEdit->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditChanged, this,
                            pGenericLineEdit, feli));
                    pGenericLineEdit->keyPressed().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditChanged, this,
                            pGenericLineEdit, feli));

                }
                // prepopulating lineedit
                std::string poptext = GeneratePrepopulationText(ppid);
                pGenericLineEdit->setText(Wt::WString(poptext));
                // populate field data, continued
                fd.le = pGenericLineEdit;
                //
            } else if (cfconf->GetFieldType(i) == CurationFieldConfiguration::textarea) {
                //
                int ppid = cfconf->GetPrePopId(i);
                Wt::WString infotext;
                infotext = "Autocomplete is triggered whenever the cursor is at the beginning ";
                infotext += "of a line. Hit 'Enter' to get to a new line. ";
                infotext += "The suggestions are retrieved from a database. Very common words ";
                infotext += "such as 'protein' would trigger 10000s of matches, so the suggestion ";
                infotext += "list is cut off at 750 entries. You can still retrieve hidden ";
                infotext += "suggestions by typing more specific terms. The list of suggestions ";
                infotext += "is continuously updated as you type.";
                Wt::WImage * i1 = new Wt::WImage("resources/icons/information16.png");
                i1->mouseWentOver().connect(boost::bind(&ConfigurableAnnotatorContainer::SetCursorHand, this, i1));
                i1->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::InfoIconClicked, this, infotext));
                //
                std::string addinfo = (ppid >= 0) ? " (prepop. field) " : " ";
                Wt::WString s(cfconf->GetFieldName(i) + addinfo);
                Wt::WTextArea * pGenericTextArea =
                        SetTextArea(new Wt::WText(s), i1, fieldcontainer);
                pGenericTextArea->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::TextAreaClicked,
                        this, pGenericTextArea));
                int x = cfconf->GetX(i);
                int y = cfconf->GetY(i);
                maxx_ = (x > maxx_) ? x : maxx_;
                maxy_ = (y > maxy_) ? y : maxy_;
                gridbox->addWidget(fieldcontainer, x, y);
                // add autocomplete
                if (cfconf->HasSuggestionBoxInfo(i)) {
                    CurationFieldConfiguration::pginfo sgbinfo(cfconf->GetSuggestionBoxInfo(i));
                    if ((sgbinfo.tablename.compare(PGONTOLOGYTABLENAME) == 0)
                            && (sgbinfo.database.compare("www-data") == 0)) {
                        SuggestionBoxFromPgOntology * sbpo = new SuggestionBoxFromPgOntology(
                                sgbinfo.col, sgbinfo.whereclause,
                                sgbinfo.tablename, sgbinfo.database);
                        sbpo->forEdit(pGenericTextArea);
                    } else {
                        SuggestionBoxFromPgOrWeb * sbpo = new SuggestionBoxFromPgOrWeb(
                                sgbinfo.col, sgbinfo.whereclause,
                                sgbinfo.tablename, sgbinfo.database);
                        sbpo->forEdit(pGenericTextArea);
                    }
                }
                // there is no validation for text area
                // prepopulating textarea
                std::string poptext = GeneratePrepopulationText(ppid);
                pGenericTextArea->setText(Wt::WString(poptext));
                // populate field data, continued
                fd.ta = pGenericTextArea;
            } else if (cfconf->GetFieldType(i) == CurationFieldConfiguration::listmaker) {
                //
                Wt::WString infotext;
                infotext = "Autocomplete is triggered whenever the cursor is at the beginning ";
                infotext += "of a line. Hit 'Enter' to get to a new line. ";
                infotext += "The suggestions are retrieved from a database. Very common words ";
                infotext += "such as 'protein' would trigger 10000s of matches, so the suggestion ";
                infotext += "list is cut off at 750 entries. You can still retrieve hidden ";
                infotext += "suggestions by typing more specific terms. The list of suggestions ";
                infotext += "is continuously updated as you type.";
                Wt::WImage * i1 = new Wt::WImage("resources/icons/information16.png");
                i1->mouseWentOver().connect(boost::bind(&ConfigurableAnnotatorContainer::SetCursorHand, this, i1));
                i1->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::InfoIconClicked, this, infotext));
                //
                Wt::WString s(cfconf->GetFieldName(i) + " ");
                Wt::WLineEdit * pListMakerLineEdit = new Wt::WLineEdit();
                Wt::WTextArea * pGenericTextArea =
                        SetListMaker(new Wt::WText(s), i1, pListMakerLineEdit, fieldcontainer);
                pListMakerLineEdit->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditClicked,
                        this, pListMakerLineEdit));
                int x = cfconf->GetX(i);
                int y = cfconf->GetY(i);
                maxx_ = (x > maxx_) ? x : maxx_;
                maxy_ = (y > maxy_) ? y : maxy_;
                gridbox->addWidget(fieldcontainer, x, y);
                // add autocomplete
                if (cfconf->HasSuggestionBoxInfo(i)) {
                    CurationFieldConfiguration::pginfo sgbinfo(cfconf->GetSuggestionBoxInfo(i));
                    if ((sgbinfo.tablename.compare(PGONTOLOGYTABLENAME) == 0)
                            && (sgbinfo.database.compare("www-data") == 0)) {
                        SuggestionBoxFromPgOntology * sbpo = new SuggestionBoxFromPgOntology(
                                sgbinfo.col, sgbinfo.whereclause,
                                sgbinfo.tablename, sgbinfo.database);
                        sbpo->forEdit(pListMakerLineEdit);
                    } else {
                        SuggestionBoxFromPgOrWeb * sbpo = new SuggestionBoxFromPgOrWeb(
                                sgbinfo.col, sgbinfo.whereclause,
                                sgbinfo.tablename, sgbinfo.database);
                        sbpo->forEdit(pListMakerLineEdit);
                    }
                }
                // validation for pListMakerLineEdit
                if (cfconf->HasValidationInfo(i)) {
                    CurationFieldConfiguration::pginfo valinfo(cfconf->GetValidationInfo(i));
                    FetchList * feli = new FetchList(valinfo.database, valinfo.tablename,
                            valinfo.col, valinfo.whereclause);
                    pListMakerLineEdit->changed().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditChanged, this,
                            pListMakerLineEdit, feli));
                    // for newer version of wt library, use textInput()
                    pListMakerLineEdit->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditChanged, this,
                            pListMakerLineEdit, feli));
                    pListMakerLineEdit->keyPressed().connect(boost::bind(&ConfigurableAnnotatorContainer::LineEditChanged, this,
                            pListMakerLineEdit, feli));
                }
                // prepopulating textarea
                int ppid = cfconf->GetPrePopId(i);
                std::string poptext = GeneratePrepopulationText(ppid);
                pListMakerLineEdit->setText(Wt::WString(poptext));
                // populate field data, continued
                fd.ta = pGenericTextArea;
            } else if (cfconf->GetFieldType(i) == CurationFieldConfiguration::checkbox) {
                Wt::WString s(cfconf->GetFieldName(i));
                Wt::WCheckBox * pGenericCheckBox =
                        SetCheckBox(s, cfconf->GetCheckboxDefault(i), fieldcontainer);
                pGenericCheckBox->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::CheckBoxClicked,
                        this, pGenericCheckBox));
                int x = cfconf->GetX(i);
                int y = cfconf->GetY(i);
                maxx_ = (x > maxx_) ? x : maxx_;
                maxy_ = (y > maxy_) ? y : maxy_;
                gridbox->addWidget(fieldcontainer, x, y);
                // populate field data, continued
                fd.cb = pGenericCheckBox;
            } else if (cfconf->GetFieldType(i) == CurationFieldConfiguration::combobox) {
                Wt::WString s(cfconf->GetFieldName(i));
                std::string choices(cfconf->GetComboChoice(i));
                Wt::WComboBox * pGenericComboBox =
                        SetComboBox(new Wt::WText(s), choices, fieldcontainer);
                pGenericComboBox->activated().connect(boost::bind(&ConfigurableAnnotatorContainer::ComboBoxChanged,
                        this, pGenericComboBox));
                int x = cfconf->GetX(i);
                int y = cfconf->GetY(i);
                maxx_ = (x > maxx_) ? x : maxx_;
                maxy_ = (y > maxy_) ? y : maxy_;
                gridbox->addWidget(fieldcontainer, x, y);
                fd.co = pGenericComboBox;
            }
            fielddata_.push_back(fd);
        }
        subcont_.fields->setLayout(gridbox);
    }
}

std::string ConfigurableAnnotatorContainer::GeneratePrepopulationText(int ppid) {
    PgPrepopulation * ppp = new PgPrepopulation(PGPREPOPULATION, PGPREPOPULATIONTABLENAME);
    std::string ret;
    int ppmode;
    std::string ppdata;
    std::string syndatabase;
    std::string syntablename;
    std::string syncolname;
    std::string synwhereclause;
    ppp->ReadTableData(ppid, ppmode, ppdata,
            syndatabase, syntablename, syncolname, synwhereclause);
    std::multimap<std::string, std::string> synindex;
    synindex.clear();
    if ((ppmode == PrePopulationDialog::termsandsynonyms)
            || (ppmode == PrePopulationDialog::termssynonymsandcategories)) {
        PgColumn * pgsyn = new PgColumn(syndatabase, syntablename, syncolname, synwhereclause);
        std::set<std::string>::iterator sit;
        std::set<std::string> pgresult(pgsyn->GetList());
        for (sit = pgresult.begin(); sit != pgresult.end(); sit++) {
            std::vector<std::string> splits;
            boost::split(splits, *sit, boost::is_any_of("|"));
            for (int i = 0; i < splits.size(); i++)
                for (int j = 0; j < splits.size(); j++)
                    if (j != i) synindex.insert(std::make_pair(splits[i], splits[j]));
        }
    }
    if (ppmode == PrePopulationDialog::statictext) {
        ret = ppdata;
    } else if (ppmode == PrePopulationDialog::viewerinfodata) {
        if (ppdata.compare("Terms") == 0)
            ret = CleanWhiteSpaces(curationdata_.term.toUTF8());
        else if (ppdata.compare("Positions") == 0)
            ret = CleanWhiteSpaces(curationdata_.bestring);
        else if (ppdata.compare("Add. Annotations") == 0)
            ret = CleanWhiteSpaces(curationdata_.additionalannotations);
        else if (ppdata.compare("Filename") == 0)
            ret = CleanWhiteSpaces(curationdata_.filename);
        else if (ppdata.compare("Paper Id") == 0)
            ret = CleanWhiteSpaces(curationdata_.paperid);
        else if (ppdata.compare("Annotator") == 0)
            ret = CleanWhiteSpaces(curationdata_.owner.toUTF8());
        else if (ppdata.compare("Created") == 0) {
            char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&curationdata_.lastupdate));
            ret = CleanWhiteSpaces(std::string(buff));
        } else if (ppdata.compare("Version") == 0)
            ret = CleanWhiteSpaces(curationdata_.version.toUTF8());
    } else {
        std::vector<std::string> ppcategories;
        bool catanded = (ppdata.find("&") != std::string::npos);
        if (catanded)
            boost::split(ppcategories, ppdata, boost::is_any_of("&"));
        else
            boost::split(ppcategories, ppdata, boost::is_any_of("|"));

        std::multimap<std::string, std::string> map;
        ExtractTermsFromAdditionalAnnotations(curationdata_.additionalannotations, map);
        std::string finaltext("");
        std::vector<std::string>::iterator vsit;
        bool allppcategoriesfound(true);
        for (vsit = ppcategories.begin(); vsit != ppcategories.end(); vsit++) {
            if (curationdata_.additionalannotations.find(*vsit) != std::string::npos) {
                std::multimap<std::string, std::string>::iterator mmss;
                bool ppcathit = false;
                for (mmss = map.begin(); mmss != map.end(); mmss++) {
                    // still need to take care of catanded;
                    if ((*mmss).first.compare(*vsit) == 0) {
                        if (!finaltext.empty()) finaltext += " ";
                        if (ppmode != PrePopulationDialog::categories)
                            finaltext += (*mmss).second;
                        if ((ppmode == PrePopulationDialog::termsandsynonyms)
                                || (ppmode == PrePopulationDialog::termssynonymsandcategories))
                            if (synindex.count((*mmss).second) > 0) {
                                finaltext += " [";
                                std::pair<std::multimap<std::string, std::string>::iterator, std::multimap<std::string, std::string>::iterator> pbe;
                                pbe = synindex.equal_range((*mmss).second);
                                std::multimap<std::string, std::string>::iterator mssi;
                                for (mssi = pbe.first; mssi != pbe.second; mssi++) {
                                    if (mssi != pbe.first) finaltext += ", ";
                                    finaltext += (*mssi).second;
                                }
                                finaltext += "]";
                            }
                        if ((ppmode == PrePopulationDialog::categories) ||
                                (ppmode == PrePopulationDialog::termsandcategories) ||
                                (ppmode == PrePopulationDialog::termssynonymsandcategories)) {
                            if (ppmode != PrePopulationDialog::categories) finaltext += " ";
                            finaltext += "(" + (*mmss).first + ")";
                        }
                        ppcathit = true;
                    }
                }
                allppcategoriesfound = (allppcategoriesfound && ppcathit);
            }
        }
        if (catanded && !allppcategoriesfound)
            ret = "";
        else
            ret = finaltext;
    }
    return ret;
}

void ConfigurableAnnotatorContainer::CreateButtonsContent() {
    postdatabutton_ = new Wt::WPushButton("Send data to external database");
    postdatabutton_->setStyleClass("btn-mini");
    postdatabutton_->clicked().connect(this, &ConfigurableAnnotatorContainer::PostDataButtonClicked);
    postdatabutton_->setToolTip("Send to URL provided.");
    postdatabutton_->setInline(true);
    postdatabutton_->enable();
    subcont_.buttons->addWidget(postdatabutton_);
    ca_ = new Wt::WPushButton("Cancel");
    ca_->setStyleClass("btn-mini");
    ca_->setToolTip("Cancel curation.");
    ca_->setInline(true);
    ca_->enable();
    subcont_.buttons->addWidget(ca_);
}

int ConfigurableAnnotatorContainer::GetNewRecordIdForSavingToTpDatabase() {
    PgCurationDataFromViewer * cdfv = new PgCurationDataFromViewer(PGCURATIONDATAFROMVIEWER, PGCURATIONDATAFROMVIEWERTABLENAME);
    int ret = cdfv->NewRecordId();
    delete cdfv;
    return ret;
}

void ConfigurableAnnotatorContainer::SaveToTpDatabase(int newrecordid) {
    // save to postgres
    PgCurationDataFromViewer * cdfv = new PgCurationDataFromViewer(PGCURATIONDATAFROMVIEWER, PGCURATIONDATAFROMVIEWERTABLENAME);
    cdfv->SaveData(newrecordid,
            CleanWhiteSpaces(curationdata_.term.toUTF8()),
            CleanWhiteSpaces(curationdata_.bestring),
            CleanWhiteSpaces(curationdata_.additionalannotations),
            curationdata_.filename,
            curationdata_.owner.toUTF8(),
            curationdata_.paperid,
            curationdata_.lastupdate,
            curationdata_.version.toUTF8());
    CurationFieldConfiguration * cfc = new CurationFieldConfiguration(PGCURATIONFIELDS,
            PGCURATIONFIELDSTABLENAME, formname_);
    for (int i = 0; i < fielddata_.size(); i++) {
        CurationFieldConfiguration::fieldtype fieldtype(fielddata_[i].fieldtype);
        std::string fieldname(fielddata_[i].label);
        bool d(false);
        std::string data("");
        switch (fielddata_[i].fieldtype) {
            case CurationFieldConfiguration::lineedit:
                data = CleanWhiteSpaces(fielddata_[i].le->text().toUTF8());
                break;
            case CurationFieldConfiguration::textarea:
                data = CleanWhiteSpaces(fielddata_[i].ta->text().toUTF8());
                break;
            case CurationFieldConfiguration::checkbox:
                d = fielddata_[i].cb->isChecked();
                break;
            case CurationFieldConfiguration::combobox:
                data = fielddata_[i].co->currentText().toUTF8();
                break;
            case CurationFieldConfiguration::listmaker:
                data = CleanWhiteSpaces(fielddata_[i].ta->text().toUTF8());
                break;
        }
        cfc->SaveFieldData(newrecordid, fieldtype, fieldname, d, data);
    }
    delete cfc;
    delete cdfv;
}

void ConfigurableAnnotatorContainer::PostDataButtonClicked() {
    //
    // viewer info data items
    // iid
    // term
    // paper_id (to be queried from postgres)
    // file_location
    // position_in_sofastring
    // owner
    // last_update
    // version
    //

    std::string uri;
    std::string msg;
    std::string paramformat;
    long unsigned int iparamformat;
    // assign preliminary tpdbid
    tpdbid_ = GetNewRecordIdForSavingToTpDatabase();
    std::stringstream nridss;
    nridss << std::setfill('0') << std::setw(7) << tpdbid_;
    std::string tpdbidstring = "TPDBID:" + nridss.str();
    // get server URL and parameter format from Postgres
    CurationFormsConfiguration * cfc = new CurationFormsConfiguration(PGCURATIONFORMS, PGCURATIONFORMSTABLENAME);
    for (int i = 0; i < cfc->GetTableSize(); i++) {
        if ((cfc->GetFormName(i)).compare(formname_) == 0) {
            uri = cfc->GetPostUrl(i);
            if (uri.length() > 1)
                if (uri.find("$") == 0)
                    if (uri.rfind("$") == uri.length() - 1) {
                        std::string urlparametername = uri.substr(1, uri.length() - 2);
                        Wt::Http::ParameterValues values(urlparams_->GetParameterValues(urlparametername));
                        if (!values.empty())
                            uri = values[0];
                        else
                            uri = "ERROR resolving parameter " + urlparametername + ".";
                    }
            iparamformat = cfc->GetParameterFormat(i);
            paramformat = cfc->GetFormatName(iparamformat);
            break;
        }
    }
    if (iparamformat == CurationFormsConfiguration::JSON) {
        std::string parsestring("{\n");
        if (includepaperinfo_->isChecked()) {
            parsestring += "\"terms\": \""
                    + CleanWhiteSpaces(curationdata_.term.toUTF8()) + "\",\n";
            parsestring += "\"positions\": \""
                    + CleanWhiteSpaces(curationdata_.bestring) + "\",\n";
            parsestring += "\"additional annotations\": \""
                    + CleanWhiteSpaces(curationdata_.additionalannotations) + "\",\n";
            parsestring += "\"filename\": \""
                    + CleanWhiteSpaces(curationdata_.filename) + "\",\n";
            parsestring += "\"paper id\": \""
                    + CleanWhiteSpaces(curationdata_.paperid) + "\",\n";
            parsestring += "\"annotator\": \""
                    + CleanWhiteSpaces(curationdata_.owner.toUTF8()) + "\",\n";
            char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&curationdata_.lastupdate));
            parsestring += "\"creation time\": \""
                    + CleanWhiteSpaces(std::string(buff)) + "\",\n";
            parsestring += "\"version\": \""
                    + CleanWhiteSpaces(curationdata_.version.toUTF8()) + "\"";
            if ((fielddata_.size() > 0) || (includetpdbid_->isChecked())) parsestring += ",";
            parsestring += "\n";
        }
        if (includetpdbid_->isChecked())
            parsestring += "\"external_id\": \"" + tpdbidstring + "\",\n";
        for (std::vector<std::string>::iterator it = moreurlparams_.begin();
                it != moreurlparams_.end(); it++) {
            std::string aux(*it);
            boost::replace_first(aux, "=", "\": \"");
            parsestring += "\"" + aux + "\",\n";
        }
        for (int i = 0; i < fielddata_.size(); i++) {
            std::string label = fielddata_[i].label;
            std::string content;
            switch (fielddata_[i].fieldtype) {
                case CurationFieldConfiguration::checkbox:
                    content = (fielddata_[i].cb->isChecked()) ? "yes" : "no";
                    break;
                case CurationFieldConfiguration::lineedit:
                    content = CleanWhiteSpaces(fielddata_[i].le->text().toUTF8());
                    break;
                case CurationFieldConfiguration::textarea:
                    content = CleanWhiteSpaces(fielddata_[i].ta->text().toUTF8());
                    break;
                case CurationFieldConfiguration::combobox:
                    content = fielddata_[i].co->currentText().toUTF8();
                    break;
                case CurationFieldConfiguration::listmaker:
                    content = CleanWhiteSpaces(fielddata_[i].ta->text().toUTF8());
                    break;
            }
            parsestring += "\"" + label + "\": \"" + content + "\"";
            if (i < fielddata_.size() - 1) parsestring += ",";
            parsestring += "\n";
        }
        parsestring += "}\n";
        Wt::Json::Object result;
        Wt::Json::parse(parsestring, result);
        msg = Wt::Json::serialize(result);
    } else if (iparamformat == CurationFormsConfiguration::URI) {
        std::string parameters("");
        if (includepaperinfo_->isChecked()) {
            parameters = "&terms="
                    + CleanWhiteSpaces(curationdata_.term.toUTF8());
            parameters += "&positions="
                    + CleanWhiteSpaces(curationdata_.bestring);
            parameters += "&additional annotations="
                    + CleanWhiteSpaces(curationdata_.additionalannotations);
            parameters += "&filename="
                    + CleanWhiteSpaces(curationdata_.filename);
            parameters += "&paper id="
                    + CleanWhiteSpaces(curationdata_.paperid);
            parameters += "&annotator="
                    + CleanWhiteSpaces(curationdata_.owner.toUTF8());
            char buff[20];
            strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&curationdata_.lastupdate));
            parameters += "&creation time="
                    + CleanWhiteSpaces(std::string(buff));
            parameters += "&version="
                    + CleanWhiteSpaces(curationdata_.version.toUTF8());
        }
        for (int i = 0; i < fielddata_.size(); i++) {
            std::string label = fielddata_[i].label;
            std::string content;
            switch (fielddata_[i].fieldtype) {
                case CurationFieldConfiguration::checkbox:
                    content = (fielddata_[i].cb->isChecked()) ? "yes" : "no";
                    break;
                case CurationFieldConfiguration::lineedit:
                    content = CleanWhiteSpaces(fielddata_[i].le->text().toUTF8());
                    break;
                case CurationFieldConfiguration::textarea:
                    content = CleanWhiteSpaces(fielddata_[i].ta->text().toUTF8());
                    break;
                case CurationFieldConfiguration::combobox:
                    content = fielddata_[i].co->currentText().toUTF8();
                    break;
                case CurationFieldConfiguration::listmaker:
                    content = CleanWhiteSpaces(fielddata_[i].ta->text().toUTF8());
                    break;
            }
            parameters += "&" + label + "=" + content;
        }
        if (includetpdbid_->isChecked()) parameters += "&external_id=" + tpdbidstring;
        for (std::vector<std::string>::iterator it = moreurlparams_.begin();
                it != moreurlparams_.end(); it++)
            parameters += "&" + *it;
        if (uri.find("?") != std::string::npos)
            uri += parameters;
        else {
            boost::replace_first(parameters, "&", "?");
            uri += parameters;
        }
        msg = parameters;
    }
    //
    httpdialog_ = new Wt::WDialog("HTTP Post/Get to External Database", this);
    httpdialog_->setModal(false);
    httpdialog_->setResizable(true);
    httpdialog_->setClosable(true);
    httpdialog_->finished().connect(this, &ConfigurableAnnotatorContainer::DialogDone);
    httpdialog_->contents()->setMaximumSize(Wt::WLength(80, Wt::WLength::FontEx), Wt::WLength::Auto);
    //
    if (uri.empty())
        httpdialog_->contents()->addWidget(new Wt::WText("No server URL found!"));
    else {
        Wt::WText * sendtext = new Wt::WText("Send as: ");
        Wt::WPushButton * sendbuttonpost = new Wt::WPushButton("HTTP Post");
        Wt::WPushButton * sendbuttonget = new Wt::WPushButton("HTTP Get");
        sendtext->setInline(true);
        sendbuttonpost->setInline(true);
        sendbuttonget->setInline(true);
        sendbuttonpost->clicked().connect(boost::bind(&ConfigurableAnnotatorContainer::HandDownToHttpClient,
                this, uri, msg, iparamformat));
        Wt::WLink link(uri);
        sendbuttonget->setLink(link);
        sendbuttonget->setLinkTarget(Wt::AnchorTarget::TargetNewWindow);
        sendbuttonget->clicked().connect(httpdialog_, &Wt::WDialog::accept);
        httpdialog_->contents()->addWidget(new Wt::WText("Server: " + uri));
        httpdialog_->contents()->addWidget(new Wt::WBreak());
        httpdialog_->contents()->addWidget(new Wt::WText("Format for data transmission: " + paramformat));
        httpdialog_->contents()->addWidget(new Wt::WBreak());
        httpdialog_->contents()->addWidget(sendtext);
        if (iparamformat == CurationFormsConfiguration::JSON) {
            sendbuttonpost->decorationStyle().setForegroundColor(Wt::green);
            httpdialog_->contents()->addWidget(sendbuttonpost);
            httpdialog_->contents()->addWidget(sendbuttonget);
        } else if (iparamformat == CurationFormsConfiguration::URI) {
            sendbuttonget->decorationStyle().setForegroundColor(Wt::green);
            httpdialog_->contents()->addWidget(sendbuttonget);
            httpdialog_->contents()->addWidget(sendbuttonpost);
        }
    }
    httpdialog_->show();
}

void ConfigurableAnnotatorContainer::DialogDone(Wt::WDialog::DialogCode code) {
    httpdialog_->contents()->clear();
    delete httpdialog_;
}

void ConfigurableAnnotatorContainer::HandDownToHttpClient(std::string uri, std::string msg, long unsigned int iparamformat) {
    httpdialog_->contents()->clear();
    httpdialog_->contents()->addWidget(new Wt::WText("Waiting for response from server..."));
    Wt::Http::Message message;
    if (iparamformat == CurationFormsConfiguration::JSON)
        message.setHeader("Content-Type", "application/json; charset=utf-8");
    else if (iparamformat == CurationFormsConfiguration::URI)
        message.setHeader("Content-Type", "application/x-www-form-urlencoded; charset=utf-8");
    message.addHeader("User-Agent", "Textpresso Central Application");
    message.addBodyText(msg);
    client_ = new Wt::Http::Client(this);
    client_->setTimeout(10);
    client_->done().connect(boost::bind(&ConfigurableAnnotatorContainer::HandleHttpResponse, this, _1, _2));
    std::cerr << "uri: " << uri << std::endl;
    std::cerr << "urlencode(uri): " << urlencode(uri) << std::endl;
    if (!client_->post(urlencode(uri), message))
        std::cerr << "Post request in HandDownToHttpClient could not be scheduled." << std::endl;
    else
        Wt::WApplication::instance()->deferRendering();
}

void ConfigurableAnnotatorContainer::HandleHttpResponse(boost::system::error_code err, const Wt::Http::Message & response) {
    Wt::WApplication::instance()->resumeRendering();
    httpdialog_->contents()->clear();
    //    if (!err) {
    httpdialog_->contents()->addWidget(new Wt::WText("Response from server:"));
    httpdialog_->contents()->addWidget(new Wt::WBreak());
    if (response.status() == 200) {
        std::string aux = response.body();
        Wt::WText * responsebody = new Wt::WText(aux, Wt::TextFormat::PlainText);
        httpdialog_->contents()->addWidget(responsebody);
        httpdialog_->contents()->addWidget(new Wt::WBreak());
        std::cerr << "Successfully submitted to server. Save data locally. ";
        std::cerr << "TPDBID: " << tpdbid_ << std::endl;
        SaveToTpDatabase(tpdbid_);
    } else {
        std::stringstream resp;
        resp << response.status() << " ";
        resp << response.body() << std::endl;
        httpdialog_->contents()->addWidget(new Wt::WText(resp.str()));
    }
    httpdialog_->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", httpdialog_->contents());
    ok->clicked().connect(httpdialog_, &Wt::WDialog::accept);
    delete client_;
}

void ConfigurableAnnotatorContainer::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}

void ConfigurableAnnotatorContainer::InfoIconClicked(Wt::WString text) {
    Wt::WString title("Autocomplete Functionality");
    infomessage_ = new Wt::WMessageBox(title, text, Wt::NoIcon, Wt::Ok);
    infomessage_->buttonClicked().connect(this, &ConfigurableAnnotatorContainer::InfoOkButtonClicked);
    infomessage_->setWidth(Wt::WLength(60, Wt::WLength::FontEx));
    infomessage_->show();
}

void ConfigurableAnnotatorContainer::InfoOkButtonClicked(Wt::StandardButton b) {
    delete infomessage_;
}

void ConfigurableAnnotatorContainer::LineEditClicked(Wt::WLineEdit * le) {
    if (clicknfilltext_->isChecked() || clicknfillcat_->isChecked()) {
        Viewer * vp = dynamic_cast<Viewer*> (parent_);
        long unsigned int noofBsniplets = (vp->GetCurB()).size();
        long unsigned int noofEsniplets = (vp->GetCurE()).size();
        if (noofBsniplets > 0) {
            Wt::WText * strtptr = (vp->GetCurB())[noofBsniplets - 1];
            Wt::WText * endptr = (noofBsniplets - 1 < noofEsniplets) ?
                    (vp->GetCurE())[noofBsniplets - 1] : (vp->GetCurB())[noofBsniplets - 1];
            std::vector<Wt::WText*> t = vp->GetTerms();
            Wt::WString stext("");
            if (clicknfilltext_->isChecked())
                stext = GetTextFromTerms(strtptr, endptr, t);
            Wt::WString scat("");
            if (clicknfillcat_->isChecked())
                scat = GetCatFromTerms(strtptr, endptr, t, vp->GetPointer2Annotations());
            Wt::WString ssum("");
            if (clicknfilltext_->isChecked()) {
                ssum = stext;
                if (clicknfillcat_->isChecked())
                    ssum += Wt::WString(" (") + scat + Wt::WString(") ");
            } else
                ssum = scat;
            le->setText(le->text() + ssum);
        }
    }
}

void ConfigurableAnnotatorContainer::TextAreaClicked(Wt::WTextArea * ta) {
    if (clicknfilltext_->isChecked() || clicknfillcat_->isChecked()) {
        Viewer * vp = dynamic_cast<Viewer*> (parent_);
        long unsigned int noofBsniplets = (vp->GetCurB()).size();
        long unsigned int noofEsniplets = (vp->GetCurE()).size();
        if (noofBsniplets > 0) {
            Wt::WText * strtptr = (vp->GetCurB())[noofBsniplets - 1];
            Wt::WText * endptr = (noofBsniplets - 1 < noofEsniplets) ?
                    (vp->GetCurE())[noofBsniplets - 1] : (vp->GetCurB())[noofBsniplets - 1];
            std::vector<Wt::WText*> t = vp->GetTerms();
            Wt::WString stext("");
            if (clicknfilltext_->isChecked())
                stext = GetTextFromTerms(strtptr, endptr, t);
            Wt::WString scat("");
            if (clicknfillcat_->isChecked())
                scat = GetCatFromTerms(strtptr, endptr, t, vp->GetPointer2Annotations());
            Wt::WString ssum("");
            if (clicknfilltext_->isChecked()) {
                ssum = stext;
                if (clicknfillcat_->isChecked())
                    ssum += Wt::WString(" (") + scat + Wt::WString(") ");
            } else
                ssum = scat;
            ta->setText(ta->text() + ssum);
        }
    }
}

void ConfigurableAnnotatorContainer::LineEditChanged(Wt::WLineEdit * pLE, FetchList * feli) {
    if (pLE->text().toUTF8().length() > 2) {
        Wt::WBorder bx;
        bx.setStyle(Wt::WBorder::Solid);
        bx.setWidth(Wt::WBorder::Medium);
        if (feli->IsInList(pLE->text().toUTF8()))
            bx.setColor(Wt::green);
        else
            bx.setColor(Wt::red);
        pLE->decorationStyle().setBorder(bx);
    }
};

void ConfigurableAnnotatorContainer::MoreParametersClicked() {
    mpdialog_ = new Wt::WDialog("URL Parameters", this);
    mpdialog_->setModal(false);
    mpdialog_->setResizable(true);
    mpdialog_->setClosable(true);
    mpdialog_->finished().connect(this, &ConfigurableAnnotatorContainer::MPDialogDone);
    mpdialog_->contents()->setMaximumSize(Wt::WLength(80, Wt::WLength::FontEx), Wt::WLength::Auto);
    Wt::WVBoxLayout * vbox = new Wt::WVBoxLayout();
    mpdialog_->contents()->setLayout(vbox);
    std::vector<std::string> vec(urlparams_->GetParmVector());
    moreurlparams_.clear();
    for (std::vector<std::string>::iterator it = vec.begin(); it != vec.end(); it++) {
        Wt::WCheckBox * mpcb = new Wt::WCheckBox(*it);
        moreurlparams_.push_back(*it);
        mpcb->setChecked(true);
        mpcb->changed().connect(std::bind([ = ] (){
            std::string fs(mpcb->text().toUTF8());
            if (mpcb->isChecked())
                    moreurlparams_.push_back(fs);
            else {
                std::vector<std::string>::iterator it =
                        std::find(moreurlparams_.begin(), moreurlparams_.end(), mpcb->text().toUTF8());
                        moreurlparams_.erase(it);
            }
        }));
        vbox->addWidget(mpcb);
    }
    Wt::WPushButton * ok = new Wt::WPushButton("Ok");
    ok->clicked().connect(mpdialog_, &Wt::WDialog::accept);
    vbox->addWidget(ok);
    mpdialog_->show();
}

void ConfigurableAnnotatorContainer::MPDialogDone(Wt::WDialog::DialogCode code) {
    mpdialog_->contents()->clear();
    delete mpdialog_;
}

void ConfigurableAnnotatorContainer::CheckBoxClicked(Wt::WCheckBox * cb) {
}

void ConfigurableAnnotatorContainer::ComboBoxChanged(Wt::WComboBox * co) {
}

ConfigurableAnnotatorContainer::~ConfigurableAnnotatorContainer() {
    clear();
}
