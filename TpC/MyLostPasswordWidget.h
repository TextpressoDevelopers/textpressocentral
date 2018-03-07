/**
    Project: textpressocentral
    File name: MyLostPasswordWidget.h
    
    @author valerio
    @version 1.0 6/29/17.
*/

#ifndef TEXTPRESSOCENTRAL_MYLOSTPASSWORDWIDGET_H
#define TEXTPRESSOCENTRAL_MYLOSTPASSWORDWIDGET_H

#include <Wt/WTemplate>
#include <Wt/Auth/AbstractUserDatabase>
#include <Wt/Auth/AuthService>
#include <Wt/WContainerWidget>

/*! \class LostPasswordWidget Auth/LostPasswordWidget
 *  \brief A widget which initiates a lost-password email.
 *
 * The widget renders the <tt>"Wt.Auth.template.lost-password"</tt>
 * template. It prompts for an email address and then invokes
 * AuthService::lostPassword() with the given email address.
 *
 * \sa AuthWidget::createLostPasswordView()
 *
 * \ingroup auth
 */
class WT_API MyLostPasswordWidget : public Wt::WTemplate
{
public:
    /*! \brief Constructor
     */
    MyLostPasswordWidget(Wt::Auth::AbstractUserDatabase& users,
                       const Wt::Auth::AuthService& auth,
                       Wt::WContainerWidget *parent = 0);

protected:
    void send();
    void cancel();

private:
    Wt::Auth::AbstractUserDatabase& users_;
    const Wt::Auth::AuthService& baseAuth_;

#ifndef WT_TARGET_JAVA
    static void deleteBox(Wt::WMessageBox *box);
#else
    void deleteBox(WMessageBox *box);
#endif
};


#endif //TEXTPRESSOCENTRAL_MYLOSTPASSWORDWIDGET_H
