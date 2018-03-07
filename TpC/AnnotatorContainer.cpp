/* 
 * File:   AnnotatorContainer.cpp
 * Author: mueller
 * 
 * Created on March 12, 2014, 12:33 PM
 */


#include "AnnotatorContainer.h"
#include "SuggestionBoxFromPgOntology.h"
#include "TpCurApi.h"
#include "Viewer.h"
#include "../TextpressoCentralGlobals.h"

#include <Wt/WBreak>
#include <Wt/WCssDecorationStyle>
#include <Wt/WGridLayout>
#include <Wt/WLineEdit>
#include <Wt/WText>
#include <Wt/WTextArea>
#include <Wt/WImage>
#include <Wt/WMessageBox>
#include <Wt/Json/Object>
#include <Wt/Json/Parser>
#include <Wt/Json/Serializer>
#include <Wt/WComboBox>
#include <Wt/Http/Message>
#include <Wt/Http/Client>
#include <Wt/WApplication>

#include <boost/algorithm/string.hpp>

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

    void SetLineEdit(Wt::WLineEdit * e, Wt::WText * l, Wt::WContainerWidget * p) {
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        p->addWidget(l);
        p->addWidget(new Wt::WBreak());
        e->setWidth(Wt::WLength(48, Wt::WLength::FontEx));
        e->setHeight(Wt::WLength(2, Wt::WLength::FontEx));
        p->addWidget(e);
    }

    void SetTextArea(Wt::WTextArea * t, Wt::WText * l, Wt::WImage * i, Wt::WContainerWidget * p) {
        l->decorationStyle().font().setWeight(Wt::WFont::Bold);
        l->setInline(true);
        p->addWidget(l);
        i->setInline(true);
        p->addWidget(i);
        p->addWidget(new Wt::WBreak());
        t->setWidth(Wt::WLength(47, Wt::WLength::FontEx));
        t->setHeight(Wt::WLength(10, Wt::WLength::FontEx));
        p->addWidget(t);
    }

    std::string CleanWhiteSpaces(std::string inp) {
        std::string out = boost::replace_all_copy(inp, "\r", " ");
        boost::replace_all(out, "\n", " ");
        boost::replace_all(out, "\t", " ");
        while (out.find("  ") != std::string::npos) boost::replace_all(out, "  ", " ");
        return out;
    }
}

// basic operations

AnnotatorContainer::AnnotatorContainer(std::string rootfn, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent) {
    parent_ = parent;
    rootfn_ = rootfn;
    subcont_.immutables = NULL;
    subcont_.manuals = NULL;
    subcont_.buttons = NULL;
}

AnnotatorContainer::~AnnotatorContainer() {
    DeleteAll();
}

void AnnotatorContainer::CreateAll(immutables inp) {
    decorationStyle().setBackgroundColor(Wt::WColor(250, 250, 250));
    subcont_.immutables = CreateImmutablesContent(inp);
    subcont_.manuals = CreateManualsContent();
    subcont_.buttons = CreateButtonsContent();
    addWidget(subcont_.immutables);
    addWidget(subcont_.manuals);
    addWidget(subcont_.buttons);
}

void AnnotatorContainer::CreateAllEmpty() {
    decorationStyle().setBackgroundColor(Wt::WColor(250, 250, 250));
    clicknfill_ = new Wt::WCheckBox("Click'n Fill");
    addWidget(clicknfill_);
    subcont_.immutables = CreateEmptyImmutablesForm();
    subcont_.manuals = CreateManualsContent();
    subcont_.buttons = CreateButtonsContent();
    addWidget(subcont_.immutables);
    addWidget(subcont_.manuals);
    addWidget(subcont_.buttons);
}

