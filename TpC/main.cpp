/* 
 * File:   main.cpp
 * Author: mueller
 *
 * Created on April 2, 2013, 2:15 PM
 */

#include "TCNavWeb.h"
#include "UrlParameters.h"
#include <Wt/WApplication> 
#include <Wt/WCssStyleSheet>
#include <Wt/WBootstrapTheme>
#include <Wt/WEnvironment>

Wt::WApplication * createApplication(const Wt::WEnvironment & env) {
    Session::configureAuth();
    Wt::WApplication * app = new Wt::WApplication(env);

// This is for reading in parameters from URL string
//    std::cerr << env.clientAddress() << std::endl;
//    if (env.getParameter("fid") != NULL)
//        std::cerr << *env.getParameter("fid") << std::endl;
//    std::cerr << env.sessionId() << std::endl;
    UrlParameters * up = new UrlParameters(env);
    if (up->IsRoot()) std::cerr << "root mode is on." << std::endl;
    // The following assumes TextpressoCentral.xml is in the webserver
    // subdirectory resources/  of the working directory
    // (but does not need to be deployed within docroot):
    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/TpCentral");
    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/authstrings");
    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/authtemplates");
    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/curation-form");
    app->messageResourceBundle().use(Wt::WApplication::appRoot() + "resources/customize-colors");
    // The following assumes TpCentral.css is deployed in the web server at the
    // same location as the application:
    // HMM: work on it later:
    Wt::WCssStyleSheet tpcss("resources/TpC.css");
    app->useStyleSheet(tpcss);
    app->setTheme(new Wt::WBootstrapTheme);
    app->setTitle("Textpresso Central");
    TCNavWeb * tcnw = new TCNavWeb(up);
    app->root()->addWidget(tcnw);
    return app;

}

int main(int argc, char **argv) {
    return Wt::WRun(argc, argv, & createApplication);

}