
# distutils: language = c++
# cython: language_level=3

# import dependencies
cdef extern from "caser_c.cpp":

    int PyTuple_SetItem(tup, int pos, obj)

    cdef int PARAM_TYPE_CONSTANT
    cdef int PARAM_TYPE_BOOL
    cdef int PARAM_TYPE_ITER
    cdef int PARAM_TYPE_RANGE_INT
    cdef int PARAM_TYPE_RANGE_FLOAT

    cdef cppclass Parameter:
        Parameter* next
        int get_count()
        int full_count()
        int gen(obj)

    Parameter** new_params(int size)
    Parameter* make_param(list data)

# grid search
cdef class GridSearch:

    cdef int param_count
    cdef Parameter* param
    cdef object on_run

    def __init__(self, dict params, on_run=None):
        self.param_count = len(params)

        cdef int index = 0
        cdef str p_key
        cdef list p_values
        cdef ParamHolder p_holder
        cdef Parameter* last
        cdef Parameter* param
        for p_key, p_value in params.items():
            if p_value.__class__ is ParamHolder:
                p_holder = p_value
                p_values = p_holder.values
                p_values[1] = p_key
            else:
                p_values = [PARAM_TYPE_CONSTANT, p_key, p_value]
            param = make_param(p_values)
            if index > 0:
                last.next = param
            else:
                self.param = param
            last = param
            index += 1

        self.on_run = on_run

    @property
    def count(self):
        return self.param.full_count()

    def combinations(self):
        cdef dict obj = {}
        cdef int check = 1
        while True:
            check = self.param.gen(obj)
            yield(obj)
            if check:
                break

    def run(self):
        if self.on_run is not None:
            for kw_args in self.combinations():
                self.on_run(**kw_args)


def grid_search(**kwargs):
    def grid_search_decor(func):
        return GridSearch(kwargs, func)
    return grid_search_decor

# parameters
cdef class ParamHolder:
    cdef list values
    def __init__(self, list values):
        self.values = values

def const(obj):
    return ParamHolder([PARAM_TYPE_CONSTANT, None, obj])

def bool():
    return ParamHolder([PARAM_TYPE_BOOL, None])

def iter(obj):
    return ParamHolder([PARAM_TYPE_ITER, None, obj])

cdef _range(type, args):

    # start
    cdef object start = 0
    cdef object stop = 0
    cdef object step = 1
    if len(args) == 0:
        raise NameError('A stop must be defined')

    # get stop
    elif len(args) == 1:
        stop = args[0]

    # get start
    elif len(args) == 2:
        start, stop = args

    # get all
    elif len(args) == 3:
        start, stop, step = args

    # too many
    else:
        raise TypeError('Too many arguments supplied')

    # finished
    return ParamHolder([type, None, start, stop, step])

def range(*args):
    return _range(PARAM_TYPE_RANGE_INT, args)

def float_range(*args):
    return _range(PARAM_TYPE_RANGE_FLOAT, args)

