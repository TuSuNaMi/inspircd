/*       +------------------------------------+
 *       | Inspire Internet Relay Chat Daemon |
 *       +------------------------------------+
 *
 *  InspIRCd is copyright (C) 2002-2007 ChatSpike-Dev.
 *		       E-mail:
 *		<brain@chatspike.net>
 *		<Craig@chatspike.net>
 *
 * Written by Craig Edwards, Craig McLure, and others.
 * This program is free but copyrighted software; see
 *	    the file COPYING for details.
 *
 * ---------------------------------------------------
 */

#ifndef __CMD_USER_H__
#define __CMD_USER_H__

// include the common header files

#include "users.h"
#include "channels.h"

/** Handle /USER
 */
class cmd_user : public command_t
{
 public:
	cmd_user (InspIRCd* Instance) : command_t(Instance,"USER",0,4,true) { syntax = "<username> <localhost> <remotehost> <GECOS>"; }
	CmdResult Handle(const char** parameters, int pcnt, userrec *user);
};

#endif
