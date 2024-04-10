#include "score.hpp"

void Score::parseFile(std::string filename) {
    Events = new Event[1000];

    int index = 0;

    std::ifstream file(filename);

    if (!file.good()) {
        post("File not found: %s", filename.c_str());
        return;
    }

    std::string line;
    for (unsigned i = 0; std::getline(file, line); i++) {
        // split line by space
        std::string delimiter = " ";
        size_t pos = 0;
        std::string token;
        unsigned j = 0;
        Event e;
        while ((pos = line.find(delimiter)) != std::string::npos) {
            token = line.substr(0, pos);
            line.erase(0, pos + delimiter.length());
            switch (j) {
            case 0: // IDENTIFIER
                if (token == "NOTE") {
                    e.type = NOTE;
                } else if (token == "SILENCE") {
                    e.type = SILENCE;
                } else if (token == "CHORD") {
                    e.type = CHORD;
                }
                break;
            case 1: // MIDI NOTE
                e.midiNote = std::stof(token);
                break;
            case 2: // Duration
                e.durationMs = std::stof(token);
                break;
            }
            Events[i] = e;
            j++;
        }
        index++;
    }
    for (unsigned i = 0; i < index; i++) {
        post("Event %d: %f %f", i, Events[i].midiNote, Events[i].durationMs);
    }
}
