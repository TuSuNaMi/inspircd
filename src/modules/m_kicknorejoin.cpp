/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2009 Daniel De Graaf <danieldg@inspircd.org>
 *   Copyright (C) 2008 Pippijn van Steenhoven <pip88nl@gmail.com>
 *   Copyright (C) 2007 Dennis Friis <peavey@inspircd.org>
 *   Copyright (C) 2006-2007 Robin Burchell <robin+git@viroteck.net>
 *   Copyright (C) 2006 John Brooks <john.brooks@dereferenced.net>
 *   Copyright (C) 2006 Craig Edwards <craigedwards@brainbox.cc>
 *   Copyright (C) 2006 Oliver Lupton <oliverlupton@gmail.com>
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

typedef std::map<std::string, time_t> delaylist;

struct KickRejoinData
{
	delaylist kicked;
	unsigned int delay;

	KickRejoinData(unsigned int Delay) : delay(Delay) { }
};

/** Handles channel mode +J
 */
class KickRejoin : public ParamMode<KickRejoin, SimpleExtItem<KickRejoinData> >
{
	static const unsigned int max = 60;
 public:
	KickRejoin(Module* Creator)
		: ParamMode<KickRejoin, SimpleExtItem<KickRejoinData> >(Creator, "kicknorejoin", 'J')
	{
	}

	ModeAction OnSet(User* source, Channel* channel, std::string& parameter)
	{
		int v = ConvToInt(parameter);
		if (v <= 0)
			return MODEACTION_DENY;

		if ((IS_LOCAL(source) && ((unsigned int)v > max)))
			v = max;

		ext.set(channel, new KickRejoinData(v));
		return MODEACTION_ALLOW;
	}

	void SerializeParam(Channel* chan, const KickRejoinData* krd, std::string& out)
	{
		out.append(ConvToStr(krd->delay));
	}
};

class ModuleKickNoRejoin : public Module
{
	KickRejoin kr;

public:
	ModuleKickNoRejoin()
		: kr(this)
	{
	}

	ModResult OnUserPreJoin(LocalUser* user, Channel* chan, const std::string& cname, std::string& privs, const std::string& keygiven) CXX11_OVERRIDE
	{
		if (chan)
		{
			KickRejoinData* data = kr.ext.get(chan);
			if (data)
			{
				delaylist& kicked = data->kicked;
				for (delaylist::iterator iter = kicked.begin(); iter != kicked.end(); )
				{
					if (iter->second > ServerInstance->Time())
					{
						if (iter->first == user->uuid)
						{
							user->WriteNumeric(ERR_DELAYREJOIN, "%s :You must wait %u seconds after being kicked to rejoin (+J)",
								chan->name.c_str(), data->delay);
							return MOD_RES_DENY;
						}
						++iter;
					}
					else
					{
						// Expired record, remove.
						kicked.erase(iter++);
					}
				}
			}
		}
		return MOD_RES_PASSTHRU;
	}

	void OnUserKick(User* source, Membership* memb, const std::string &reason, CUList& excepts) CXX11_OVERRIDE
	{
		if ((!IS_LOCAL(memb->user)) || (source == memb->user))
			return;

		KickRejoinData* data = kr.ext.get(memb->chan);
		if (data)
		{
			data->kicked[memb->user->uuid] = ServerInstance->Time() + data->delay;
		}
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Channel mode to delay rejoin after kick", VF_VENDOR);
	}
};

MODULE_INIT(ModuleKickNoRejoin)
