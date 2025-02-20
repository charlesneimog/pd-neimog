#include "neimog.hpp"

extern "C" {
#include <s_stuff.h>
}

static t_class *neimogLib;

// ==============================================
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

    // add to the search path
    std::string libPath = neimogLib->c_externdir->s_name;
    std::string AbsPath = libPath + "/Abstractions/";
    std::string AudiosPath = libPath + "/Resources/Audios/";
    STUFF->st_searchpath = namelist_append(STUFF->st_searchpath, libPath.c_str(), 0);
    STUFF->st_searchpath = namelist_append(STUFF->st_searchpath, AbsPath.c_str(), 0);
    STUFF->st_searchpath = namelist_append(STUFF->st_searchpath, AudiosPath.c_str(), 0);

    std::string ExtPath = neimogLib->c_externdir->s_name;
    ExtPath += "/Help-Patches/";

    class_set_extern_dir(gensym(ExtPath.c_str()));

    ArrayRotateSetup();
    ArraySumSetup();
    musesampler_tilde_setup();
    ambi_tilde_setup();

    KlDivergenceSetup();
    RenyiDivergenceSetup();

    class_set_extern_dir(&s_);

    post("[pd-neimog] version %d.%d.%d", 0, 0, 1);
}
