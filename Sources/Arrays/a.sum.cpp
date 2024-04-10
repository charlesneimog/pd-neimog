#include <algorithm>
#include <m_pd.h>
#include <string>
#include <vector>

static t_class *PdArraySum;

// ─────────────────────────────────────
class ArraySum {
  public:
    t_object Obj;
    unsigned sumLast;
    std::string arrayName;
    t_outlet *Out;
};

// ─────────────────────────────────────
static void Sum(ArraySum *x) {
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

    int index = vecsize - x->sumLast;
    float sum = 0;
    for (int i = vecsize - x->sumLast; i < vecsize; i++) {
        sum += vec[i].w_float;
    }
    outlet_float(x->Out, sum);
    return;
}

// ─────────────────────────────────────
static void *ArraySumNew(t_symbol *s, t_float f) {
    ArraySum *x = (ArraySum *)pd_new(PdArraySum);
    x->arrayName = s->s_name;
    x->sumLast = f;
    x->Out = outlet_new(&x->Obj, &s_float);
    return (x);
}

// ─────────────────────────────────────
void ArraySumSetup(void) {
    PdArraySum = class_new(gensym("a.sum"), (t_newmethod)ArraySumNew, 0, sizeof(ArraySum), 0,
                           A_SYMBOL, A_FLOAT, 0);

    class_addbang(PdArraySum, (t_method)Sum);
}
