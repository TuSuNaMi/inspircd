/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd: (C) 2002-2009 InspIRCd Development Team
 * See: http://wiki.inspircd.org/Credits
 *
 * This program is free but copyrighted software; see
 *            the file COPYING for details.
 *
 * ---------------------------------------------------
 */

/* $ModDesc: Allows global loading of a module. */

#include "inspircd.h"

/** Handle /GLOADMODULE
 */
class CommandGloadmodule : public Command
{
 public:
	CommandGloadmodule(Module* Creator) : Command(Creator,"GLOADMODULE", 1)
	{
		flags_needed = 'o'; syntax = "<modulename> [servermask]";
		TRANSLATE3(TR_TEXT, TR_TEXT, TR_END);
	}

	CmdResult Handle (const std::vector<std::string> &parameters, User *user)
	{
		std::string servername = parameters.size() > 1 ? parameters[1] : "*";

		if (InspIRCd::Match(ServerInstance->Config->ServerName, servername))
		{
			if (ServerInstance->Modules->Load(parameters[0].c_str()))
			{
				ServerInstance->SNO->WriteToSnoMask('a', "NEW MODULE '%s' GLOBALLY LOADED BY '%s'",parameters[0].c_str(), user->nick.c_str());
				user->WriteNumeric(975, "%s %s :Module successfully loaded.",user->nick.c_str(), parameters[0].c_str());
			}
			else
			{
				user->WriteNumeric(974, "%s %s :%s",user->nick.c_str(), parameters[0].c_str(), ServerInstance->Modules->LastError().c_str());
			}
		}
		else
			ServerInstance->SNO->WriteToSnoMask('a', "MODULE '%s' GLOBAL LOAD BY '%s' (not loaded here)",parameters[0].c_str(), user->nick.c_str());

		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		return ROUTE_BROADCAST;
	}
};

/** Handle /GUNLOADMODULE
 */
class CommandGunloadmodule : public Command
{
 public:
	CommandGunloadmodule(Module* Creator) : Command(Creator,"GUNLOADMODULE", 1)
	{
		flags_needed = 'o'; syntax = "<modulename> [servermask]";
	}

	CmdResult Handle (const std::vector<std::string> &parameters, User *user)
	{
		std::string servername = parameters.size() > 1 ? parameters[1] : "*";

		if (InspIRCd::Match(ServerInstance->Config->ServerName, servername))
		{
			if (ServerInstance->Modules->Unload(parameters[0].c_str()))
			{
				ServerInstance->SNO->WriteToSnoMask('a', "MODULE '%s' GLOBALLY UNLOADED BY '%s'",parameters[0].c_str(), user->nick.c_str());
				user->WriteNumeric(973, "%s %s :Module successfully unloaded.",user->nick.c_str(), parameters[0].c_str());
			}
			else
			{
				user->WriteNumeric(972, "%s %s :%s",user->nick.c_str(), parameters[0].c_str(), ServerInstance->Modules->LastError().c_str());
			}
		}
		else
			ServerInstance->SNO->WriteToSnoMask('a', "MODULE '%s' GLOBAL UNLOAD BY '%s' (not unloaded here)",parameters[0].c_str(), user->nick.c_str());

		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		return ROUTE_BROADCAST;
	}
};

/** Handle /GRELOADMODULE
 */
class CommandGreloadmodule : public Command
{
 public:
	CommandGreloadmodule(Module* Creator) : Command(Creator, "GRELOADMODULE", 1)
	{
		flags_needed = 'o'; syntax = "<modulename> [servermask]";
	}

	CmdResult Handle(const std::vector<std::string> &parameters, User *user)
	{
		std::string servername = parameters.size() > 1 ? parameters[1] : "*";

		if (InspIRCd::Match(ServerInstance->Config->ServerName, servername))
		{
			bool ok = true;
			if (!ServerInstance->Modules->Unload(parameters[0].c_str()))
			{
				ok = false;
				user->WriteNumeric(972, "%s %s :%s",user->nick.c_str(), parameters[0].c_str(), ServerInstance->Modules->LastError().c_str());
			}
			if (!ServerInstance->Modules->Load(parameters[0].c_str()))
			{
				ok = false;
				user->WriteNumeric(974, "%s %s :%s",user->nick.c_str(), parameters[0].c_str(), ServerInstance->Modules->LastError().c_str());
			}
			ServerInstance->SNO->WriteToSnoMask('a', "MODULE '%s' GLOBALLY RELOADED BY '%s'",parameters[0].c_str(), user->nick.c_str());
			if (ok)
				user->WriteNumeric(975, "%s %s :Module successfully loaded.",user->nick.c_str(), parameters[0].c_str());
		}
		else
			ServerInstance->SNO->WriteToSnoMask('a', "MODULE '%s' GLOBAL RELOAD BY '%s' (not reloaded here)",parameters[0].c_str(), user->nick.c_str());

		return CMD_SUCCESS;
	}

	RouteDescriptor GetRouting(User* user, const std::vector<std::string>& parameters)
	{
		return ROUTE_BROADCAST;
	}
};

class ModuleGlobalLoad : public Module
{
	CommandGloadmodule cmd1;
	CommandGunloadmodule cmd2;
	CommandGreloadmodule cmd3;

 public:
	ModuleGlobalLoad(InspIRCd* Me)
		: Module(Me), cmd1(this), cmd2(this), cmd3(this)
	{
		ServerInstance->AddCommand(&cmd1);
		ServerInstance->AddCommand(&cmd2);
		ServerInstance->AddCommand(&cmd3);
	}

	virtual ~ModuleGlobalLoad()
	{
	}

	virtual Version GetVersion()
	{
		return Version("Allows global loading of a module.", VF_COMMON | VF_VENDOR, API_VERSION);
	}
};

MODULE_INIT(ModuleGlobalLoad)

