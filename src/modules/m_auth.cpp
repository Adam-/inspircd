#include "inspircd.h"
#include "account.h"
#include "ssl.h"

class AuthEventHandler : public EventHandler
{
	LocalUser *lu;

 public:
	AuthEventHandler(LocalUser *l) : lu(l) { }

	~AuthEventHandler()
	{
		close(GetFd());
	}

	void HandleEvent(EventType et, int errornum)
	{
		char buf[512];
		int i = recv(GetFd(), buf, sizeof(buf), 0);
		if (i <= 0)
		{
			ServerInstance->SE->DelFd(this);
			delete this;
			return;
		}

		std::string str(buf, i - 1);

		lu->WriteServ(str);
		if (!str.find("notice "))
			lu->WriteServ("NOTICE %s :*** %s", lu->nick.c_str(), str.substr(7).c_str());
		else if (!str.find("login "))
		{
			AccountExtItem* accountext = GetAccountExtItem();
			if (accountext)
				accountext->set(lu, str.substr(6));
		}
		else if (!str.find("error "))
			ServerInstance->Users->QuitUser(lu, str.substr(6));
		else if (!str.find("raw "))
			lu->SendText(str.substr(4));
	}
};

class ModuleAuth : public Module
{
	SimpleExtItem<EventHandler> ext;

	std::string program;

	EventHandler *StartWorker(LocalUser *user)
	{
		int fds[2];

    		if (socketpair(AF_UNIX, SOCK_STREAM, 0, fds))
			return NULL;

		pid_t i = fork();
		if (i == -1)
		{
			close(fds[0]);
			close(fds[1]);
			return NULL;
		}

		if (i)
		{
			/* parent */
			close(fds[1]);
			EventHandler *e = new AuthEventHandler(user);
			e->SetFd(fds[0]);
			return e;
		}

		close(fds[0]);

		/* child */
		dup2(fds[1], 0);
		dup2(fds[1], 1);
		dup2(fds[1], 2);

		/* close existing fds */
		for (int i = 3; i < ServerInstance->SE->GetMaxFds(); ++i)
			close(i);

		char *args[4];
		args[0] = (char *) program.c_str();
		args[1] = (char *) user->GetIPString();
		SocketCertificateRequest req(&user->eh, this);
		args[2] = req.cert ? (char *) req.cert->fingerprint.c_str() : (char *) "<none>";
		args[3] = NULL;

		execv(program.c_str(), args);
		_exit(0);
	}

 public:
 	ModuleAuth() : ext("eventhandlers", this)
 	{
		program = "/tmp/lol";
 	}

 	void init()
 	{
		ServerInstance->Modules->AddService(ext);

 		Implementation eventlist[] = { I_OnUserRegister, I_OnCheckReady };
 		ServerInstance->Modules->Attach(eventlist, this, sizeof(eventlist) / sizeof(Implementation));

		ServerInstance->Modules->SetPriority(this, I_OnUserRegister, PRIORITY_LAST);

 		OnRehash(NULL);
 	}

 	Version GetVersion()
 	{
 		return Version("", VF_NONE);
 	}

 	void OnRehash(User *user)
 	{
#if 0
 		ConfigTag *tag = ServerInstance->Config->ConfValue("captcha");
 		key = tag->getString("key");
 		message = tag->getString("message");

 		irc::spacesepstream sep(tag->getString("restricted"));
 		commands.clear();
 		for (std::string token; sep.GetToken(token);)
 			commands.push_back(token);

 		irc::spacesepstream sep2(tag->getString("whitelist"));
 		whitelist.clear();
 		for (std::string token; sep2.GetToken(token);)
 			whitelist.push_back(token);
#endif
 	}

	ModResult OnUserRegister(LocalUser *user)
	{
		EventHandler *e = StartWorker(user);
		ext.set(user, e);
		ServerInstance->SE->AddFd(e, EVENT_READ);
		return MOD_RES_PASSTHRU;
	}

	ModResult OnCheckReady(LocalUser *user)
	{
		return ext.get(user) ? MOD_RES_DENY : MOD_RES_PASSTHRU;
	}
};

MODULE_INIT(ModuleAuth)
