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
	Module* CallInit()
	{
		PyObject *o = PyImport_ImportModule("Test");
		printf("o %p\n", o);

		PyObject *o2 = PyObject_GetAttrString(o, "Test");
		printf("o2 %p\n", o2);

		PyObject *o3 = PyObject_CallFunctionObjArgs(o2, NULL);
		printf("o3 %p\n", o3);

		Module *ptr = NULL;
		int res = SWIG_ConvertPtr(o3, (void**) &ptr, SWIG_TypeQuery("Module *"), 0);

		if (PyErr_Occurred())
			PyErr_PrintEx(0);

		return (Module*) ptr;
	}

	std::string GetVersion()
	{
		return "Python";
	}
};

class ModulePython : public Module
{
public:
	ModulePython()
	{
		Py_Initialize();

		char pathbuf[] = "path";
		PyObject *py_sys = PySys_GetObject(pathbuf);
		PyList_Append(py_sys, PyString_FromString("/home/adam/ircd/inspircd/m_python"));

		ServerInstance->Modules->Load("Test", new PythonImportManager());
	}

	~ModulePython()
	{
		Py_Finalize();
	}

	void OnRehash(User* user)
	{
	}

	Version GetVersion()
	{
		return Version("Python", VF_VENDOR);
	}
};

MODULE_INIT(ModulePython)
