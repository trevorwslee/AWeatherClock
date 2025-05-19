#ifndef MELODY_HELPERS_H
#define MELODY_HELPERS_H


struct Melody {
  const char* name;
  const char* song;
  const char* octave;
  const char* beat;
  int beatSpeed;
};


extern Melody Melodies[];
extern const int NumMelodies;



int toNoteIdx(char noteName, char halfNote);
int getNoteFreq(int octave, int noteIdx);

#endif