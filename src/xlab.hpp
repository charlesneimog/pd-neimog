#include <string>
#include <vector>

#include <m_pd.h>

extern "C" {
#include <m_imp.h>
#include <s_stuff.h>
}

void arrayrotate_setup(void);
void arraysum_setup(void);
void arrayappend_setup(void);

void kldivergence_setup(void);
void renyi_setup(void);
void euclidean_setup(void);
void entropy_setup(void);
void kalman_setup(void);

extern "C" void pdlua_setup(void);

// ╭─────────────────────────────────────╮
// │                UTILS                │
// ╰─────────────────────────────────────╯
void infinite0x2erecord_tilde_setup(void);

// ╭─────────────────────────────────────╮
// │           Library Objects           │
// ╰─────────────────────────────────────╯

class xlab {
  public:
    xlab() {};
    t_object Obj;

  private:
};
