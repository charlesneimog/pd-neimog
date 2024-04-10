#include "follower.hpp"
#include <math.h>
#include <vector>

static t_class *FollowerObj;
struct Peak {
    unsigned int index;

    float Freq;
    float Mag;
    float Phase;

    float prevFreq;
    float prevMag;
    float prevPhase;

    bool prevPartial;

    Peak *previous;
};

typedef struct peak {
    t_float p_freq;
    t_float p_amp;
    t_float p_ampreal;
    t_float p_ampimag;
    t_float p_pit;
    t_float p_db;
    t_float p_salience;
    t_float p_tmp;
} t_peak;

typedef std::vector<Peak *> Peaks;

static unsigned int sigmund_ilog2(int n) {
    int ret = -1;
    while (n) {
        n >>= 1;
        ret++;
    }
    return (ret);
}

#define NEGBINS 4
#define ALLOCA_MAXBYTES 400000
#define BUF_ALLOCA(n) ((n) < ALLOCA_MAXBYTES ? alloca(n) : getbytes(n))
#define PEAKTHRESHFACTOR 0.6
#define W_ALPHA 0.5
#define W_BETA 0.5
#define LOG2 0.693147180559945

#define BUF_FREEA(x, n) ((((n) < ALLOCA_MAXBYTES) || (freebytes((x), (n)), 0)))
#define STEPSPEROCTAVE 48
#define PITCHSTEPS(npts) (STEPSPEROCTAVE * sigmund_ilog2((npts)))
#define PITCHNPEAK 12

const double PI = 3.14159265358979323846;

// ==============================================
static t_float sinx(t_float theta, t_float sintheta) {
    if (theta > -0.003 && theta < 0.003)
        return (1);
    else
        return (sintheta / theta);
}

#define LOGTEN 2.302585092994
static t_float sigmund_powtodb(t_float f) {
    if (f <= 0)
        return (0);
    else {
        t_float val = 100 + 10. / LOGTEN * log(f);
        return (val < 0 ? 0 : val);
    }
}

static t_float window_hann_mag(t_float pidetune, t_float sinpidetune) {
    return (W_ALPHA * sinx(pidetune, sinpidetune) -
            0.5 * W_BETA * (sinx(pidetune + PI, sinpidetune) + sinx(pidetune - PI, sinpidetune)));
}

static t_float sigmund_ftom(t_float f) { return (f > 0 ? 17.3123405046 * log(.12231220585 * f) : -1500); }

