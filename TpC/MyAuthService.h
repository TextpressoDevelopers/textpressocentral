/**
    Project: textpressocentral
    File name: MyAuthService.h
    
    @author valerio
    @version 1.0 6/29/17.
*/

#ifndef TEXTPRESSOCENTRAL_MYAUTHSERVICE_H
#define TEXTPRESSOCENTRAL_MYAUTHSERVICE_H

#include <Wt/Auth/AuthService>
#include <Wt/Auth/AbstractUserDatabase>

class MyAuthService: public Wt::Auth::AuthService {
public:
    MyAuthService(): Wt::Auth::AuthService() {};
    virtual void lostPassword(const std::string& emailAddress,
                              Wt::Auth::AbstractUserDatabase& users) const;
};


#endif //TEXTPRESSOCENTRAL_MYAUTHSERVICE_H
