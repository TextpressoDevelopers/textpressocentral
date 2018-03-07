/* 
 * File:   Login.h
 * Author: mueller
 *
 * Created on April 4, 2013, 11:46 AM
 */

#ifndef LOGIN_H
#define	LOGIN_H

#include <Wt/WContainerWidget>
#include "AuthWidget.h"

class Login : public Wt::WContainerWidget {
public:
    Login(AuthWidget * aw, Wt::WContainerWidget * parent = NULL);
    virtual ~Login();
private:
};

#endif	/* LOGIN_H */
