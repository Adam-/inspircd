%module(directors="1") inspircd
%{

#define INSPIRCD_SWIG

/*#include "compat.h"*/
/*#include "typedefs.h"*/
#include "inspircd.h"
#include "base.h"
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
/*#include "snomasks.h"*/
#include "filelogger.h"
#include "modules.h"
#include "threadengine.h"
#include "configreader.h"
#include "inspstring.h"
#include "protocol.h"

%}

%include "std_string.i"
%include "std_vector.i"
/*%template(Parameters) std::vector<std::string>;*/

%ignore ServerConfig::errstr;
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
%ignore ListenSocket::iohookprov;
//%ignore ExtensionItem::ExtensibleType;
//%ignore ExtensibleType;
//%ignore ServerLimits
%ignore CullResult;
%ignore classbase::cull;

%feature("director");

%feature("director:except") {
        if ($error != NULL) {
                PyErr_Print();
        }
}

%feature("nodirector") CommandBase::GetRouting;

%include "compat.h"
%include "typedefs.h"
%include "inspircd.h"
%include "base.h"
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
/*%include "snomasks.h"*/
%include "filelogger.h"
%include "modules.h"
%include "threadengine.h"
%include "configreader.h"
%include "inspstring.h"
%include "protocol.h"

