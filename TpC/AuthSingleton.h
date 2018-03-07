/**
    Project: textpressocentral
    File name: AuthSingleton.h
    
    @author valerio
    @version 1.0 6/30/17.
*/

#ifndef TEXTPRESSOCENTRAL_AUTHSINGLETON_H
#define TEXTPRESSOCENTRAL_AUTHSINGLETON_H

class AuthSingleton {
public:
    static AuthSingleton& getInstance() {
        static AuthSingleton instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }

    void setUserValidity(bool validity) {
        userValidity = validity;
    }

    bool isUserValid() const {
        return userValidity;
    }

private:
    bool userValidity;

    AuthSingleton():
            userValidity(false) { };

public:
    AuthSingleton(AuthSingleton const&) = delete;
    void operator=(AuthSingleton const&) = delete;
};

#endif //TEXTPRESSOCENTRAL_AUTHSINGLETON_H
