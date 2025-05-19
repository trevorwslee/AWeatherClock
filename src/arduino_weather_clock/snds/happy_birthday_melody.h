

/*

I asked LLM to generate this file's "happy birthday" melody.

Prompt:

At the end are the data structures for the melody "amazing grace". Can you figure out the data for the melody "happy birthday"?

IMPORTANT notes:
* Each musical note MUST be composed of exactly two characters, no more and no less.
* Hence, the data "nodenames" / "octaves" / "beats" for each musical note MUST be composed with two characters, and therefore might be padded with space " "
* For "nodenames" -- e.g "C " for C; "C#" for C sharp; and "Cb" for C flat
* For "octaves" -- e.g. "0 " for octave 0; "1 " for octave 1; "2 " for octave 2; note that it can be negative like "-1" (still TWO chars), for lower octaves

-----------------

const char* amazing_grace_nodenames = "G C E C E D C A G G C E C E D G E G E G E C G A C C A G G C E C E D C ";
const char* amazing_grace_octaves   = "0 1 1 1 1 1 1 0 0 0 1 1 1 1 1 1 1 1 1 1 1 1 0 0 1 1 0 0 0 1 1 1 1 1 1 ";
const char* amazing_grace_beats     = "2 4 1 1 4 2 4 2 4 2 4 1 1 4 2 8 2 1 1 1 1 4 2 4 1 1 1 4 2 4 1 1 4 2 8 ";

const int amazing_grace_beatSpeed = 300;


*/



/* GPT-4o results */
// const char* happy_birthday_nodenames = "G G A G C B G G A G D C G G G E C B A F F E C D C ";
// const char* happy_birthday_octaves   = "0 0 0 0 1 1 0 0 0 0 1 1 0 0 0 0 1 1 1 0 0 0 1 1 1 ";
// const char* happy_birthday_beats     = "1 1 2 2 2 4 1 1 2 2 2 4 1 1 2 2 2 2 4 1 1 2 2 2 4 ";
// const int happy_birthday_beatSpeed = 300;


/* DeepSeek R1 results */
const char* happy_birthday_nodenames = "G G A G C B G G A G D C G G G E C B A F F E C D C ";
const char* happy_birthday_octaves   = "0 0 0 0 1 0 0 0 0 0 1 1 0 0 0 1 1 0 0 0 0 0 1 1 1 ";
const char* happy_birthday_beats     = "2 1 2 2 4 4 2 1 2 2 4 4 1 1 2 2 2 1 4 2 2 2 2 2 4 ";
const int happy_birthday_beatSpeed = 300;
