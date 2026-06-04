#include <string>
#include <vector>

#include <m_pd.h>

extern "C" {
#include <m_imp.h>
#include <s_stuff.h>
}

extern "C" void pdlua_setup(void);

// ╭─────────────────────────────────────╮
// │           Library Objects           │
// ╰─────────────────────────────────────╯

class xlab {
  public:
    xlab() {};
    t_object Obj;

  private:
};
