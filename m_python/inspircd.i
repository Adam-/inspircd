%module(directors="1") inspircd
%{

#include "inspircd.h"
#include "base.h"
#include "inspircd_config.h"
#include "cull_list.h"
#include "extensible.h"
#include "numerics.h"
#include "uid.h"
#include "users.h"
#include "channels.h"
#include "timer.h"
#include "hashcomp.h"
#include "logger.h"
#include "usermanager.h"
#include "socket.h"
#include "ctables.h"
#include "command_parse.h"
#include "mode.h"
#include "socketengine.h"
#include "snomasks.h"
#include "filelogger.h"
#include "modules.h"
#include "threadengine.h"
#include "configreader.h"
#include "inspstring.h"
#include "protocol.h"

%}

%include "std_string.i"

/*%ignore reference;*/
%ignore ServiceProvider;
%ignore ModuleRef;
%ignore ServerConfig::errstr;
%ignore LocalIntExt;
%ignore InspIRCd::NICKForced;
%ignore InspIRCd::OperQuit;
%ignore InspIRCd::GenRandom;
%ignore InspIRCd::IsNick;
%ignore InspIRCd::IsIdent;
%ignore InspIRCd::IsChannel;
%ignore InspIRCd::IsSID;
%ignore InspIRCd::Rehash;
%ignore InspIRCd::FloodQuitUser;
%ignore InspIRCd::OnCheckExemption;
%ignore classbase::cull;
%ignore LocalUser::eh;
/*%ignore cull;*/
%ignore creator;
%ignore source;
%ignore dest;
%ignore dynamic_reference_base;
/*%ignore interfacebase;*/
%ignore ConfigReader::ConfigReader;
%ignore ConfigReader::~ConfigReader;

%feature("director");
%feature("nodirector") Command::GetRouting;

%feature("director:except") {
        if($error != NULL) {
                PyErr_Print();
        }
}

%include "inspircd.h"
%include "base.h"
%include "inspircd_config.h"
%include "cull_list.h"
%include "extensible.h"
%include "numerics.h"
%include "uid.h"
%include "users.h"
%include "channels.h"
%include "timer.h"
%include "logger.h"
%include "usermanager.h"
%include "socket.h"
%include "ctables.h"
%include "command_parse.h"
%include "mode.h"
%include "socketengine.h"
%include "snomasks.h"
%include "filelogger.h"
%include "modules.h"
%include "threadengine.h"
%include "configreader.h"
%include "inspstring.h"
%include "protocol.h"

