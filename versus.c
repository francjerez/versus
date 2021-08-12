/*

MIT License 
Copyright (c) 2021 Franc Jerez
https://github.com/francjerez/versus

*/
#include <Python.h> 

#define POS(n) ((n > 0) * (n)) 
#define UCS(o) (PyUnicode_IS_ASCII(o) ? ((PyASCIIObject *)o + 1) : PyUnicode_DATA(o))

typedef struct _ {
	uint32_t x, y, z, _;
} node;

// Object equality comparison in sublinear time (WCS:n/block-size, AVG:1)
static _Bool eq(PyObject *a, PyObject *b) {

	PyTypeObject
	   *s = Py_TYPE(a),
	   *t = Py_TYPE(b);

	Py_ssize_t
		n = Py_SIZE(a),
		m = Py_SIZE(b);

	if (a == b) {
		return 1;
	}  
	if (n != m || s != t) {    
		return 0;
	}
	if (t == &PyUnicode_Type) {  
		return memcmp(UCS(a), UCS(b), n * PyUnicode_KIND(a)) == 0;
	} 
	if (t == &PyLong_Type && Py_ABS(n) == 1) {
		return *((PyLongObject *)a)->ob_digit == *((PyLongObject *)b)->ob_digit;
	} 
	
	return (t->tp_richcompare)(a, b, Py_EQ) == Py_True; 
}

// Deterministic sequence alignment in subquadratic space (WCS[pathological:ABC/CBA]:n*m, AVG:nlogm)
static PyObject *ab(PyObject *self, PyObject *args, _Bool u) {

	PyObject 
	   *A, **a,
	   *B, **b,
	   *O = PyList_New(0), *o;

	if (!PyArg_UnpackTuple(args, "", 2, 2, &A, &B) || !PyList_Check(A) || !PyList_Check(B)) {
		PyErr_SetString(PyExc_Exception, "Bad input type");
		return NULL;
	}
	Py_ssize_t
		n = Py_SIZE(A),
		m = Py_SIZE(B);

	if (!n || !m || n > UINT_MAX || m > UINT_MAX) {
		PyErr_SetString(PyExc_Exception, "Bad input size");	
		return NULL;
	}

	a = PySequence_Fast_ITEMS(A);
	b = PySequence_Fast_ITEMS(B);

	int8_t 
		f;

	int64_t
		d, k, i,
		r = n + m;

	uint32_t
		j = 0,
		x, y, z,
		l = (uint32_t)(m < n ? m : n),
	   *v_= calloc(r + 3, sizeof *v_), *v = &v_[m + 1],
	   *w_= calloc(r + 3, sizeof *w_), *w = &w_[m + 1],
		s = l > USHRT_MAX ? UINT_MAX : (l * l) + (l == 1) + 1;

	node
	   *c = calloc(s, sizeof *c);

	if (!c) {  
		PyErr_SetString(PyExc_Exception, "Lack of memory");
		return NULL;
	}

	// Trace CS's
	//  1986.11 Myers's edit graph (greedy algorithm)
	//  1985.01 Ukkonen's diagonal k-band (search space refinement)
	//  1976.06 Hunt's chained k-candidates (vector storage refinement)
	for (d = 0; d <= r; d++) {
		for (k = -(d - (POS(d - m) * 2)); k <= (d - (POS(d - n) * 2)); k += 2) {
			i = k;
			if (k == -d || (k != d && v[k - 1] < v[k + 1])) {
				x = v[++i];
			}
			else {
				x = v[--i] + 1;
			}
			z = 0;
			y = (uint32_t)(x - k);
			w[k] = w[i];
			while (x < n && y < m && eq(a[x], b[y])) {
				j += !z;
				c[j] = (node){++x, ++y, ++z, j - w[i]};// Top CS's (compressed)
				w[k] = j;
				if (j == UINT_MAX) {
					PyErr_WarnEx(PyExc_Warning, "Too many edges", 1);
					goto _;
				}
			}
			if (x >= n && y >= m) {
				j = w[k];// LCS end point
				goto _;
			}
			v[k] = x;
		}
	}_:;

	free(v_);
	free(w_);

	// Backtrack optimal LCS/SES 
	do {
		x = c[j].x;
		y = c[j].y;
		z = c[j].z;
		j-= c[j]._;
		if (u) {// SES
			f = 0;
			if (y < m) {// Ins

				// f = 1 -> This is an insertion (old <-> new + range)
				PyList_Insert(O, 0, o = Py_BuildValue("I, I, I, i", x, y, m - y, ++f));
				Py_DECREF(o);
			}
			if (x < n) {// Del

				// f =-1 -> This is a deletion (old + range <-> new)
				// f = 0 -> This del & next ins are a substitution (1:ins - 1:del = 0:sub)
				PyList_Insert(O, 0, o = Py_BuildValue("I, I, I, i", x, y, n - x, --f));
				Py_DECREF(o);
			}
		}
		m = y - z;
		n = x - z;
		if (!u && z) {// LCS
			PyList_Insert(O, 0, o = Py_BuildValue("I, I, I", n, m, z));
			Py_DECREF(o);
		}
	} while (z);

	free(c);
	return O;
}

static PyObject *lcs(PyObject *self, PyObject *args) { 
	return ab(self, args, 0);
}

static PyObject *ses(PyObject *self, PyObject *args) {
	return ab(self, args, 1);
}

static PyMethodDef _[] = {  
	{"lcs", lcs, METH_VARARGS, "[(old, new, old&new.ranges), ...]"},  
	{"ses", ses, METH_VARARGS, "[(old, new, -|+.range, -|+), ...]"}, 
	{NULL}
};

static struct PyModuleDef versus = {
	PyModuleDef_HEAD_INIT, "versus", "1.0.0", -1, _
};

PyMODINIT_FUNC PyInit_versus(void) {
	return PyModule_Create(&versus); 
}