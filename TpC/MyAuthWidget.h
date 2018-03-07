/* 
 * File:   MyAuthWidget.h
 * Author: mueller
 *
 * Created on April 12, 2016, 7:17 PM
 */

#ifndef MYAUTHWIDGET_H
#define	MYAUTHWIDGET_H

#include <Wt/Auth/AuthWidget>
#include "Wt/WWidget"
#include "MyLostPasswordWidget.h"

class Session;
class MyAuthWidget : public Wt::Auth::AuthWidget {
public:
    MyAuthWidget(Session & session);
    Wt::Signal<void> & LoggedInViewSignal() {
        return loggedinview_;
    }
    virtual Wt::WWidget* createLostPasswordView();
protected:
    virtual void createLoggedInView();
private:
    Session & session_;
    Wt::Signal<void> loggedinview_;
};

#endif	/* MYAUTHWIDGET_H */