Wt::WContainerWidget * AnnotatorContainer::CreateEmptyImmutablesForm() {
    Wt::WContainerWidget * ret = new Wt::WContainerWidget();
    ret->resize(Wt::WLength(120, Wt::WLength::FontEx), Wt::WLength::Auto);
    // data to be populated:
    // iid
    // term
    // paper_id (to be queried from postgres)
    // file_location
    // position_in_sofastring
    // owner
    // last_update
    // version
    texts_.term = new Wt::WText();
    texts_.term->setWordWrap(true);
    SetText(texts_.term, new Wt::WText("Terms: "), ret);
    ret->addWidget(new Wt::WBreak());
    //
    texts_.bestring = new Wt::WText();
    SetText(texts_.bestring, new Wt::WText("Positions: "), ret);
    ret->addWidget(new Wt::WBreak());
    //
    texts_.filename = new Wt::WText();
    SetText(texts_.filename, new Wt::WText("Filename: "), ret);
    //
    texts_.paperid = new Wt::WText();
    SetText(texts_.paperid, new Wt::WText(" Paper Id: "), ret);
    //
    texts_.owner = new Wt::WText();
    SetText(texts_.owner, new Wt::WText(" Annotator: "), ret);
    //
    texts_.lastupdate = new Wt::WText();
    SetText(texts_.lastupdate, new Wt::WText(" Created: "), ret);
    //
    texts_.version = new Wt::WText();
    SetText(texts_.version, new Wt::WText(" Version: "), ret);
    ret->addWidget(new Wt::WBreak());
    return ret;
}

Wt::WString AnnotatorContainer::GetTextFromTerms(Wt::WText * strtptr,
        Wt::WText * endptr, std::vector<Wt::WText*> & terms) {
    Wt::WString ret;
    long unsigned int startind = FindIndexFromPointer(strtptr, terms);
    long unsigned int endind = FindIndexFromPointer(endptr, terms);
    //
    for (int j = startind; j <= endind; j++)
        ret += terms[j]->text();
    return ret;
}

