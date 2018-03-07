// This may look like C code, but it's really -*- C++ -*-
/*
 * Copyright (C) 2009 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */
#ifndef USER_H_
#define USER_H_

#include <Wt/Dbo/Types>
#include <Wt/WDate>
#include <Wt/Dbo/WtSqlTraits>
#include <Wt/WGlobal>

namespace dbo = Wt::Dbo;

class User;
typedef Wt::Auth::Dbo::AuthInfo<User> AuthInfo;

class User {
public:
  /* You probably want to add other user information here, e.g. */
  std::string affiliation;
  std::string permissions;

  template<class Action>
  void persist(Action & a)
  {
    dbo::field(a, affiliation, "affiliation");
    dbo::field(a, permissions, "permissions");
  }
};


DBO_EXTERN_TEMPLATES(User);

#endif // USER_H_
