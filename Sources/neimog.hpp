#include <m_pd.h>

#include <m_imp.h>

#include <string>

// ╭─────────────────────────────────────╮
// │            Array Objects            │
// ╰─────────────────────────────────────╯
void ArrayRotateSetup(void);
void ArraySumSetup(void);

// ╭─────────────────────────────────────╮
// │               Samples               │
// ╰─────────────────────────────────────╯
extern "C" void musesampler_tilde_setup(void);

// ╭─────────────────────────────────────╮
// │           Espacialização            │
// ╰─────────────────────────────────────╯
extern "C" void ambi_tilde_setup(void);

// ╭─────────────────────────────────────╮
// │          Partial Tracking           │
// ╰─────────────────────────────────────╯

// ╭─────────────────────────────────────╮
// │              Follower               │
// ╰─────────────────────────────────────╯

// ╭─────────────────────────────────────╮
// │  Information Theory and Statistics  │
// ╰─────────────────────────────────────╯

void KlDivergenceSetup(void);
void RenyiDivergenceSetup(void);

// ╭─────────────────────────────────────╮
// │           Library Objects           │
// ╰─────────────────────────────────────╯

class Neimog {
  public:
    Neimog(){};

    t_object Obj;

  private:
};
