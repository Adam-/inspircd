/*
 * InspIRCd -- Internet Relay Chat Daemon
 *
 *   Copyright (C) 2014 Adam <Adam@anope.org>
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


#include <Python.h>
#include "inspircd.h"
#include "swig.h"

class PythonImportManager : public ImportManager
{
	std::string name;
	PyObject *mod; /* python module */

 public:
	PythonImportManager(const std::string &n) : name(n)
	{
		mod = PyImport_ImportModule(name.c_str());

		if (PyErr_Occurred())
			PyErr_Print();
	}

	~PythonImportManager()
	{
		Py_XDECREF(mod);

		PyObject *utils = PyImport_ImportModule("inspircd_util");
		if (utils)
		{
			PyObject *unload = PyObject_GetAttrString(utils, "unload");
			if (unload)
			{
				PyObject_CallFunctionObjArgs(unload, PyString_FromString(name.c_str()));
				Py_DECREF(unload);
			}
			Py_DECREF(utils);
		}
		
		if (PyErr_Occurred())
			PyErr_Print();
	}

	Module* CallInit() CXX11_OVERRIDE
	{
		if (!mod)
			return NULL;

		PyObject *clazz = PyObject_GetAttrString(mod, name.c_str());
		if (clazz == NULL)
		{
			if (PyErr_Occurred())
				PyErr_Print();
			return NULL;
		}

		PyObject *insp_mod = PyObject_CallFunctionObjArgs(clazz, NULL);
		Py_DECREF(clazz);
		if (insp_mod == NULL)
		{
			if (PyErr_Occurred())
				PyErr_Print();
			return NULL;
		}

		Module *m = NULL;
		int res = SWIG_ConvertPtr(insp_mod, reinterpret_cast<void **>(&m), SWIG_TypeQuery("Module *"), 0);
		if (!SWIG_IsOK(res))
		{
			Py_DECREF(insp_mod);
			return NULL;
		}

		return m;
	}

	std::string GetVersion() CXX11_OVERRIDE
	{
		return "Python";
	}
};

class ModulePython : public Module
{
	std::set<std::string> paths;

	void Unload()
	{
		const ModuleManager::ModuleMap& mods = ServerInstance->Modules->GetModules();
	  	for (ModuleManager::ModuleMap::const_iterator i = mods.begin(); i != mods.end(); ++i)
		{
			Module* m = i->second;
			if (dynamic_cast<PythonImportManager *>(m->ModuleDLLManager))
				ServerInstance->Modules->Unload(m);
		}

		/* I need my modules to completely unload *now* */
		ServerInstance->AtomicActions.Run();
	}

public:
	ModulePython()
	{
		Py_Initialize();
	}

	~ModulePython()
	{
		Unload();

		Py_Finalize();
	}

	Version GetVersion() CXX11_OVERRIDE
	{
		return Version("Python", VF_VENDOR);
	}

	void ReadConfig(ConfigStatus& status) CXX11_OVERRIDE
	{
		ConfigTag* tag = ServerInstance->Config->ConfValue("python");
		std::string path = tag->getString("path");

		irc::sepstream sep(path, ';');
		for (std::string token; sep.GetToken(token);)
			if (!paths.count(token))
			{
				paths.insert(token);

				char pathbuf[] = "path";
				PyObject *py_sys = PySys_GetObject(pathbuf);
				PyList_Append(py_sys, PyString_FromString(token.c_str()));
			}

		Unload();

		ConfigTagList modules = ServerInstance->Config->ConfTags("pythonmodule");
		for (ConfigIter i = modules.first; i != modules.second; ++i)
		{
			tag = i->second;
			std::string name = tag->getString("name");

			ServerInstance->Modules->Load(name, new PythonImportManager(name));
		}
	}
};

MODULE_INIT(ModulePython)
