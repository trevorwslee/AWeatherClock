
#include <Arduino.h>
#include "melody_helpers.h"


#include "snds/amazing_grace_melody.h"
#include "snds/happy_birthday_melody.h"

Melody Melodies[] = {
  Melody {"Amazing Grace", amazing_grace_nodenames, amazing_grace_octaves, amazing_grace_beats, amazing_grace_beatSpeed},
  Melody {"Birthday Song", happy_birthday_nodenames, happy_birthday_octaves, happy_birthday_beats, happy_birthday_beatSpeed},
};
const int NumMelodies = sizeof(Melodies) / sizeof(Melodies[0]);


// noteName: C, D, E, F, G, A, B
// halfNote: #, b
int toNoteIdx(char noteName, char halfNote) {
  int noteIdx = 0;
  switch (noteName) {
    case 'C': noteIdx = 0; break;
    case 'D': noteIdx = 2; break;
    case 'E': noteIdx = 4; break;
    case 'F': noteIdx = 5; break;
    case 'G': noteIdx = 7; break;
    case 'A': noteIdx = 9; break;
    case 'B': noteIdx = 11; break;
  }
  if (halfNote == '#') {
    noteName = noteIdx + 1; 
  } else if (halfNote == 'b') {
    noteName = noteIdx - 1;
  }
  return noteIdx;
}


// octave: can be negative
// noteIdx: 0 to 11; i.e. 12 note indexes in an octave
int getNoteFreq(int octave, int noteIdx) {
  int n = noteIdx + 12 * octave - 8;
  float freq = 440.0 * pow(2, n / 12.0);  // 440 is A
  return round(freq);
}