std::string AnnotatorContainer::GetBEFromInp(Wt::WText * strtptr, Wt::WText * endptr, immutables inp) {
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

void AnnotatorContainer::PopulateImmutablesContent(immutables inp) {
    //
    // immutable data items
    // iid
    // term
    // paper_id (to be queried from postgres)
    // file_location
    // position_in_sofastring
    // owner
    // last_update
    // version
    //
    psqldata_.term = "";
    psqldata_.bestring = "";
    long unsigned int noofBsniplets = inp.curB.size();
    long unsigned int noofEsniplets = inp.curE.size();
    for (long unsigned int i = 0; i < noofBsniplets; i++) {
        //
        Wt::WText * strtptr = inp.curB[i];
        Wt::WText * endptr = (i < noofEsniplets) ? inp.curE[i] : inp.curB[i];
        psqldata_.term += GetTextFromTerms(strtptr, endptr, inp.terms);
        psqldata_.bestring += GetBEFromInp(strtptr, endptr, inp);
        if (i < noofBsniplets) {
            psqldata_.bestring += " ";
            psqldata_.term += "|";
        }
    }
    psqldata_.filename = inp.filename;
    //get rid of root dir name
    boost::replace_all(psqldata_.filename, rootfn_, "");
    psqldata_.owner = inp.owner;
    psqldata_.paperid = inp.paperid;
    psqldata_.lastupdate = time(0);
    psqldata_.version = "1.0";
    //
    texts_.term->setText(psqldata_.term);
    texts_.bestring->setText(psqldata_.bestring);
    texts_.filename->setText(psqldata_.filename);
    texts_.paperid->setText(psqldata_.paperid);
    texts_.owner->setText(psqldata_.owner);
    char buff[20];
    strftime(buff, 20, "%Y-%m-%d %H:%M:%S", localtime(&psqldata_.lastupdate));
    texts_.lastupdate->setText(buff);
    texts_.version->setText(psqldata_.version);
}

Wt::WContainerWidget * AnnotatorContainer::CreateImmutablesContent(immutables inp) {
    Wt::WContainerWidget * ret = CreateEmptyImmutablesForm();
    PopulateImmutablesContent(inp);
    return ret;
}

Wt::WContainerWidget * AnnotatorContainer::CreateManualsContent() {
    Wt::WContainerWidget * ret = new Wt::WContainerWidget();
    //
    // data to be entered by curator:
    // eid
    // dbxref
    // evidence_code
    // curation_status
    // curation_use
    // source
    // ontology_terms
    // biological_entities
    // comment
    psqldata_.eid = new Wt::WLineEdit();
    Wt::WContainerWidget * l1 = new Wt::WContainerWidget();
    SetLineEdit(psqldata_.eid, new Wt::WText("Annotation Id"), l1);
    psqldata_.eid->clicked().connect(boost::bind(&AnnotatorContainer::LineEditClicked, this, psqldata_.eid));
    //
    psqldata_.dbxref = new Wt::WLineEdit();
    Wt::WContainerWidget * l2 = new Wt::WContainerWidget();
    SetLineEdit(psqldata_.dbxref, new Wt::WText("DB Crossreferences"), l2);
    psqldata_.dbxref->clicked().connect(boost::bind(&AnnotatorContainer::LineEditClicked, this, psqldata_.dbxref));
    //
    psqldata_.evidencecode = new Wt::WLineEdit();
    Wt::WContainerWidget * l3 = new Wt::WContainerWidget();
    SetLineEdit(psqldata_.evidencecode, new Wt::WText("Evidence code"), l3);
    psqldata_.evidencecode->clicked().connect(boost::bind(&AnnotatorContainer::LineEditClicked, this, psqldata_.evidencecode));
    //
    psqldata_.curationstatus = new Wt::WLineEdit();
    Wt::WContainerWidget * l4 = new Wt::WContainerWidget();
    SetLineEdit(psqldata_.curationstatus, new Wt::WText("Curation status"), l4);
    psqldata_.curationstatus->clicked().connect(boost::bind(&AnnotatorContainer::LineEditClicked, this, psqldata_.curationstatus));
    //
    psqldata_.curationuse = new Wt::WLineEdit();
    Wt::WContainerWidget * l5 = new Wt::WContainerWidget();
    SetLineEdit(psqldata_.curationuse, new Wt::WText("Curation use"), l5);
    psqldata_.curationuse->clicked().connect(boost::bind(&AnnotatorContainer::LineEditClicked, this, psqldata_.curationuse));
    //
    psqldata_.source = new Wt::WLineEdit();
    Wt::WContainerWidget * l6 = new Wt::WContainerWidget();
    SetLineEdit(psqldata_.source, new Wt::WText("Source"), l6);
    psqldata_.source->clicked().connect(boost::bind(&AnnotatorContainer::LineEditClicked, this, psqldata_.source));
    //
    psqldata_.ontologyterms = new Wt::WTextArea();
    psqldata_.ontologyterms->clicked().connect(boost::bind(&AnnotatorContainer::TextAreaClicked, this, psqldata_.ontologyterms));
    Wt::WContainerWidget * a1 = new Wt::WContainerWidget();
    Wt::WString infotext;
    infotext = "Autocomplete is triggered whenever the cursor is at the beginning ";
    infotext += "of a line. Hit 'Enter' to get to a new line. ";
    infotext += "The suggestions are retrieved from a database. Very common words ";
    infotext += "such as 'protein' would trigger 10000s of matches, so the suggestion ";
    infotext += "list is cut off at 750 entries. You can still retrieve hidden ";
    infotext += "suggestions by typing more specific terms. The list of suggestions ";
    infotext += "is continuously updated as you type.";

    Wt::WImage * i1 = new Wt::WImage("resources/icons/information16.png");
    i1->mouseWentOver().connect(boost::bind(&AnnotatorContainer::SetCursorHand, this, i1));
    i1->clicked().connect(boost::bind(&AnnotatorContainer::InfoIconClicked, this, infotext));
    SetTextArea(psqldata_.ontologyterms, new Wt::WText("Ontology Terms "), i1, a1);
    SuggestionBoxFromPgOntology * sbot
            = new SuggestionBoxFromPgOntology("term", "", PGONTOLOGYTABLENAME, PGONTOLOGY);
    sbot->forEdit(psqldata_.ontologyterms);
    //
    psqldata_.biologicalentities = new Wt::WTextArea();
    psqldata_.biologicalentities->clicked().connect(boost::bind(&AnnotatorContainer::TextAreaClicked, this, psqldata_.biologicalentities));
    Wt::WContainerWidget * a2 = new Wt::WContainerWidget();
    Wt::WImage * i2 = new Wt::WImage("resources/icons/information16.png");
    i2->mouseWentOver().connect(boost::bind(&AnnotatorContainer::SetCursorHand, this, i2));
    i2->clicked().connect(boost::bind(&AnnotatorContainer::InfoIconClicked, this, infotext));
    SetTextArea(psqldata_.biologicalentities, new Wt::WText("Biological Entities "), i2, a2);
    SuggestionBoxFromPgOntology * sbbe
            = new SuggestionBoxFromPgOntology("term", "eid ~'^PR'", PGONTOLOGYTABLENAME, PGONTOLOGY);
    sbbe->forEdit(psqldata_.biologicalentities);
    //
    psqldata_.comment = new Wt::WTextArea();
    psqldata_.comment->clicked().connect(boost::bind(&AnnotatorContainer::TextAreaClicked, this, psqldata_.comment));

    Wt::WContainerWidget * a3 = new Wt::WContainerWidget();
    infotext = "The comment box does not feature any autocompletion functionality.";
    Wt::WImage * i3 = new Wt::WImage("resources/icons/information16.png");
    i3->mouseWentOver().connect(boost::bind(&AnnotatorContainer::SetCursorHand, this, i3));
    i3->clicked().connect(boost::bind(&AnnotatorContainer::InfoIconClicked, this, infotext));
    SetTextArea(psqldata_.comment, new Wt::WText("Comments "), i3, a3);
    //
    Wt::WGridLayout * hbox = new Wt::WGridLayout();
    hbox->addWidget(l1, 0, 0);
    hbox->addWidget(l2, 0, 1);
    hbox->addWidget(l3, 0, 2);
    hbox->addWidget(a1, 1, 0);
    hbox->addWidget(a2, 1, 1);
    hbox->addWidget(a3, 1, 2);
    hbox->addWidget(l4, 2, 0);
    hbox->addWidget(l5, 2, 1);
    hbox->addWidget(l6, 2, 2);
    ret->setLayout(hbox);
    return ret;
}

Wt::WContainerWidget * AnnotatorContainer::CreateButtonsContent() {
    Wt::WContainerWidget * ret = new Wt::WContainerWidget();
    //
    // IMPLEMENT VALIDATION ACCORDING TO WITTY PAGE VALIDATION
    //
    sf_ = new Wt::WPushButton("Save to Textpresso database");
    sf_->setStyleClass("btn-mini");
    sf_->clicked().connect(this, &AnnotatorContainer::SfButtonClicked);
    sf_->setToolTip("Save annotation and close window.");
    sf_->setInline(true);
    sf_->enable();
    ret->addWidget(sf_);
    js_ = new Wt::WPushButton("Post data to external database");
    js_->setStyleClass("btn-mini");
    js_->clicked().connect(this, &AnnotatorContainer::JsonButtonClicked);
    js_->setToolTip("Send Json object.");
    js_->setInline(true);
    js_->enable();
    ret->addWidget(js_);
    ca_ = new Wt::WPushButton("Cancel");
    ca_->setStyleClass("btn-mini");
    ca_->clicked().connect(this, &AnnotatorContainer::CaButtonClicked);
    ca_->setToolTip("Cancel curation.");
    ca_->setInline(true);
    ca_->enable();
    ret->addWidget(ca_);
    return ret;
}

void AnnotatorContainer::DeleteAll() {
    DeleteImmutablesContent();
    DeleteManualsContent();
    DeleteButtonsContent();
    clear();
}

void AnnotatorContainer::DeleteImmutablesContent() {
    if (subcont_.immutables != NULL) delete subcont_.immutables;
}

void AnnotatorContainer::DeleteManualsContent() {
    if (subcont_.manuals != NULL) delete subcont_.manuals;
}

void AnnotatorContainer::DeleteButtonsContent() {
    if (subcont_.buttons != NULL) delete subcont_.buttons;
}

void AnnotatorContainer::SfButtonClicked() {
    // save to postgres
    TpCurApi * tpca = new TpCurApi();
    TpCurApi::dataentry d;
    d.biological_entities = CleanWhiteSpaces(psqldata_.biologicalentities->text().toUTF8());
    d.comment = CleanWhiteSpaces(psqldata_.comment->text().toUTF8());
    d.curation_status = CleanWhiteSpaces(psqldata_.curationstatus->text().toUTF8());
    d.curation_use = CleanWhiteSpaces(psqldata_.curationuse->text().toUTF8());
    d.dbxref = CleanWhiteSpaces(psqldata_.dbxref->text().toUTF8());
    d.eid = CleanWhiteSpaces(psqldata_.eid->text().toUTF8());
    d.evidence_code = CleanWhiteSpaces(psqldata_.evidencecode->text().toUTF8());
    d.file_location = CleanWhiteSpaces(psqldata_.filename);
    d.last_update = psqldata_.lastupdate;
    d.ontology_terms = CleanWhiteSpaces(psqldata_.ontologyterms->text().toUTF8());
    d.owner = CleanWhiteSpaces(psqldata_.owner.toUTF8());
    d.paper_id = CleanWhiteSpaces(psqldata_.paperid);
    d.position_in_sofastring = CleanWhiteSpaces(psqldata_.bestring);
    d.source = CleanWhiteSpaces(psqldata_.source->text().toUTF8());
    d.term = CleanWhiteSpaces(psqldata_.term.toUTF8());
    d.version = CleanWhiteSpaces(psqldata_.version.toUTF8());
    tpca->SubmitRow(d);
    delete tpca;
    DeleteAll();
}

void AnnotatorContainer::CaButtonClicked() {
    DeleteAll();
}

void AnnotatorContainer::JsonButtonClicked() {
    js_->disable();
    //
    std::string parsestring("{\n");
    parsestring += "\"publication_id\": \""
            + CleanWhiteSpaces(psqldata_.paperid) + "\",\n";
    parsestring += "\"annotation_id\": \""
            + CleanWhiteSpaces(psqldata_.eid->text().toUTF8()) + "\",\n";
    parsestring += "\"terms\": \""
            + CleanWhiteSpaces(psqldata_.ontologyterms->text().toUTF8()) + "\",\n";
    parsestring += "\"entities\": \""
            + CleanWhiteSpaces(psqldata_.biologicalentities->text().toUTF8()) + "\"\n";
    parsestring += "}\n";
    Wt::Json::Object result;
    Wt::Json::parse(parsestring, result);
    //
    httpdialog_ = new Wt::WDialog("HTTP Post to External Database", this);
    httpdialog_->setModal(false);
    httpdialog_->setResizable(true);
    httpdialog_->setClosable(true);
    httpdialog_->finished().connect(this, &AnnotatorContainer::DialogDone);
    //
    Wt::WText * explanation = new Wt::WText("Select server and send: ");
    //
    Wt::WComboBox * servercombobox = new Wt::WComboBox();
    for (int i = 0; i < ServerNames_No; i++)
        servercombobox->addItem(Wt::WString(ServerNames[i]));
    //
    Wt::WPushButton * sendbutton = new Wt::WPushButton("Send!");
    sendbutton->clicked().connect(boost::bind(&AnnotatorContainer::HandDownToHttpClient,
            this, Wt::Json::serialize(result), servercombobox));
    //
    servercombobox->setVerticalAlignment(Wt::AlignTop);
    //
    httpdialog_->contents()->addWidget(explanation);
    httpdialog_->contents()->addWidget(new Wt::WBreak());
    httpdialog_->contents()->addWidget(servercombobox);
    httpdialog_->contents()->addWidget(sendbutton);
    httpdialog_->show();
}

void AnnotatorContainer::HandDownToHttpClient(std::string msg, Wt::WComboBox * servercombobox) {
    std::string server = servercombobox->currentText().toUTF8();
    httpdialog_->contents()->clear();
    httpdialog_->contents()->addWidget(new Wt::WText("Waiting for response from server..."));
    Wt::Http::Message message2GO;
    message2GO.setHeader("Content-Type", "text/html; charset=utf-8");
    message2GO.addHeader("User-Agent", "Textpresso Central Application");
    message2GO.addBodyText(msg);
    Wt::Http::Client * client = new Wt::Http::Client(this);
    client->setTimeout(10);
    client->done().connect(boost::bind(&AnnotatorContainer::HandleHttpResponse, this, _1, _2));
    if (!client->post(server, message2GO))
        std::cerr << "Post request in HandDownToHttpClient could not be scheduled." << std::endl;
    else
        Wt::WApplication::instance()->deferRendering();
    std::cerr << "HDTHC finished." << std::endl;
}

void AnnotatorContainer::HandleHttpResponse(boost::system::error_code err, const Wt::Http::Message & response) {
    Wt::WApplication::instance()->resumeRendering();
    httpdialog_->contents()->clear();
    if (!err) {
        httpdialog_->contents()->addWidget(new Wt::WText("Response from server:"));
        httpdialog_->contents()->addWidget(new Wt::WBreak());
        if (response.status() == 200) {
            std::string aux = response.body();
            boost::replace_all(aux, "<", "\\<");
            boost::replace_all(aux, ">", "\\>");
            Wt::WText * responsebody = new Wt::WText(aux);
            httpdialog_->contents()->addWidget(responsebody);
        } else {
            std::stringstream resp;
            resp << response.status() << " ";
            resp << response.body() << std::endl;
            httpdialog_->contents()->addWidget(new Wt::WText(resp.str()));
        }
    } else {

        std::string aux = "HandleHttpResponse error: " + err.message() + ".";
        httpdialog_->contents()->addWidget(new Wt::WText(aux));
        httpdialog_->contents()->addWidget(new Wt::WBreak());
        aux = "If problem persists, contact textpresso@caltech.edu.";
        httpdialog_->contents()->addWidget(new Wt::WText(aux));
        Wt::log("error") << "HandleHttpResponse error: " << err.message();
    }
    httpdialog_->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton * ok = new Wt::WPushButton("Ok", httpdialog_->contents());
    ok->clicked().connect(httpdialog_, &Wt::WDialog::accept);

}

void AnnotatorContainer::DialogDone(Wt::WDialog::DialogCode code) {
    httpdialog_->contents()->clear();
    delete httpdialog_;
    js_->enable();
}

void AnnotatorContainer::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}

