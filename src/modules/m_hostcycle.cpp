/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2013 Attila Molnar <attilamolnar@hush.com>
 *   Copyright (C) 2009-2010 Daniel De Graaf <danieldg@inspircd.org>
 *
 * This file is part of InspIRCd.  InspIRCd is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation, version 2.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include "inspircd.h"

class ModuleHostCycle : public Module
{
	/** Send fake quit/join/mode messages for host or ident cycle.
	 */
	static void DoHostCycle(User* user, const std::string& newident, const std::string& newhost, const char* quitmsg)
	{
		// GetFullHost() returns the original data at the time this function is called
		const std::string quitline = ":" + user->GetFullHost() + " QUIT :" + quitmsg;

		already_sent_t silent_id = ++LocalUser::already_sent_id;
		already_sent_t seen_id = ++LocalUser::already_sent_id;

		IncludeChanList include_chans(user->chans.begin(), user->chans.end());
		std::map<User*,bool> exceptions;

		FOREACH_MOD(OnBuildNeighborList, (user, include_chans, exceptions));

		for (std::map<User*,bool>::iterator i = exceptions.begin(); i != exceptions.end(); ++i)
		{
			LocalUser* u = IS_LOCAL(i->first);
			if (u && !u->quitting)
			{
				if (i->second)
				{
					u->already_sent = seen_id;
					u->Write(quitline);
				}
				else
				{
					u->already_sent = silent_id;
				}
			}
		}

		std::string newfullhost = user->nick + "!" + newident + "@" + newhost;

		for (IncludeChanList::const_iterator i = include_chans.begin(); i != include_chans.end(); ++i)
		{
			Membership* memb = *i;
			Channel* c = memb->chan;
			const std::string joinline = ":" + newfullhost + " JOIN " + c->name;
			std::string modeline;

			if (!memb->modes.empty())
			{
				modeline = ":" + (ServerInstance->Config->CycleHostsFromUser ? newfullhost : ServerInstance->Config->ServerName)
					+ " MODE " + c->name + " +" + memb->modes;

				for (size_t j = 0; j < memb->modes.length(); j++)
					modeline.append(" ").append(user->nick);
			}

			const UserMembList* ulist = c->GetUsers();
			for (UserMembList::const_iterator j = ulist->begin(); j != ulist->end(); ++j)
			{
				LocalUser* u = IS_LOCAL(j->first);
				if (u == NULL || u == user)
					continue;
				if (u->already_sent == silent_id)
					continue;

				if (u->already_sent != seen_id)
				{
					u->Write(quitline);
					u->already_sent = seen_id;
				}

				u->Write(joinline);
				if (!memb->modes.empty())
					u->Write(modeline);
			}
		}
	}

 public:
	void OnChangeIdent(User* user, const std::string& newident) CXX11_OVERRIDE
	{
		DoHostCycle(user, newident, user->dhost, "Changing ident");
	}

	void OnChangeHost(User* user, const std::string& newhost) CXX11_OVERRIDE
	{
		DoHostCycle(user, user->ident, newhost, "Changing host");
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Cycles users in all their channels when their host or ident changes", VF_VENDOR);
	}
};

MODULE_INIT(ModuleHostCycle)