static int sigmund_cmp_freq(const void *p1, const void *p2) {
    if ((*(t_peak **)p1)->p_freq > (*(t_peak **)p2)->p_freq)
        return (1);
    else if ((*(t_peak **)p1)->p_freq < (*(t_peak **)p2)->p_freq)
        return (-1);
    else
        return (0);
}
// ==============================================
static void sigmund_tweak(int npts, t_float *ftreal, t_float *ftimag, int npeak, t_peak *peaks,
                          t_float fperbin, int loud) {
    t_peak **peakptrs = (t_peak **)alloca(sizeof(*peakptrs) * (npeak + 1));
    t_peak negpeak;
    int peaki, j, k;
    t_float ampreal[3], ampimag[3];
    t_float binperf = 1. / fperbin;
    t_float phaseperbin = (npts - 0.5) / npts, oneovern = 1. / npts;
    if (npeak < 1)
        return;
    for (peaki = 0; peaki < npeak; peaki++)
        peakptrs[peaki + 1] = &peaks[peaki];
    qsort(peakptrs + 1, npeak, sizeof(*peakptrs), sigmund_cmp_freq);
    peakptrs[0] = &negpeak;
    negpeak.p_ampreal = peakptrs[1]->p_ampreal;
    negpeak.p_ampimag = -peakptrs[1]->p_ampimag;
    negpeak.p_freq = -peakptrs[1]->p_freq;
    for (peaki = 1; peaki <= npeak; peaki++) {
        int cbin = peakptrs[peaki]->p_freq * binperf + 0.5;
        int nsub = (peaki == npeak ? 1 : 2);
        t_float windreal, windimag, windpower, detune, pidetune, sinpidetune, cospidetune, ampcorrect, ampout,
            ampoutreal, ampoutimag, freqout;
        if (cbin < 0 || cbin > 2 * npts - 3)
            continue;
        for (j = 0; j < 3; j++)
            ampreal[j] = ftreal[cbin + 2 * j - 2], ampimag[j] = ftimag[cbin + 2 * j - 2];
        for (j = 0; j < nsub; j++) {
            t_peak *neighbor = peakptrs[(peaki - 1) + 2 * j];
            t_float neighborreal = npts * neighbor->p_ampreal;
            t_float neighborimag = npts * neighbor->p_ampimag;
            for (k = 0; k < 3; k++) {
                t_float freqdiff = (0.5 * PI) * ((cbin + 2 * k - 2) - binperf * neighbor->p_freq);
                t_float sx = sinx(freqdiff, sin(freqdiff));
                t_float phasere = cos(freqdiff * phaseperbin);
                t_float phaseim = sin(freqdiff * phaseperbin);
                ampreal[k] -= sx * (phasere * neighborreal - phaseim * neighborimag);
                ampimag[k] -= sx * (phaseim * neighborreal + phasere * neighborimag);
            }
            /* post("b %f %f", ampreal[1], ampimag[1]); */
        }

        windreal = W_ALPHA * ampreal[1] - (0.5 * W_BETA) * (ampreal[0] + ampreal[2]);
        windimag = W_ALPHA * ampimag[1] - (0.5 * W_BETA) * (ampimag[0] + ampimag[2]);
        windpower = windreal * windreal + windimag * windimag;
        detune = (W_BETA * (ampreal[0] - ampreal[2]) *
                      (2.0 * W_ALPHA * ampreal[1] - W_BETA * (ampreal[0] + ampreal[2])) +
                  W_BETA * (ampimag[0] - ampimag[2]) *
                      (2.0 * W_ALPHA * ampimag[1] - W_BETA * (ampimag[0] + ampimag[2]))) /
                 (4.0 * windpower);
        if (detune > 0.5)
            detune = 0.5;
        else if (detune < -0.5)
            detune = -0.5;
        /* if (loud > 0)
            post("tweak: windpower %f, bin %d, detune %f",
                windpower, cbin, detune); */
        pidetune = PI * detune;
        sinpidetune = sin(pidetune);
        cospidetune = cos(pidetune);

        ampcorrect = 1.0 / window_hann_mag(pidetune, sinpidetune);

        ampout = oneovern * ampcorrect * sqrt(windpower);
        ampoutreal = oneovern * ampcorrect * (windreal * cospidetune - windimag * sinpidetune);
        ampoutimag = oneovern * ampcorrect * (windreal * sinpidetune + windimag * cospidetune);
        freqout = (cbin + 2 * detune) * fperbin;
        /* if (loud > 1)
            post("amp %f, freq %f", ampout, freqout); */

        peakptrs[peaki]->p_freq = freqout;
        peakptrs[peaki]->p_amp = ampout;
        peakptrs[peaki]->p_ampreal = ampoutreal;
        peakptrs[peaki]->p_ampimag = ampoutimag;
    }
}

