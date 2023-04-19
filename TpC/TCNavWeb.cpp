/* 
 * File:   TCNavWeb.cpp
 * Author: mueller
 * 
 * Created on April 2, 2013, 2:24 PM
 */

#include "displaySwitches.h"
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

namespace {

    Wt::WString textFromFile(std::string filename) {
        std::ifstream f(filename.c_str());
        std::string in("");
        std::string txt("");
        while (getline(f, in)) txt += in;
        f.close();
        return Wt::WString(txt);
    }
}

TCNavWeb::TCNavWeb(UrlParameters * urlparameters, Wt::WContainerWidget * parent) : Wt::WContainerWidget(parent),
session_("user=www-data dbname=www-data"), urlparameters_(urlparameters) {
    displaySwitches switches;
    dialog_ = nullptr;
    singlepaperitem_ = NULL;
    session_.login().changed().connect(this, & TCNavWeb::AuthEvent);
    verifieduserlabel_ = new Wt::WAnchor();
    verifieduserlabel_->decorationStyle().setForegroundColor(Wt::WColor(255, 0, 0));
    verifieduserlabel_->setMaximumSize(Wt::WLength(80, Wt::WLength::FontEx), Wt::WLength(1, Wt::WLength::FontEx));
    verifieduserlabel_->mouseWentOver().connect(boost::bind(&TCNavWeb::SetCursorHand, this, verifieduserlabel_));
    verifieduserlabel_->clicked().connect(boost::bind(&TCNavWeb::verifyEmailAddress, this));
    loganchor_ = new Wt::WText("Login!");
    loganchor_->setMaximumSize(Wt::WLength(40, Wt::WLength::FontEx),
            Wt::WLength(1, Wt::WLength::FontEx));
    loganchor_->clicked().connect(boost::bind(&TCNavWeb::displayLogin, this));
    loganchor_->decorationStyle().setForegroundColor(Wt::WColor(0, 0, 255));
    loganchor_->mouseWentOver().connect(boost::bind(&TCNavWeb::SetCursorHand, this, loganchor_));
    authWidget_ = new AuthWidget(session_);
    authWidget_->model()->addPasswordAuth(& Session::passwordAuth());
    authWidget_->setRegistrationEnabled(true);
    authWidget_->processEnvironment();
    if (boost::filesystem::exists("/tmp/tpcunstable")) {
        Wt::WMessageBox * maintenancenote = new Wt::WMessageBox("Maintenance", "Site might be unstable.", Wt::Information, Wt::Ok);
        maintenancenote->buttonClicked().connect(std::bind([ = ] (){
            delete maintenancenote;
        }));
        maintenancenote->show();
    }
    //
    navigation_ = new Wt::WNavigationBar();
    navigation_->addStyleClass("main-nav");
    navigation_->setResponsive(false);
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
    //
    home_ = new Home(session_, this);
    // Customization instantiation has to come before search instantiation;
    customization_ = new Customization(this);
    customization_->LoadContent(session_);
    search_ = new Search(urlparameters_, session_, this);
    download_ = new Download();
    aboutus_ = new AboutUs();
    curation_ = new Curation(session_, urlparameters_, this);
    browsers_ = new Browsers(this);
    papers_ = new Papers(this);
    //
    homemenuitem_ = leftMenu_->addItem("Home", home_);
    homemenuitem_->setPathComponent("home");
    home_->LoadContent(search_);
    //
    searchmenuitem_ = leftMenu_->addItem("Search", search_);
    searchmenuitem_->setPathComponent("search");
    //
    if (switches.isNotSuppressed("curation")) {
            curationmenuitem_ = leftMenu_->addItem("Curation", curation_);
            curationmenuitem_->setPathComponent("curation");
            curationmenuitem_->triggered().connect(this, &TCNavWeb::MenuItemTriggered);
        }
    //
    if (switches.isNotSuppressed("papers"))
        if (switches.isNotSuppressed("login")) {
            papersmenuitem_ = leftMenu_->addItem("Papers", papers_);
            papersmenuitem_->setPathComponent("papers");
            papersmenuitem_->triggered().connect(this, &TCNavWeb::MenuItemTriggered);
            if (Wt::WApplication::instance()->internalPath() == "/papers")
                papers_->LoadContent(session_);
        }
    //
    if (switches.isNotSuppressed("browsers")) {
        browsersmenuitem_ = leftMenu_->addItem("Browsers", browsers_);
        browsersmenuitem_->setPathComponent("browsers");
        browsersmenuitem_->triggered().connect(this, &TCNavWeb::MenuItemTriggered);
        if (Wt::WApplication::instance()->internalPath() == "/browsers")
            browsers_->LoadContent(session_, this);
    }
    //
    if (switches.isNotSuppressed("customization"))
        if (switches.isNotSuppressed("login"))
            leftMenu_->addItem("Customization", customization_)
            ->setPathComponent("customization");
    //
    if (switches.isNotSuppressed("download"))
        leftMenu_->addItem("Download", download_)
        ->setPathComponent("download");
    //
    leftMenu_->addItem("About Us", aboutus_)
            ->setPathComponent("aboutus");
    //    

    if (urlparameters_->IsRoot()) {
        permissions_ = new Permissions(urlparameters_, this);
        leftMenu_->addItem("Permissions", permissions_)
                ->setPathComponent("permissions");
    }
    help_ = new Help();
    help_->LoadContent();
    // Create a popup submenu for the Help menu.
    Wt::WPopupMenu * popup = new Wt::WPopupMenu();
    popup->addItem("Content");
    popup->addSeparator();
    popup->addItem("Contact us");
    popup->addItem("Copyright");
    popup->itemSelected().connect(std::bind([ = ] (Wt::WMenuItem * item){
        Wt::WString msgtxt("");
        if (item->text().toUTF8().compare("Content") == 0) {
            WDialog* helpDialog = new Wt::WDialog("Introduction & First Steps");
                    helpDialog->contents()->addWidget(help_);
                    Wt::WPushButton *ok = new Wt::WPushButton("Ok", helpDialog->contents());
                    ok->clicked().connect(helpDialog, &Wt::WDialog::accept);
                    helpDialog->finished().connect(std::bind([ = ] (){
                delete helpDialog;
            }));
            helpDialog->show();
        }
        if (item->text().toUTF8().compare("Contact us") == 0)
                msgtxt = textFromFile("/data/textpresso/etc/contactus.txt");
        else if (item->text().toUTF8().compare("Copyright") == 0) {
            std::string date(__DATE__);
                    std::string year(date.substr(date.length() - 4, 4));
                    msgtxt = Wt::WString::fromUTF8("© 2003 - " + year
                    + " Textpresso, California Institute of Technology. "
                    "Site was built on "
                    + date + ", " + __TIME__ + ". For more details see the "
                    "'About Us' tab in the navigation menu.");
        } //else
        //msgtxt = Wt::WString::fromUTF8("<p>To be implemented: {1}</p>").arg(item->text());
        if (!msgtxt.empty()) {
            Wt::WMessageBox * messageBox =
                    new Wt::WMessageBox(item->text(), msgtxt, Wt::Information, Wt::Ok);
                    messageBox->textWidget()->setWordWrap(true);
                    messageBox->buttonClicked().connect(std::bind([ = ] (){
                delete messageBox;
            }));
            messageBox->show();
        }
    }, std::placeholders::_1));

    Wt::WMenuItem * item = new Wt::WMenuItem("Help");
    item->setMenu(popup);
    leftMenu_->addItem(item);

    //
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
    if (switches.isNotSuppressed("login")) {

        layout->addWidget(loganchor_, 0, Wt::AlignRight);
        layout->addWidget(new Wt::WBreak());
        layout->addWidget(verifieduserlabel_, 0, Wt::AlignRight);
    }
    layout->addWidget(contentsStack_);
    changed_ = true;
}

