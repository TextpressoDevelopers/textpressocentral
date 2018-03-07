// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef SESSION_H_
#define SESSION_H_

#include <Wt/Auth/Login>

#include <Wt/Dbo/Session>
#include <Wt/Dbo/ptr>
#include <Wt/Dbo/backend/Postgres>

#include "User.h"

namespace dbo = Wt::Dbo;

typedef Wt::Auth::Dbo::UserDatabase<AuthInfo> UserDatabase;

class Session : public dbo::Session
{
public:
  static void configureAuth();
  Session(const std::string & Db);
  ~Session();
  dbo::ptr<User> user();
  dbo::ptr<User> user(const Wt::Auth::User & user);
  Wt::Auth::AbstractUserDatabase & users();
  Wt::Auth::Login & login() { return login_; }
  static const Wt::Auth::AuthService & auth();
  static const Wt::Auth::PasswordService & passwordAuth();
private:
  dbo::backend::Postgres connection_;
  UserDatabase * users_;
  Wt::Auth::Login login_;
};

#endif // SESSION_H_
