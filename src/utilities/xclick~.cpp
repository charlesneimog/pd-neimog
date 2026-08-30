#include "m_pd.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define BUF_MAX 1024

typedef struct _xclick {
    t_object x_obj;
    int x_nsamples;
    int x_bufsize;
    t_float *x_buffer;
    t_float x_bufini[BUF_MAX];
    int x_nleft;
    t_float *x_head;
    int x_waveform; // 0=click,1=sine,2=rect,3=triangle,4=noise
} t_xclick;

static t_class *xclick_class;

static void fill_waveform(t_xclick *x) {
    int N = x->x_bufsize;
    switch (x->x_waveform) {
    case 0: // click
        memset(x->x_buffer, 0, sizeof(float) * N);
        x->x_buffer[0] = 1.0f;
        x->x_nsamples = 1;
        break;
    case 1: // sine 440Hz
        for (int i = 0; i < N; i++)
            x->x_buffer[i] = sinf(2.0f * M_PI * 440.0f * i / 44100.0f);
        x->x_nsamples = N;
        break;
    case 2: // rect 440Hz
        for (int i = 0; i < N; i++)
            x->x_buffer[i] = (fmodf(i * 440.0f / 44100.0f, 1.0f) < 0.5f) ? 1.0f : -1.0f;
        x->x_nsamples = N;
        break;
    case 3: // triangle 440Hz
        for (int i = 0; i < N; i++) {
            float t = fmodf(i * 440.0f / 44100.0f, 1.0f);
            x->x_buffer[i] = 4.0f * fabsf(t - 0.5f) - 1.0f;
        }
        x->x_nsamples = N;
        break;
    case 4: // noise
        for (int i = 0; i < N; i++)
            x->x_buffer[i] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        x->x_nsamples = N;
        break;
    }
    x->x_nleft = 0;
    x->x_head = x->x_buffer;
}

static void xclick_bang(t_xclick *x) {
    x->x_nleft = x->x_nsamples;
    x->x_head = x->x_buffer;
}

static void xclick_setwave(t_xclick *x, t_symbol *s) {
    if (!s)
        return;
    if (strcmp(s->s_name, "-osc") == 0)
        x->x_waveform = 1;
    else if (strcmp(s->s_name, "-rect") == 0)
        x->x_waveform = 2;
    else if (strcmp(s->s_name, "-triangle") == 0)
        x->x_waveform = 3;
    else if (strcmp(s->s_name, "-noise") == 0)
        x->x_waveform = 4;
    else
        x->x_waveform = 0;
    fill_waveform(x);
}

static t_int *xclick_perform(t_int *w) {
    t_xclick *x = (t_xclick *)(w[1]);
    int nblock = (int)(w[2]);
    t_float *out = (t_float *)(w[3]);

    if (x->x_nleft) {
        t_float *head = x->x_head;
        int nleft = x->x_nleft;

        while (nblock--) {
            if (nleft > 0) {
                float amp = (float)nleft / (float)x->x_nsamples; // line from 1 -> 0
                *out++ = (*head++) * amp;
                nleft--;
                x->x_nleft--;
            } else {
                *out++ = 0.0f; // silence after buffer ends
            }
        }

        x->x_head = head;
    } else {
        while (nblock--)
            *out++ = 0.0f;
    }

    return (w + 4);
}

static void xclick_dsp(t_xclick *x, t_signal **sp) {
    dsp_add(xclick_perform, 3, x, sp[0]->s_n, sp[0]->s_vec);
}

static void *xclick_new(t_symbol *s, int ac, t_atom *av) {
    t_xclick *x = (t_xclick *)pd_new(xclick_class);
    x->x_nsamples = 1;
    x->x_bufsize = BUF_MAX;
    x->x_buffer = x->x_bufini;
    x->x_buffer[0] = 1.0f;
    x->x_nleft = 0;
    x->x_head = x->x_buffer;
    x->x_waveform = 0; // default click
    outlet_new((t_object *)x, &s_signal);

    // check for flag argument
    if (ac && av->a_type == A_SYMBOL)
        xclick_setwave(x, atom_getsymbol(av));

    fill_waveform(x);
    return x;
}

static void xclick_free(t_xclick *x) {
    if (x->x_buffer != x->x_bufini)
        freebytes(x->x_buffer, x->x_bufsize * sizeof(*x->x_buffer));
}

extern "C" void xclick_tilde_setup(void) {
    xclick_class = class_new(gensym("xclick~"), (t_newmethod)xclick_new, (t_method)xclick_free,
                             sizeof(t_xclick), 0, A_GIMME, 0);
    class_addbang(xclick_class, xclick_bang);
    class_addmethod(xclick_class, (t_method)xclick_dsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(xclick_class, (t_method)xclick_setwave, gensym("wave"), A_DEFSYM, 0);
}
