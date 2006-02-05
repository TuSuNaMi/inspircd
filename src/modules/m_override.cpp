/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd is copyright (C) 2002-2006 ChatSpike-Dev.
 *                       E-mail:
 *                <brain@chatspike.net>
 *           	  <Craig@chatspike.net>
 *     
 * Written by Craig Edwards, Craig McLure, and others.
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

using namespace std;

#include <stdio.h>
#include "users.h"
#include "channels.h"
#include "modules.h"
#include "helperfuncs.h"

/* $ModDesc: Provides support for unreal-style oper-override */

class ModuleOverride : public Module
{
	Server *Srv;
	bool NoisyOverride;
	ConfigReader *Conf;
	
 public:
 
	ModuleOverride(Server* Me)
		: Module::Module(Me)
	{
	
		// here we initialise our module. Use new to create new instances of the required
		// classes.
		
		Srv = Me;
		Conf = new ConfigReader;
		
		// read our config options (main config file)
		NoisyOverride = Conf->ReadFlag("override","noisy",0);
	}
	
	virtual void OnRehash(std::string parameter)
	{
		// on a rehash we delete our classes for good measure and create them again.
		delete Conf;
		Conf = new ConfigReader;
		// re-read our config options on a rehash
		NoisyOverride = Conf->ReadFlag("override","noisy",0);
	}

	void Implements(char* List)
	{
		List[I_OnRehash] = List[I_OnAccessCheck] = List[I_On005Numeric] = List[I_OnUserPreJoin] = List[I_OnUserPreKick] = 1;
	}

        virtual void On005Numeric(std::string &output)
        {
		output = output + std::string(" OVERRIDE");
        }

	virtual bool CanOverride(userrec* source, char* token)
	{
		// checks to see if the oper's type has <type:override>
		for (int j =0; j < Conf->Enumerate("type"); j++)
                {
			std::string typen = Conf->ReadValue("type","name",j);
			if (!strcmp(typen.c_str(),source->oper))
			{
				// its defined, return its value as a boolean for if the token is set
                        	std::string tokenlist = Conf->ReadValue("type","override",j);
				return strstr(tokenlist.c_str(),token);
                        }
                }
		// its not defined at all, count as false
		return false;
	}

	virtual int OnUserPreKick(userrec* source, userrec* user, chanrec* chan, std::string reason)
	{
		if ((*source->oper) && (CanOverride(source,"KICK")))
		{
			if (((Srv->ChanMode(source,chan) == "%") && (Srv->ChanMode(user,chan) == "@")) || (Srv->ChanMode(source,chan) == ""))
			{
				Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Kicked "+std::string(user->nick)+" on "+std::string(chan->name)+" ("+reason+")");
			}
			/* Returning -1 explicitly allows the kick */
			return -1;
		}
		return 0;
	}
	
	virtual int OnAccessCheck(userrec* source,userrec* dest,chanrec* channel,int access_type)
	{
		if (*source->oper)
		{
			if ((Srv) && (source) && (channel))
			{
				// Fix by brain - allow the change if they arent on channel - rely on boolean short-circuit
				// to not check the other items in the statement if they arent on the channel
				if ((!Srv->IsOnChannel(source,channel)) || ((Srv->ChanMode(source,channel) != "%") && (Srv->ChanMode(source,channel) != "@")))
				{
					switch (access_type)
					{
						case AC_DEOP:
							if (CanOverride(source,"MODEDEOP"))
							{
								Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Deopped "+std::string(dest->nick)+" on "+std::string(channel->name));
                                                                return ACR_ALLOW;
							}
                                                        else return ACR_DEFAULT;
						break;
						case AC_OP:
							if (CanOverride(source,"MODEOP"))
							{
								Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Opped "+std::string(dest->nick)+" on "+std::string(channel->name));
                                                                return ACR_ALLOW;
							}
                                                        else return ACR_DEFAULT;
						break;
						case AC_VOICE:
							if (CanOverride(source,"MODEVOICE"))
							{
								Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Voiced "+std::string(dest->nick)+" on "+std::string(channel->name));
                                                                return ACR_ALLOW;
							}
                                                        else return ACR_DEFAULT;
						break;
						case AC_DEVOICE:
							if (CanOverride(source,"MODEDEVOICE"))
							{
								Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Devoiced "+std::string(dest->nick)+" on "+std::string(channel->name));
                                                                return ACR_ALLOW;
							}
                                                        else return ACR_DEFAULT;
						break;
						case AC_HALFOP:
							if (CanOverride(source,"MODEHALFOP"))
							{
								Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Halfopped "+std::string(dest->nick)+" on "+std::string(channel->name));
                                                                return ACR_ALLOW;
							}
                                                        else return ACR_DEFAULT;
						break;
						case AC_DEHALFOP:
							if (CanOverride(source,"MODEDEHALFOP"))
							{
								Srv->SendOpers("*** NOTICE: "+std::string(source->nick)+" Override-Dehalfopped "+std::string(dest->nick)+" on "+std::string(channel->name));
                                                                return ACR_ALLOW;
							}
                                                        else return ACR_DEFAULT;
						break;
					}
				}
			}
			if (CanOverride(source,"OTHERMODE"))
			{
				return ACR_ALLOW;
			}
			else
			{
				return ACR_DEFAULT;
			}
		}

		return ACR_DEFAULT;
	}
	
	virtual int OnUserPreJoin(userrec* user, chanrec* chan, const char* cname)
	{
		if (strchr(user->modes,'o'))
		{
			if (chan)
			{
				if ((chan->binarymodes & CM_INVITEONLY) && (CanOverride(user,"INVITE")))
				{
					if (NoisyOverride)
					{
						irc::string x = chan->name;
						if (!user->IsInvited(x))
						{
							WriteChannelWithServ((char*)Srv->GetServerName().c_str(),chan,"NOTICE %s :%s invited himself into the channel",cname,user->nick);
						}
					}
					Srv->SendOpers("*** "+std::string(user->nick)+" used operoverride to bypass +i on "+std::string(cname));
					return -1;
				}
                                if ((chan->key[0]) && (CanOverride(user,"KEY")))
                                {
                                        if (NoisyOverride)
                                                WriteChannelWithServ((char*)Srv->GetServerName().c_str(),chan,"NOTICE %s :%s bypassed the channel key",cname,user->nick);
                                        Srv->SendOpers("*** "+std::string(user->nick)+" used operoverride to bypass +k on "+std::string(cname));
					return -1;
                                }
                                if ((chan->limit >= Srv->CountUsers(chan)) && (CanOverride(user,"LIMIT")))
                                {
                                        if (NoisyOverride)
                                                WriteChannelWithServ((char*)Srv->GetServerName().c_str(),chan,"NOTICE %s :%s passed through your channel limit",cname,user->nick);
                                        Srv->SendOpers("*** "+std::string(user->nick)+" used operoverride to bypass +l on "+std::string(cname));
					return -1;
                                }

				if (CanOverride(user,"BANWALK"))
				{
					// other join
        	                        return -1;
				}
			}
		}
		return 0;
	}
	
	virtual ~ModuleOverride()
	{
		delete Conf;
	}
	
	virtual Version GetVersion()
	{
		return Version(1,0,0,1,VF_VENDOR);
	}
};


class ModuleOverrideFactory : public ModuleFactory
{
 public:
	ModuleOverrideFactory()
	{
	}
	
	~ModuleOverrideFactory()
	{
	}
	
	virtual Module * CreateModule(Server* Me)
	{
		return new ModuleOverride(Me);
	}
	
};


extern "C" void * init_module( void )
{
	return new ModuleOverrideFactory;
}

