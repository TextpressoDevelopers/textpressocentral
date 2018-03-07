/* 
 * File:   TCNavWeb.cpp
 * Author: mueller
 * 
 * Created on April 2, 2013, 2:24 PM
 */

#include "TCNavWeb.h"
#include <Wt/Auth/AuthModel>
#include <Wt/Auth/AuthWidget>
#include <Wt/Auth/PasswordService>
#include <Wt/WLineEdit>
#include <Wt/WMessageBox>
#include <Wt/WNavigationBar>
#include <Wt/WPopupMenu>
#include <Wt/WPopupMenuItem>
#include <Wt/WStackedWidget>
#include <Wt/WText>
#include <Wt/WImage>
#include <Wt/WAnchor>
#include <Wt/WVBoxLayout>
#include <Wt/WBorder>
#include <Wt/WCssDecorationStyle>
#include <boost/filesystem.hpp>

TCNavWeb::TCNavWeb(UrlParameters * urlparameters, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent),
session_("user=www-data dbname=www-data"), urlparameters_(urlparameters) {
    dialog = nullptr;
    singlepaperitem_ = NULL;
    session_.login().changed().connect(this, & TCNavWeb::AuthEvent);
    verifieduserlabel = new Wt::WAnchor();
    verifieduserlabel->decorationStyle().setForegroundColor(Wt::WColor(255, 0, 0));
    verifieduserlabel->setMaximumSize(Wt::WLength(80, Wt::WLength::FontEx), Wt::WLength(1, Wt::WLength::FontEx));
    verifieduserlabel->mouseWentOver().connect(boost::bind(&TCNavWeb::SetCursorHand, this, verifieduserlabel));
    verifieduserlabel->clicked().connect(boost::bind(&TCNavWeb::verifyEmailAddress, this));
    loganchor_ = new Wt::WText("Login!");
    loganchor_->setMaximumSize(Wt::WLength(40, Wt::WLength::FontEx),
                               Wt::WLength(1, Wt::WLength::FontEx));
    loganchor_->clicked().connect(boost::bind(&TCNavWeb::displayLogin, this));
    loganchor_->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 255));
    loganchor_->mouseWentOver().connect(boost::bind(&TCNavWeb::SetCursorHand, this, loganchor_));
    authWidget = new AuthWidget(session_);
    authWidget->model()->addPasswordAuth(& Session::passwordAuth());
    authWidget->setRegistrationEnabled(true);
    authWidget->processEnvironment();
    if (boost::filesystem::exists("/tmp/tpcunstable")) {
        Wt::WMessageBox * maintenancenote = new Wt::WMessageBox("Maintenance", "Site might be unstable.", Wt::Information, Wt::Ok);
        maintenancenote->buttonClicked().connect(std::bind([ = ] (){
            delete maintenancenote;
        }));
        maintenancenote->show();
    }
    navigation_ = new Wt::WNavigationBar();
    navigation_->addStyleClass("main-nav");
    navigation_->setResponsive(false);
    //navigation_->setMaximumSize(Wt::WLength::Auto, Wt::WLength(1, Wt::WLength::FontEx));
    contentsStack_ = new Wt::WStackedWidget();
    contentsStack_->addStyleClass("contents");
    //
    //Add Logo
    Wt::WAnchor * anchor = new Wt::WAnchor(Wt::WLink(Wt::WLink::Url, "http://textpresso.org/"));
    anchor->setTarget(Wt::TargetThisWindow);
    Wt::WImage * logoimage = new Wt::WImage(Wt::WLink("resources/icons/textpresso_new_half.jpg"), anchor);
    logoimage->setWidth(Wt::WLength(100, Wt::WLength::Pixel));
    navigation_->addWidget(anchor, Wt::AlignRight);
    //
    // Setup a Left-aligned menu.
    leftMenu_ = new Wt::WMenu(contentsStack_);
    navigation_->addMenu(leftMenu_);
    leftMenu_->setInternalPathEnabled();
    leftMenu_->setInternalBasePath("/");
    home_ = new Home(session_, this);
    homemenuitem_ = leftMenu_->addItem("Home", home_);
    homemenuitem_->setPathComponent("home");
    home_->LoadContent();
    // Customization instantiation has to come before curation instantiation;
    customization_ = new Customization(session_);

    search_ = new Search(urlparameters_, session_, this);
    searchmenuitem_ = leftMenu_->addItem("Search", search_);
    searchmenuitem_->setPathComponent("search");
    home_->LoadContent(search_);

    aboutus_ = new AboutUs();
    curation_ = new Curation(session_, urlparameters_, this);
    curationmenuitem_ = leftMenu_->addItem("Curation", curation_);
    curationmenuitem_->setPathComponent("curation");
    curationmenuitem_->triggered().connect(this, &TCNavWeb::MenuItemTriggered);
    //    leftMenu->addItem("Curation database", new CurationDatabase(session_))
    //            ->setPathComponent("curationdatabase");
    //leftMenu_->addItem("NLP", new NLP())
    //        ->setPathComponent("nlp");
