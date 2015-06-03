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

#define SWIG_TYPE_TABLE "inspircd"

#include <Python.h>
#include "inspircd.h"
#include "swig.h"

/* $CompileFlags: exec("swig -python -c++ -external-runtime ../src/modules/m_python/swig.h") pkgconfincludes("python","/Python.h","") */
/* $LinkerFlags: rpath("pkg-config --libs python") pkgconflibs("python","/libpython2.7.so","-lpython2.7") */

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
				PyObject *ret = PyObject_CallFunctionObjArgs(unload, PyString_FromString(name.c_str()));
				Py_XDECREF(ret);
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

class ModulePython : public Module, public SocketEngine::IONotifier
{
	PyThreadState *state;
	std::set<std::string> paths;

	void Unload()
	{
		const ModuleManager::ModuleMap& mods = ServerInstance->Modules->GetModules();
	  	for (ModuleManager::ModuleMap::const_iterator i = mods.begin(); i != mods.end(); ++i)
		{
			Module* m = i->second;
//			if (dynamic_cast<PythonImportManager *>(m->ModuleDLLManager))
//				ServerInstance->Modules->Unload(m);
		}

		/* I need my modules to completely unload *now* */
		ServerInstance->AtomicActions.Run();
	}

public:
	ModulePython() : state(NULL)
	{
		Py_Initialize();

		PyEval_InitThreads();

		SocketEngine::notifier = this;
	}

	~ModulePython()
	{
		Unload();

		Py_Finalize();

		SocketEngine::notifier = NULL;
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

	void Unlock() CXX11_OVERRIDE
	{
		assert(state == NULL);
		state = PyEval_SaveThread();
		assert(state != NULL);
	}

	void Lock() CXX11_OVERRIDE
	{
		assert(state != NULL);
		PyEval_RestoreThread(state);
		state = NULL;
	}
};

MODULE_INIT(ModulePython)
