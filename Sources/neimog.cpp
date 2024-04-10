#include "neimog.hpp"

static t_class *neimogLib;

static void *neimogNew(void) {
    Neimog *x = (Neimog *)pd_new(neimogLib);
    return (x);
}

// ==============================================
extern "C" void setup_pd0x2dneimog(void) {
    int major, minor, micro;
    sys_getversion(&major, &minor, &micro);
    if (major < 0 && minor < 54) {
        return;
    }

    neimogLib = class_new(gensym("pd-neimog"), (t_newmethod)neimogNew, 0, sizeof(Neimog),
                          CLASS_NOINLET, A_NULL, 0);

    ArrayRotateSetup();
    ArraySumSetup();
    musesampler_tilde_setup();
    ambi_tilde_setup();

    post("");
    post("[pd-neimog] version %d.%d.%d", 0, 0, 1);
    post("");
}
