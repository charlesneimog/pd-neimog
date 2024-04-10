#include <fstream>
#include <m_pd.h>
#include <string>
#include <vector>

enum EVENT_TYPE {
    NOTE,
    SILENCE,
    CHORD,
};

class Event {
  public:
    float midiNote;
    float durationMs;
    EVENT_TYPE type;
};

class Score {
  public:
    float getPitch() { return pitch; }
    float getChroma() { return chroma; }
    float getEventIndex() { return index; }

    void parseFile(std::string filename);
    void setScoreFile(std::string filename) { scoreFile = filename; }

  private:
    std::string scoreFile;

    Event *Events;
    unsigned index;
    float pitch;
    float chroma;
    float time;
};
