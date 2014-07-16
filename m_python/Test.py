import inspircd

class Test(inspircd.Module):
	def __init__(self):
		inspircd.Module.__init__(self)

	def GetVersion(self):
		return inspircd.Version("lol!?", inspircd.VF_VENDOR);

	def OnWhois(self, source, dest):
		inspircd.cvar.ServerInstance.SendWhoisLine(source, dest, 320, "%s %s :%s" % (source.nick, dest.nick, "moo"))