// ==============================================
static void sigmund_getrawpeaks(int npts, t_float *insamps, int npeak, t_peak *peakv, int *nfound,
                                t_float *power, t_float srate, int loud, t_float hifreq) {
    t_float oneovern = 1.0 / (t_float)npts;
    t_float fperbin = 0.5 * srate * oneovern, totalpower = 0;
    int npts2 = 2 * npts, i, bin, bufsize = sizeof(t_float) * (2 * NEGBINS + 6 * npts);
    int peakcount = 0;
    t_float *fp1, *fp2;
    t_float *rawreal, *rawimag, *maskbuf, *powbuf;
    t_float *bigbuf = (t_float *)BUF_ALLOCA(bufsize);
    int maxbin = hifreq / fperbin;
    if (maxbin > npts - NEGBINS)
        maxbin = npts - NEGBINS;
    /* if (loud) post("tweak %d", tweak); */
    maskbuf = bigbuf + npts2;
    powbuf = maskbuf + npts;
    rawreal = powbuf + npts + NEGBINS;
    rawimag = rawreal + npts + NEGBINS;
    for (i = 0; i < npts; i++)
        maskbuf[i] = 0;

    for (i = 0; i < npts; i++)
        bigbuf[i] = insamps[i];
    for (i = npts; i < 2 * npts; i++)
        bigbuf[i] = 0;
    mayer_realfft(npts2, bigbuf);
    for (i = 0; i < npts; i++)
        rawreal[i] = bigbuf[i];
    for (i = 1; i < npts - 1; i++)
        rawimag[i] = bigbuf[npts2 - i];
    rawreal[-1] = rawreal[1];
    rawreal[-2] = rawreal[2];
    rawreal[-3] = rawreal[3];
    rawreal[-4] = rawreal[4];
    rawimag[0] = rawimag[npts - 1] = 0;
    rawimag[-1] = -rawimag[1];
    rawimag[-2] = -rawimag[2];
    rawimag[-3] = -rawimag[3];
    rawimag[-4] = -rawimag[4];
    for (i = 0, fp1 = rawreal, fp2 = rawimag; i < maxbin; i++, fp1++, fp2++) {
        t_float x1 = fp1[1] - fp1[-1], x2 = fp2[1] - fp2[-1], p = powbuf[i] = x1 * x1 + x2 * x2;
        if (i >= 2)
            totalpower += p;
    }
    powbuf[maxbin] = powbuf[maxbin + 1] = 0;
    *power = 0.5 * totalpower * oneovern * oneovern;

    for (peakcount = 0; peakcount < npeak; peakcount++) {
        t_float pow1, maxpower = 0, windreal, windimag, windpower, detune, pidetune, sinpidetune, cospidetune,
                      ampcorrect, ampout, ampoutreal, ampoutimag, freqout, powmask;
        int bestindex = -1;

        for (bin = 2, fp1 = rawreal + 2, fp2 = rawimag + 2; bin < maxbin; bin++, fp1++, fp2++) {
            pow1 = powbuf[bin];
            if (pow1 > maxpower && pow1 > maskbuf[bin]) {
                t_float thresh = PEAKTHRESHFACTOR * (powbuf[bin - 2] + powbuf[bin + 2]);
                if (pow1 > thresh)
                    maxpower = pow1, bestindex = bin;
            }
        }

        if (totalpower <= 0 || maxpower < 1e-10 * totalpower || bestindex < 0)
            break;
        fp1 = rawreal + bestindex;
        fp2 = rawimag + bestindex;
        powmask = maxpower * 1;
        int bin;
        int bin1 = (bestindex > 52 ? bestindex - 50 : 2);
        int bin2 = (maxbin < bestindex + 50 ? bestindex + 50 : maxbin);
        for (bin = bin1; bin < bin2; bin++) {
            t_float bindiff = bin - bestindex;
            t_float mymask;
            mymask = powmask / (1. + bindiff * bindiff * bindiff * bindiff);
            if (bindiff < 2 && bindiff > -2)
                mymask = 2 * maxpower;
            if (mymask > maskbuf[bin])
                maskbuf[bin] = mymask;
        }
        windreal = fp1[1] - fp1[-1];
        windimag = fp2[1] - fp2[-1];
        windpower = windreal * windreal + windimag * windimag;
        detune =
            ((fp1[1] * fp1[1] - fp1[-1] * fp1[-1]) + (fp2[1] * fp2[1] - fp2[-1] * fp2[-1])) / (2 * windpower);

        if (detune > 0.5)
            detune = 0.5;
        else if (detune < -0.5)
            detune = -0.5;
        pidetune = PI * detune;
        sinpidetune = sin(pidetune);
        cospidetune = cos(pidetune);
        ampcorrect = 1.0 / sinx(pidetune + (PI / 2), cospidetune) + sinx(pidetune - (PI / 2), -cospidetune);

        ampout = ampcorrect * sqrt(windpower);
        ampoutreal = ampcorrect * (windreal * cospidetune - windimag * sinpidetune);
        ampoutimag = ampcorrect * (windreal * sinpidetune + windimag * cospidetune);

        /* the frequency is the sum of the bin frequency and detuning */
        peakv[peakcount].p_freq = (freqout = (bestindex + 2 * detune)) * fperbin;
        peakv[peakcount].p_amp = oneovern * ampout;
        peakv[peakcount].p_ampreal = oneovern * ampoutreal;
        peakv[peakcount].p_ampimag = oneovern * ampoutimag;
    }

    sigmund_tweak(npts, rawreal, rawimag, peakcount, peakv, fperbin, loud);
    sigmund_tweak(npts, rawreal, rawimag, peakcount, peakv, fperbin, loud);

    for (i = 0; i < peakcount; i++) {
        peakv[i].p_pit = sigmund_ftom(peakv[i].p_freq);
        peakv[i].p_db = sigmund_powtodb(peakv[i].p_amp);
    }
    *nfound = peakcount;
    BUF_FREEA(bigbuf, bufsize);
}

