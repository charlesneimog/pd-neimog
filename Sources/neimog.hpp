#include <m_pd.h>

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
// │           Library Objects           │
// ╰─────────────────────────────────────╯
class Neimog {
  public:
    Neimog(){};

    t_object Obj;

  private:
};