void TCNavWeb::displayLogin() {

    dialog_ = new Wt::WDialog("Textpresso Central Authentication");
    dialog_->contents()->addWidget(new Login(authWidget_));
    dialog_->contents()->addStyleClass("form-group");
    Wt::WPushButton *cancel = new Wt::WPushButton("Cancel");
    dialog_->footer()->addWidget(cancel);
    dialog_->rejectWhenEscapePressed();
    cancel->clicked().connect(std::bind([ = ](){
        dialog_->reject();
    }));
    dialog_->show();
}

void TCNavWeb::AuthEvent() {
    if (dialog_ != nullptr) {
        dialog_->reject();
    }
    if (session_.login().state() > 0 && loganchor_ != NULL) {
        loganchor_->setText(session_.login().user().identity("loginname").toUTF8());
        Wt::log("notice") << "User " + session_.login().user().identity("loginname").toUTF8() + " logged in.";
    } else {
        Wt::log("notice") << "User logged out.";
        loganchor_->setText("Login!");
    }
    if (!isUserVerified()) {
        verifieduserlabel_->setText("Email address not verified! Click here to resend verification email");
    } else {

        verifieduserlabel_->setText("");
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
    else if (x == papersmenuitem_)
        papers_->LoadContent(session_);
    else

        if (x == browsersmenuitem_)
        browsers_->LoadContent(session_, this);
}

void TCNavWeb::DeleteSinglePaperItem() {

    delete singlepaperitem_;
    singlepaperitem_ = NULL;
}

void TCNavWeb::SetSinglePaperItem(const PaperAddress& x) {

    singlepaperitem_ = new PaperAddress(x);
    changed_ = true;
}

bool TCNavWeb::paperHasChanged() {

    return changed_;
}

void TCNavWeb::setNoChange() {

    changed_ = false;
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