//    leftMenu_->addItem("Ontology", new Ontology(session_))
//            ->setPathComponent("ontology");
    //    leftMenu->addItem("Lists", new Lists())
    //            ->setPathComponent("lists");
    leftMenu_->addItem("Papers", new Papers(session_, this))
            ->setPathComponent("papers");
    //    leftMenu->addItem("Workflow", new Workflow())
    //            ->setPathComponent("workflow");
    leftMenu_->addItem("Browsers", new Browsers(session_, this))
            ->setPathComponent("browsers");
    //leftMenu_->addItem("Login/Logout", new Login(authWidget))
    //        ->setPathComponent("login");
    //leftMenu->addItem("Protein2GO", new ModAnnotator())
    //    leftMenu->addItem("MOD Annotator", new ModAnnotator())
    //            ->setPathComponent("modannotator");
    leftMenu_->addItem("Customization", customization_)
            ->setPathComponent("customization");
    leftMenu_->addItem("About Us", aboutus_)
            ->setPathComponent("aboutus");
    if (urlparameters_->IsRoot()) {
        permissions_ = new Permissions(urlparameters_, this);
        leftMenu_->addItem("Permissions", permissions_)
                ->setPathComponent("permissions");
    }
    /* keep for later use
    // Create a popup submenu for the Help menu.
    Wt::WPopupMenu * popup = new Wt::WPopupMenu();
    popup->addItem("Contents");
    popup->addItem("Index");
    popup->addSeparator();
    popup->addItem("Contact us");
    popup->addItem("About");
    popup->itemSelected().connect(std::bind([ = ] (Wt::WMenuItem * item){
        Wt::WString msgtxt("");
        if (item->text().toUTF8().compare("About") == 0)
            msgtxt= Wt::WString::fromUTF8("© 2015 Textpresso, California Institute of Technology. Build date: "
                    + std::string(__DATE__) + ", " + __TIME__);
        else
            msgtxt = Wt::WString::fromUTF8("<p>To be implemented: {1}</p>").arg(item->text());
        Wt::WMessageBox * messageBox = new Wt::WMessageBox (item->text(), msgtxt, Wt::Information, Wt::Ok);
        messageBox->textWidget()->setWordWrap(true);
        messageBox->buttonClicked().connect(std::bind([ = ] ()
        {
            delete messageBox;
        }));
        messageBox->show();
    }, std::placeholders::_1));

    Wt::WMenuItem * item = new Wt::WMenuItem("Help");
    item->setMenu(popup);
    leftMenu_->addItem(item);
     */
    // Add a Search control.
    Wt::WLineEdit *edit = new Wt::WLineEdit();
    edit->setEmptyText("Enter a search term");
    edit->enterPressed().connect(boost::bind(&TCNavWeb::SearchEntered, this, leftMenu_, edit));
    navigation_->addSearch(edit);
    //
    Wt::WVBoxLayout * layout = new Wt::WVBoxLayout();
    setLayout(layout);
    AuthEvent();
    if (urlparameters_->IsRoot()) {
        Wt::WBorder bx;
        bx.setWidth(Wt::WBorder::Thick);
        bx.setStyle(Wt::WBorder::Ridge);
        bx.setColor(Wt::red);
        decorationStyle().setBorder(bx);
    }
    layout->addWidget(navigation_);
    layout->addWidget(loganchor_, 0, Wt::AlignRight);
    layout->addWidget(new Wt::WBreak());
    layout->addWidget(verifieduserlabel, 0, Wt::AlignRight);
    layout->addWidget(contentsStack_);
    changed = true;
}

