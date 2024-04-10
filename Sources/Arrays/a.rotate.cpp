#include <algorithm>
#include <m_pd.h>
#include <string>
#include <vector>

static t_class *PdArrayRotate;

// ─────────────────────────────────────
class ArrayRotate {
  public:
    t_object Obj;
    unsigned redrawAt;
    unsigned redrawCount;
    std::string arrayName;
};

static void ArrayRedraw(ArrayRotate *x, t_float f) {
    x->redrawAt = f;
    return;
}

// ─────────────────────────────────────
static void Rotate(ArrayRotate *x, t_symbol *s, int argc, t_atom *argv) {

    for (int i = 0; i < argc; i++) {
        if (argv[i].a_type != A_FLOAT) {
            pd_error(x, "All arguments must be floats");
            return;
        }
    }

    t_garray *array;
    int vecsize;
    t_word *vec;

    t_symbol *pd_symbol = gensym(x->arrayName.c_str());
    if (!(array = (t_garray *)pd_findbyclass(pd_symbol, garray_class))) {
        pd_error(x, "[Python] Array %s not found.", x->arrayName.c_str());
        return;
    } else if (!garray_getfloatwords(array, &vecsize, &vec)) {
        pd_error(x, "[Python] Bad template for tabwrite '%s'.", x->arrayName.c_str());
        return;
    }

    std::vector<float> inBuffer;
    for (int i = 0; i < vecsize; i++) {
        inBuffer.push_back(vec[i].w_float);
    }
    std::rotate(inBuffer.begin(), inBuffer.begin() + argc, inBuffer.end());

    for (int i = 0; i < vecsize; i++) {
        vec[i].w_float = inBuffer[i];
    }

    for (int i = 0; i < argc; i++) {
        int index = vecsize - argc + i;
        vec[index].w_float = atom_getfloat(argv + i);
    }

    x->redrawCount++;
    if (x->redrawCount >= x->redrawAt) {
        garray_redraw(array);
        x->redrawCount = 0;
    }
    return;
}

// ─────────────────────────────────────
static void *ArrayRotateNew(t_symbol *s) {
    ArrayRotate *x = (ArrayRotate *)pd_new(PdArrayRotate);
    x->arrayName = s->s_name;
    return (x);
}

// ─────────────────────────────────────
void ArrayRotateSetup(void) {
    PdArrayRotate = class_new(gensym("a.rotate"), (t_newmethod)ArrayRotateNew, 0,
                              sizeof(ArrayRotate), 0, A_SYMBOL, 0);

    class_addlist(PdArrayRotate, (t_method)Rotate);
    class_addmethod(PdArrayRotate, (t_method)ArrayRedraw, gensym("redraw"), A_FLOAT, 0);
}