// ==============================================
int Follower::FindPeaks() {

    int peakn = 64;
    t_peak *peakv = (t_peak *)alloca(sizeof(t_peak) * peakn);

    int nfound = 0;
    t_float freq = 0, quality = 0, evenness = 0, power = 0, note = 0;
    sigmund_getrawpeaks(1024, inBuffer.data(), peakn, peakv, &nfound, &power, sys_getsr(), 0, 20000);

    return 0;
}

// ==============================================
static void SetScore(Follower *x, t_symbol *score) {
    //
    std::string scoreFile(score->s_name);
    x->ObjScore->parseFile(scoreFile);
}

// ==============================================
t_int *Follower::Perform(t_int *w) {
    Follower *x = (Follower *)(w[1]);
    t_sample *in = (t_sample *)(w[2]);
    unsigned n = (unsigned)(w[3]);

    std::rotate(x->inBuffer.begin(), x->inBuffer.begin() + n, x->inBuffer.end());
    std::copy(in, in + n, x->inBuffer.end() - n);
    x->bufferIndex += n;
    if (x->bufferIndex == 256) {
        x->FindPeaks();
        x->bufferIndex = 0;
    }

    return (w + 4);
}

// ==============================================
static void AddDsp(Follower *x, t_signal **sp) {
    dsp_add(x->Perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
    x->bufferIndex = 0;
}

// ==============================================
static void *NewFollower(t_symbol *s, int argc, t_atom *argv) {
    Follower *x = (Follower *)pd_new(FollowerObj);
    x->EventIndex = outlet_new(&x->xObj, &s_float);
    x->Tempo = outlet_new(&x->xObj, &s_float);
    x->ObjScore = new Score();
    x->FrameSize = 1024;

    x->inBuffer.resize(x->FrameSize);
    x->peaks.resize(64);
    for (int i = 0; i < x->FrameSize; i++) {
        x->inBuffer[i] = 0;
    }

    return x;
}

// ==============================================
static void *FreeFollower(Follower *x) {
    //
    return nullptr;
}

// ==============================================
/* Setup the object */
extern "C" void follower_tilde_setup(void) {
    FollowerObj = class_new(gensym("follower~"), (t_newmethod)NewFollower, (t_method)FreeFollower,
                            sizeof(Follower), CLASS_DEFAULT, A_GIMME, 0);

    CLASS_MAINSIGNALIN(FollowerObj, Follower, xSample);

    class_addmethod(FollowerObj, (t_method)AddDsp, gensym("dsp"), A_CANT, 0);
    class_addmethod(FollowerObj, (t_method)SetScore, gensym("score"), A_SYMBOL, 0);
}