void TCNavWeb::displayLogin() {
    dialog = new Wt::WDialog("");
    dialog->contents()->addWidget(new Login(authWidget));
    dialog->contents()->addStyleClass("form-group");
    Wt::WPushButton *cancel = new Wt::WPushButton("Cancel");
    dialog->footer()->addWidget(cancel);
    dialog->rejectWhenEscapePressed();
    cancel->clicked().connect(std::bind([=]() {
        dialog->reject();
    }));
    dialog->show();
}

void TCNavWeb::AuthEvent() {
    if (dialog != nullptr) {
        dialog->reject();
    }
    if (session_.login().state() > 0 && loganchor_ != NULL) {
        loganchor_->setText(session_.login().user().identity("loginname").toUTF8());
        Wt::log("notice") << "User " + session_.login().user().identity("loginname").toUTF8() + " logged in.";
    } else {
        Wt::log("notice") << "User logged out.";
        loganchor_->setText("Login!");
    }
    if (!isUserVerified()) {
        verifieduserlabel->setText("Email address not verified! Click here to resend verification email");
    } else {
        verifieduserlabel->setText("");
    }
}

void TCNavWeb::verifyEmailAddress() {
    auto user = session_.login().user();
    string email = session_.login().user().unverifiedEmail();
    session_.auth().verifyEmailAddress(session_.login().user(), email);
    WDialog* verificationSentDialog = new Wt::WDialog("Verification Email");
    verificationSentDialog->setWidth(Wt::WLength(50, Wt::WLength::FontEx));
    Wt::WText* t1 = new Wt::WText("A new verification email has been sent.",
                                  verificationSentDialog->contents());
    t1->setWordWrap(true);
    verificationSentDialog->contents()->addWidget(new Wt::WBreak());
    verificationSentDialog->contents()->addWidget(new Wt::WBreak());
    Wt::WPushButton *ok = new Wt::WPushButton("Ok", verificationSentDialog->contents());
    ok->clicked().connect(verificationSentDialog, &Wt::WDialog::accept);
    verificationSentDialog->finished().connect(std::bind([ = ] (){
        delete verificationSentDialog;
    }));
    verificationSentDialog->show();
}

void TCNavWeb::SearchEntered(Wt::WMenu * m, Wt::WLineEdit * edit) {
    m->select(searchmenuitem_);
    search_->SimpleKeywordSearchApi(edit->text());
}

void TCNavWeb::MenuItemTriggered(Wt::WMenuItem * x) {
    if (x == curationmenuitem_)
        curation_->LoadContent();
}

void TCNavWeb::DeleteSinglePaperItem() {
    delete singlepaperitem_;
    singlepaperitem_ = NULL;
}

void TCNavWeb::SetSinglePaperItem(const PaperAddress& x) {
    singlepaperitem_ = new PaperAddress(x);
    changed = true;
}

bool TCNavWeb::paperHasChanged() {
    return changed;
}

void TCNavWeb::setNoChange() {
    changed = false;
}

/*!
 * check if the current user has a verified email address. If no user is logged in, return true
 *
 * @return whether the user has a verified email address or true if no user is logged in
 */
bool TCNavWeb::isUserVerified() {
    if (session_.login().state() != 0) {
        string uid = session_.login().user().identity("loginname").toUTF8();
        pqxx::connection cn("dbname=www-data");
        pqxx::work w(cn);
        pqxx::result r;
        std::stringstream pc;
        string tablename("auth_info");
        pc << "select * from " << tablename << " where id=(select auth_info_id from auth_identity where identity='"
           << uid << "')";
        r = w.exec(pc.str());
        for (auto record : r) {
            std::string unverifiedEmail;
            if (record["unverified_email"].to(unverifiedEmail)) {
                return unverifiedEmail.size() == 0;
            }
        }
        w.commit();
    } else {
        return true;
    }
}

/*!
 * change the cursor to a pointing cursor when mouse is over a link
 * @param w a widget representing a clickable object
 */
void TCNavWeb::SetCursorHand(Wt::WWidget * w) {
    w->decorationStyle().setCursor(Wt::PointingHandCursor);
}