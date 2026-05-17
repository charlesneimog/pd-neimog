#ifndef XLAB_SHARED
#define XLAB_SHARED

#include <m_pd.h>
#include <string.h>

// ─────────────────────────────────────
static t_float xlab_get_float_argument(int argc, t_atom *argv, const char *flag, t_float def) {
    for (int i = 0; i < argc - 1; i++) {
        if (argv[i].a_type == A_SYMBOL && strcmp(atom_getsymbol(argv + i)->s_name, flag) == 0 &&
            argv[i + 1].a_type == A_FLOAT) {
            return atom_getfloat(argv + i + 1);
        }
    }

    return def;
}

// ─────────────────────────────────────
static t_symbol *xlab_get_symbol_argument(int argc, t_atom *argv, t_symbol *flag, const char *def) {
    for (int i = 0; i < argc - 1; i++)
        if (argv[i].a_type == A_SYMBOL && atom_getsymbol(argv + i) == flag &&
            argv[i + 1].a_type == A_SYMBOL)
            return atom_getsymbol(argv + i + 1);

    return gensym(def);
}
#endif
