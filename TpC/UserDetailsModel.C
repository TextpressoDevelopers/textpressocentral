/*
 * Copyright (C) 2012 Emweb bvba, Kessel-Lo, Belgium.
 *
 * See the LICENSE file for terms of use.
 */

#include "UserDetailsModel.h"
#include "User.h"
#include "Session.h"

const Wt::WFormModel::Field UserDetailsModel::affiliation = "affiliation";
const Wt::WFormModel::Field UserDetailsModel::permissions = "permissions";

UserDetailsModel::UserDetailsModel(Session & session, Wt::WObject * parent)
  : Wt::WFormModel(parent),
    session_(session)
{
  addField(affiliation, Wt::WString::tr("affiliation-info"));
  addField(permissions, Wt::WString::tr("permissions-info"));
}

void UserDetailsModel::save(const Wt::Auth::User & authUser)
{
  Wt::Dbo::ptr<User> user = session_.user(authUser);
  user.modify()->affiliation = valueText(affiliation).toUTF8();
  user.modify()->permissions = valueText(permissions).toUTF8();
}
