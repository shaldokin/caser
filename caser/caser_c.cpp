
// include dependencies
#include "caser_c.h"

// parameter
Parameter::Parameter() {};

Parameter::Parameter(PyObject* name) {
  Py_XINCREF(name);
  this->name = name;
};

Parameter::~Parameter() {
  Py_XDECREF(this->name);
};

long int Parameter::get_count() {
  return 1;
};

long int Parameter::full_count() {
  long int o_count = this->get_count();
  if (this->next != nullptr)
    o_count *= this->next->full_count();
  return o_count;
}

bool Parameter::gen(PyObject* output) {
  if (this->do_this_next) {
    this->_gen(this->value, this->last_round);
    Py_INCREF(this->value);
    this->do_this_next = this->next == nullptr;
  };
  if (this->next != nullptr) {
    this->do_this_next = this->next->gen(output);
  };
  PyDict_SetItem(output, this->name, this->value);
  return this->do_this_next && this->last_round;
};

void Parameter::_gen(PyObject*& output, bool& finished) {
  output = Py_None;
  finished = true;
};

// constant
ConstantParameter::ConstantParameter(PyObject* name, PyObject* value) {
  Py_XINCREF(name);
  this->name = name;
  Py_XINCREF(value);
  this->value = value;
};

ConstantParameter::~ConstantParameter() {
  Py_XDECREF(this->name);
  Py_XDECREF(this->value);
};

void ConstantParameter::_gen(PyObject*& output, bool& finished) {
  output = this->value;
  finished = true;
};

// iter
IterParameter::IterParameter(PyObject* name, PyObject* iter) {
  Py_XINCREF(name);
  this->name = name;
  Py_XINCREF(iter);
  this->iter = iter;
  if (PyList_Check(iter)) {
    this->type = CASER_ITER_TYPE_LIST;
    this->size = PyList_Size(iter);
  } else if (PyTuple_Check(iter)) {
    this->type = CASER_ITER_TYPE_TUPLE;
    this->size = PyTuple_Size(iter);
  } else {
    this->type = CASER_ITER_TYPE_OTHER;
    this->size = PyObject_Length(iter);
  };
  this->index = 0;
};

IterParameter::~IterParameter() {
  Py_XDECREF(this->name);
  Py_XDECREF(this->iter);
};

long int IterParameter::get_count() {
  return this->size;
};

void IterParameter::_gen(PyObject*& output, bool& finished) {
  if (this->type == CASER_ITER_TYPE_TUPLE)
    output = PyTuple_GetItem(this->iter, index);
  else if (this->type == CASER_ITER_TYPE_LIST)
    output = PyList_GetItem(this->iter, index);
  else
    output = PyObject_GetItem(this->iter, PyLong_FromLong(index));
  this->index++;
  if (this->index == this->size) {
    this->index = 0;
    finished = true;
  } else
    finished = false;
};

// bool
BoolParameter::BoolParameter(PyObject* name) {
  Py_XINCREF(name);
  this->name = name;
  this->value = true;
};

long int BoolParameter::get_count() {
  return 2;
};

void BoolParameter::_gen(PyObject*& output, bool& finished) {
  if (this->value) {
    output = Py_True;
    this->value = false;
    finished = false;
  } else {
    output = Py_False;
    this->value = true;
    finished = true;
  };
};

// range
template <class num_type>
RangeParameter<num_type>::RangeParameter(PyObject* name, PyObject* start, PyObject* stop, PyObject* step) {
  Py_XINCREF(name);
  this->name = name;
  this->start = this->cast_from_py(start);
  this->stop = this->cast_from_py(stop);
  this->step = this->cast_from_py(step);
  this->size =  (this->stop - this->start) / this->step;
  this->current = this->start;
  this->is_forward = this->stop > this->start;
  if (this->is_forward)
    this->stop -= 1;
  else
    this->stop += 1;
};

template <class num_type>
long int RangeParameter<num_type>::get_count() {
  return this->size;
};

template <>
long int RangeParameter<long int>::cast_from_py(PyObject* obj) {
  return PyLong_AsLong(obj);
};
template <>
PyObject* RangeParameter<long int>::cast_to_py(const long int value) {
  return PyLong_FromLong(value);
};

template <>
double RangeParameter<double>::cast_from_py(PyObject* obj) {
  return PyFloat_AsDouble(obj);
};
template <>
PyObject* RangeParameter<double>::cast_to_py(const double value) {
  return PyFloat_FromDouble(value);
};

template <class num_type>
void RangeParameter<num_type>::_gen(PyObject*& output, bool& finished) {
  output = this->cast_to_py(this->current);
  if (this->is_forward)
    finished = this->current >= this->stop;
  else
    finished = this->current <= this->stop;
  if (finished)
    this->current = this->start;
  else
    this->current += this->step;
};

// util
Parameter** new_params(long int size) {
  return (Parameter**)malloc(size * sizeof(void*));
};

Parameter* make_param(PyObject* data) {

  // get type
  long int p_type = PyLong_AsLong(PyList_GetItem(data, 0));
  PyObject* p_name = PyList_GetItem(data, 1);

  // constant
  if (p_type == PARAM_TYPE_CONSTANT)
    return new ConstantParameter(p_name, PyList_GetItem(data, 2));

  // iter
  else if (p_type == PARAM_TYPE_ITER)
    return new IterParameter(p_name, PyList_GetItem(data, 2));

  // bool
  else if (p_type == PARAM_TYPE_BOOL)
    return new BoolParameter(p_name);

  // range
  else if (p_type == PARAM_TYPE_RANGE_INT)
    return new RangeParameter<long int>(p_name, PyList_GetItem(data, 2), PyList_GetItem(data, 3), PyList_GetItem(data, 4));

  // range-float
  else if (p_type == PARAM_TYPE_RANGE_FLOAT)
    return new RangeParameter<double>(p_name, PyList_GetItem(data, 2), PyList_GetItem(data, 3), PyList_GetItem(data, 4));

};
