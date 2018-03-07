/**
    Project: textpressocentral
    File name: MyAuthService.cpp
    
    @author valerio
    @version 1.0 6/30/17.
*/

#include "MyAuthService.h"
#include "AuthSingleton.h"
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/WRandom>
#include <Wt/Auth/Token>
#include <Wt/Auth/HashFunction>
#include "AuthSingleton.h"

using namespace Wt;
using namespace Wt::Auth;

void MyAuthService::lostPassword(const std::string &emailAddress, Wt::Auth::AbstractUserDatabase &users) const {
    /*
     * This will check that a user exists in the database, and if so,
     * send an email.
     */
    User user = users.findWithEmail(emailAddress);

    if (user.isValid()) {
        std::string random = WRandom::generateId(randomTokenLength());
        std::string hash = tokenHashFunction()->compute(random, std::string());

        WDateTime expires = WDateTime::currentDateTime();
        expires = expires.addSecs(emailTokenValidity() * 60);

        Token t(hash, expires);
        user.setEmailToken(t, User::LostPassword);
        sendLostPasswordMail(emailAddress, user, random);
        AuthSingleton::getInstance().setUserValidity(true);
    } else {
        AuthSingleton::getInstance().setUserValidity(false);
    }
}
