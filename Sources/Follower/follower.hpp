#include <algorithm>
#include <unistd.h>

#include <fstream>
#include <m_pd.h>
#include <string>
#include <thread>
#include <vector>

#include "score.hpp"

#define NHARMONICS 16

typedef struct _pitchpt {
    t_float p_weight;
    t_float p_loudness;
    t_float p_evenness;
    t_float p_unused;
} t_pitchpt;

class Follower {
  public:
    t_object xObj;
    t_sample xSample;
    float pitchHz[64];
    Score *ObjScore;
    std::thread *pitch;
    std::thread *time;
    std::vector<float> inBuffer;
    float *chromaBuffer;
    float *timeBuffer;
    struct peak {
        t_float p_freq;
        t_float p_amp;
    };
    std::vector<peak> peaks;
    float blockSize;
    unsigned FrameSize;
    unsigned bufferIndex;

    t_float x_hweights[NHARMONICS];
    t_pitchpt *x_pitchbuf;

    t_outlet *EventIndex;
    t_outlet *Tempo;

    // functions
    void SetScore(Follower *x, t_symbol *score);
    static t_int *Perform(t_int *w);
    int FindPeaks();
};
