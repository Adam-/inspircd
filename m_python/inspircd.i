%module(directors="1") inspircd
%{

#include "inspircd.h"

%}

%include "std_string.i"
%include "std_vector.i"
%template(Parameters) std::vector<std::string>;

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

%feature("director");

%feature("director:except") {
        if ($error != NULL) {
                PyErr_Print();
        }
}

%include "inspircd.h"

