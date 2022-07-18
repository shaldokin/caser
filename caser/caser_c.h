
// include dependencies
#include <Python.h>

// parameter
class Parameter {
  public:

    PyObject* name = nullptr;

    Parameter();
    Parameter(PyObject* name);
    virtual ~Parameter();
    virtual long int get_count();
    long int full_count();

    // generator
    PyObject* value = nullptr;
    Parameter* next = nullptr;
    bool do_this_next = true;
    bool last_round = false;
    bool gen(PyObject* output);
    virtual void _gen(PyObject*& output, bool& finished);
};

class ConstantParameter : public Parameter {
  public:
    PyObject* value;
    ConstantParameter(PyObject* name, PyObject* value);
    ~ConstantParameter();
    void _gen(PyObject*& output, bool& finished);
};

// iter
#define CASER_ITER_TYPE_TUPLE 1
#define CASER_ITER_TYPE_LIST 2
#define CASER_ITER_TYPE_OTHER 3

class IterParameter : public Parameter {
  public:

    char type;
    PyObject* iter;
    long int size;

    IterParameter(PyObject* name, PyObject* iter);
    ~IterParameter();
    long int get_count();

    // gen
    unsigned int index;
    void _gen(PyObject*& output, bool& finished);
};

// bool
class BoolParameter : public Parameter {
  public:

    BoolParameter(PyObject* name);
    long int get_count();

    // gen
    bool value;
    void _gen(PyObject*& output, bool& finished);
};

// range
template <class num_type>
class RangeParameter : public Parameter {
  public:

    long int size;
    num_type start, stop, step;
    RangeParameter(PyObject* name, PyObject* start, PyObject* stop, PyObject* step);
    long int get_count();

    // specialization
    num_type cast_from_py(PyObject* obj);
    PyObject* cast_to_py(const num_type value);

    // gen
    bool is_forward;
    num_type current;
    void _gen(PyObject*& output, bool& finished);
};

// util
#define PARAM_TYPE_CONSTANT 1
#define PARAM_TYPE_BOOL 2
#define PARAM_TYPE_ITER 3
#define PARAM_TYPE_RANGE_INT 4
#define PARAM_TYPE_RANGE_FLOAT 5
Parameter* make_param(PyObject* data);

Parameter** new_params(long int size);
