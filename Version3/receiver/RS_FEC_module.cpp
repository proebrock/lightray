#include <Python.h>
#include "RS_FEC.h"

const uint8_t msg_len = 16;
const uint8_t ecc_len = 16;

static PyObject *decodeError;

/////////////////////////////////////

static char encode_docstring[] =
	"Encode message";

static PyObject *RS_FEC_encode(PyObject *self, PyObject *args)
{
	PyObject *bufobj;
	Py_buffer view;

	// Get the passed Python object
	if (!PyArg_ParseTuple(args, "O", &bufobj))
	{
		return NULL;
	}
	// Attempt to extract buffer information from it
	if (PyObject_GetBuffer(bufobj, &view,
		PyBUF_ANY_CONTIGUOUS | PyBUF_FORMAT) == -1)
	{
		return NULL;
	}
	if (view.ndim != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Expected a 1-dimensional array");
		PyBuffer_Release(&view);
		return NULL;
	}
	if (view.format != NULL)
	{
		if (strcmp(view.format,"B") != 0)
		{
			PyErr_SetString(PyExc_TypeError, "Expected an array of unsigned byte");
			PyBuffer_Release(&view);
			return NULL;
		}
	}
	if (view.len != msg_len)
	{
		PyErr_SetString(PyExc_TypeError, "Invalid buffer size");
		PyBuffer_Release(&view);
		return NULL;
	}

	char dst[msg_len + ecc_len];
	RS::ReedSolomon<msg_len, ecc_len> rs;
	rs.Encode(static_cast<const void *>(view.buf), static_cast<void *>(dst));

	PyBuffer_Release(&view);
	return PyByteArray_FromStringAndSize(dst, msg_len + ecc_len);
}

/////////////////////////////////////

static char decode_docstring[] =
	"Decode message";

static PyObject *RS_FEC_decode(PyObject *self, PyObject *args)
{
	PyObject *bufobj;
	Py_buffer view;

	// Get the passed Python object
	if (!PyArg_ParseTuple(args, "O", &bufobj))
	{
		return NULL;
	}
	// Attempt to extract buffer information from it
	if (PyObject_GetBuffer(bufobj, &view,
		PyBUF_ANY_CONTIGUOUS | PyBUF_FORMAT) == -1)
	{
		return NULL;
	}
	if (view.ndim != 1)
	{
		PyErr_SetString(PyExc_TypeError, "Expected a 1-dimensional array");
		PyBuffer_Release(&view);
		return NULL;
	}
	if (view.format != NULL)
	{
		if (strcmp(view.format,"B") != 0)
		{
			PyErr_SetString(PyExc_TypeError, "Expected an array of unsigned byte");
			PyBuffer_Release(&view);
			return NULL;
		}
	}
	if (view.len != (msg_len + ecc_len))
	{
		PyErr_SetString(PyExc_TypeError, "Invalid buffer size");
		PyBuffer_Release(&view);
		return NULL;
	}

	char dst[msg_len];
	RS::ReedSolomon<msg_len, ecc_len> rs;
	int result = rs.Decode(static_cast<const void *>(view.buf), static_cast<void *>(dst));

	if (result != 0)
	{
		PyErr_SetString(decodeError, "Decoding error");
		PyBuffer_Release(&view);
		return NULL;
	}

	PyBuffer_Release(&view);
	return PyByteArray_FromStringAndSize(dst, msg_len);
}

/////////////////////////////////////

static PyMethodDef RS_FEC_methods[] =
{
	{ "encode", RS_FEC_encode, METH_VARARGS, encode_docstring },
	{ "decode", RS_FEC_decode, METH_VARARGS, decode_docstring },
	{ NULL, NULL, 0, NULL }
};

static struct PyModuleDef RS_FEC_module = {
	PyModuleDef_HEAD_INIT,
	"RS_FEC",
	"This module provides an interface for Reed-Solomon encoding and decoding",
	-1,
	RS_FEC_methods
};

PyMODINIT_FUNC
PyInit_RS_FEC(void)
{
	PyObject *m;
    m = PyModule_Create(&RS_FEC_module);
	if (m == NULL)
		return NULL;

	decodeError = PyErr_NewExceptionWithDoc("RS_FEC.DecodeError",
		"Decoding was not possible, too many erroneous bytes", NULL, NULL);
	Py_INCREF(decodeError);
	PyModule_AddObject(m, "DecodeError", decodeError);

	return m;
}