void AnnotatorContainer::InfoIconClicked(Wt::WString text) {
    Wt::WString title("Autocomplete Functionality");
    infomessage_ = new Wt::WMessageBox(title, text, Wt::NoIcon, Wt::Ok);
    infomessage_->buttonClicked().connect(this, &AnnotatorContainer::InfoOkButtonClicked);
    infomessage_->setWidth(Wt::WLength(60, Wt::WLength::FontEx));
    infomessage_->show();
}

void AnnotatorContainer::InfoOkButtonClicked(Wt::StandardButton b) {
    delete infomessage_;
}

void AnnotatorContainer::LineEditClicked(Wt::WLineEdit * le) {
    if (clicknfill_->isChecked()) {
        Viewer * vp = dynamic_cast<Viewer*> (parent_);
        long unsigned int noofBsniplets = (vp->GetCurB()).size();
        long unsigned int noofEsniplets = (vp->GetCurE()).size();
        Wt::WText * strtptr = (vp->GetCurB())[noofBsniplets - 1];
        Wt::WText * endptr = (noofBsniplets - 1 < noofEsniplets) ?
                (vp->GetCurE())[noofBsniplets - 1] : (vp->GetCurB())[noofBsniplets - 1];
        std::vector<Wt::WText*> t = vp->GetTerms();
        Wt::WString s = GetTextFromTerms(strtptr, endptr, t);
        le->setText(s);
    }
}

void AnnotatorContainer::TextAreaClicked(Wt::WTextArea * le) {
    if (clicknfill_->isChecked()) {
        Viewer * vp = dynamic_cast<Viewer*> (parent_);
        long unsigned int noofBsniplets = (vp->GetCurB()).size();
        long unsigned int noofEsniplets = (vp->GetCurE()).size();
        Wt::WText * strtptr = (vp->GetCurB())[noofBsniplets - 1];
        Wt::WText * endptr = (noofBsniplets - 1 < noofEsniplets) ?
                (vp->GetCurE())[noofBsniplets - 1] : (vp->GetCurB())[noofBsniplets - 1];
        std::vector<Wt::WText*> t = vp->GetTerms();
        Wt::WString s = GetTextFromTerms(strtptr, endptr, t);
        le->setText(s);
    }
}
