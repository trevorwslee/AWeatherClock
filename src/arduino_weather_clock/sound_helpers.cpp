#include "config.h"
#include "sound_helpers.h"
#include "melody_helpers.h"


struct _Beep {
  int freq;
  int durationMillis;
};

static _Beep _Beeps[] = {
  {1000, 100},
  {3000, 200},
  {1000, 100}
};
static int const _NumBeeps = sizeof(_Beeps) / sizeof(_Beeps[0]);




// struct _Melody {
//   String melodyName;
//   const char* song;
//   const char* octave;
//   const char* beat;
//   int beatSpeed;
// };


// #include "snds/amazing_grace_melody.h"
// #include "snds/happy_birthday_melody.h"

// static _Melody _Melodies[] = {
//   _Melody {"Happy Birthday", happy_birthday_nodenames, happy_birthday_octaves, happy_birthday_beats, happy_birthday_beatSpeed},
//   _Melody {"Amazing Grace", amazing_grace_nodenames, amazing_grace_octaves, amazing_grace_beats, amazing_grace_beatSpeed},
// };
// static int const _NumMelodies = sizeof(_Melodies) / sizeof(_Melodies[0]);


enum class _SoundAlarmToneType {
  Buzzer,
  ES8311
};

struct _SoundAlarmParam {
  bool (*checkStopCallback)();
  void* alarmParams;
  _SoundAlarmToneType toneType;
  void* toneParams;
};




#if defined(ES8311_PA) // ES8311  
  #include "AudioTools.h"
  #include "AudioTools/AudioLibs/I2SCodecStream.h"
  DriverPins audioPins;
  AudioBoard audioBoard(AudioDriverES8311, audioPins);
  struct _ES311SoundBeepParams {
    SineWaveGenerator<int16_t>& audioSineWave;
    StreamCopy& audioCopier;
  };
#endif

void _playToneWithBuzzer(int freq, int duration) {
  #if defined(BUZZER_PIN)
    #if defined(ESP32)
      tone(BUZZER_PIN, freq, duration);
      delay(duration);
    #else  
      long tillMicros = 1000 * (millis() + duration);
      long waitMicros = (1000000.0 / freq) / 2;
      bool high = true;
      while (true) {
        long diffMicros = tillMicros - 1000 * millis();
        if (diffMicros < waitMicros) {
          break;
        } 
        digitalWrite(BUZZER_PIN, high ? 1 : 0);
        high = !high; 
        delayMicroseconds(waitMicros);
      }  
    #endif
  #else     
    delay(duration);
  #endif    
}

void _playTone(int freq, int duration, _SoundAlarmToneType type, void* params) {
  if (type == _SoundAlarmToneType::Buzzer) {
    _playToneWithBuzzer(freq, duration);
  } else {
#if defined(ES8311_PA) // ES8311  
    _ES311SoundBeepParams* es8311Params = (_ES311SoundBeepParams*) params;
    float cycleCount = duration / 5.5;  // around 180 cycles per second
    es8311Params->audioSineWave.setFrequency(freq);
  #if defined(ES8311_VOLUME)    
    audioBoard.setVolume(ES8311_VOLUME);
  #endif    
    for (int i = 0; i < cycleCount; i++) {
      es8311Params->audioCopier.copy();
    }
#endif
  }
}

  

long _playAlarmBeep(_SoundAlarmToneType toneType, void* toneParams) {
  long usedMillis = 0;
  for (int i = 0; i < _NumBeeps; i++) {
    _Beep& beep = _Beeps[i];
    int freq = beep.freq;
    int duration = beep.durationMillis;
    _playTone(freq, duration, toneType, toneParams);
    usedMillis += beep.durationMillis;
  }
  return usedMillis;
}

void _soundAlarmBeep(_SoundAlarmParam& soundAlarmParam) {
  while (!soundAlarmParam.checkStopCallback()) {
    long usedMillis = _playAlarmBeep(soundAlarmParam.toneType, soundAlarmParam.toneParams);
    long delayMillis = 1000 - usedMillis;
    if (delayMillis > 0) {
      delay(delayMillis);
    }
  }
}

void _soundAlarmMelody(_SoundAlarmParam& soundAlarmParam) {
  //_Melody& melody = _Melodies[melodyIdx];
  Melody& melody = *(Melody*) soundAlarmParam.alarmParams;
  while (!soundAlarmParam.checkStopCallback()) {
    for (int i = 0;;) {
      if (soundAlarmParam.checkStopCallback()) {
        break;
      }

      char noteName = melody.song[i];
      char halfNote = melody.song[i + 1];
      
      if (noteName == 0) {
        // reached end of song => break out of loop
        break;
      }

      // convert the song note into tone frequency
      int noteIdx = toNoteIdx(noteName, halfNote);
      int freq = getNoteFreq(melody.octave[i] - '0', noteIdx);

      // get the how to to play the note/tone for 
      int duration = melody.beatSpeed * (melody.beat[i] - '0');

      // play the note/tone
      _playTone(freq, duration, soundAlarmParam.toneType, soundAlarmParam.toneParams);

      // increment i by 2
      i += 2;    
    }
    delay(1000);
  }
}





#if defined(ES8311_PA) // ES8311  
#include "snds/star_wars_music.h"
#include "melody_helpers.h"
bool _copyStarWars30Data(bool (*checkStopCallback)()) {
  AudioInfo info(22050, 1, 16);
  while (!checkStopCallback()) {
    //audioBoard.setVolume(0);
    I2SCodecStream out(audioBoard);
    MemoryStream music(StarWars30_raw, StarWars30_raw_len);
    StreamCopy copier(out, music); // copies sound into i2s
    auto config = out.defaultConfig();
    config.copyFrom(info);
    out.begin(config);
  #if defined(ES8311_VOLUME)    
    audioBoard.setVolume(ES8311_VOLUME);
  #endif  
    copier.begin();
    while (!checkStopCallback() && copier.copy()) {
    }
    out.end();
    delay(1000);
  }
  return true;
}
void _copyAlarmToneData(bool (*checkStopCallback)(), void* alarmParams, void (*soundAlarmFunc)(_SoundAlarmParam&)) {
  AudioInfo audioInfo(44100, 2, 16);
  SineWaveGenerator<int16_t> audioSineWave(32000);
  GeneratedSoundStream<int16_t> audioSound(audioSineWave);
  I2SCodecStream audioOut(audioBoard);
  StreamCopy audioCopier(audioOut, audioSound);
  _ES311SoundBeepParams es8311Params = {audioSineWave, audioCopier};
  auto config = audioOut.defaultConfig();
  config.copyFrom(audioInfo);
  audioOut.begin(config);
  audioSineWave.begin(audioInfo/*, N_B4*/); 
  _SoundAlarmParam soundAlarmParam = _SoundAlarmParam{checkStopCallback, alarmParams, _SoundAlarmToneType::ES8311, &es8311Params};
  soundAlarmFunc(soundAlarmParam);
  audioSineWave.end();
}
bool _copyAlarmMelodyData(bool (*checkStopCallback)(), int melodyIdx) {
  if (melodyIdx < 0 || melodyIdx >= NumMelodies) {
    return false;
  }
  _copyAlarmToneData(checkStopCallback, &Melodies[melodyIdx], _soundAlarmMelody);
  return true;
  //_soundAlarmMelody(checkStopCallback, _Melodies[melodyIdx], _SoundAlarmToneType::Buzzer, nullptr);
  //return true;

}
void _copyAlarmBeepData(bool (*checkStopCallback)()) {
  _copyAlarmToneData(checkStopCallback, nullptr, _soundAlarmBeep);
  // AudioInfo audioInfo(44100, 2, 16);
  // SineWaveGenerator<int16_t> audioSineWave(32000);
  // GeneratedSoundStream<int16_t> audioSound(audioSineWave);
  // I2SCodecStream audioOut(audioBoard);
  // StreamCopy audioCopier(audioOut, audioSound);
  // _ES311SoundBeepParams es8311Params = {audioSineWave, audioCopier};
  // auto config = audioOut.defaultConfig();
  // config.copyFrom(audioInfo);
  // audioOut.begin(config);
  // audioSineWave.begin(audioInfo/*, N_B4*/); 
  // _SoundAlarmParam soundAlarmParam = _SoundAlarmParam{checkStopCallback, nullptr, _SoundAlarmToneType::ES8311, &es8311Params};
  // _soundAlarmBeep(soundAlarmParam);
  // audioSineWave.end();
}
#endif



// #if defined(ES8311_PA) // ES8311  
// void soundAlarmWithES8311(bool (*checkStopCallback)(), AlarmPreferredType preferType, int param) {
//   if (preferType == AlarmPreferredType::Music) {
//     _copyStarWars30Data(checkStopCallback);
//   } else {
//     _copyAlarmBeepData(checkStopCallback);
//   }
// }
// #endif  

void _soundAlarmBeepWithBuzzer(bool (*checkStopCallback)()) {
  _SoundAlarmParam soundAlarmParam = _SoundAlarmParam{checkStopCallback, nullptr, _SoundAlarmToneType::Buzzer, nullptr};
  _soundAlarmBeep(soundAlarmParam);
  // while (!checkStopCallback()) {
  //   _soundAlarmBeep(soundAlarmParam);
  //   // long usedMillis = _soundAlarmBeep(_SoundAlarmToneType::Buzzer, nullptr);
  //   // long delayMillis = 1000 - usedMillis;
  //   // if (delayMillis > 0) {
  //   //   delay(delayMillis);
  //   // }
  // }
}


bool _soundAlarmMelodyWithBuzzer(bool (*checkStopCallback)(), int melodyIdx) {
  if (melodyIdx < 0 || melodyIdx >= NumMelodies) {
    return false;
  }
  _SoundAlarmParam soundAlarmParam = _SoundAlarmParam{checkStopCallback, &Melodies[melodyIdx], _SoundAlarmToneType::Buzzer, nullptr};
  _soundAlarmMelody(soundAlarmParam);
  return true;
  // if (melodyIdx < 0 || melodyIdx >= _NumMelodies) {
  //   return false;
  // }
  

  // _Melody& melody = _Melodies[melodyIdx];

  // while (!checkStopCallback()) {
  //   for (int i = 0;;) {
  //     if (checkStopCallback()) {
  //       break;
  //     }

  //     char noteName = melody.song[i];
  //     char halfNote = melody.song[i + 1];
      
  //     if (noteName == 0) {
  //       // reached end of song => break out of loop
  //       break;
  //     }

  //     // convert the song note into tone frequency
  //     int noteIdx = toNoteIdx(noteName, halfNote);
  //     int freq = getNoteFreq(melody.octave[i] - '0', noteIdx);

  //     // get the how to to play the note/tone for 
  //     int duration = melody.beatSpeed * (melody.beat[i] - '0');

  //     // play the note/tone
  //     _playTone(freq, duration, _SoundAlarmToneType::Buzzer, nullptr);
  //     //PlayTone(dumbdisplay, freq, duration, playToSpeaker);

  //     // increment i by 2
  //     i += 2;    
  //   }
  //   delay(1000);
  // }
  // return true;
}


long playAlarmBeepWithBuzzer() {
  return _playAlarmBeep(_SoundAlarmToneType::Buzzer, nullptr);
}

#if defined(USE_TASK_FOR_ALARM_SOUND)
void soundAlarm(bool (*checkStopCallback)(), AlarmPreferredType preferType, int param) {
  // to be called as a task
  #if defined(ES8311_PA) // ES8311  
  if (preferType == AlarmPreferredType::Music) {
    if (_copyStarWars30Data(checkStopCallback)) {
      return;
    }
  }
  if (preferType == AlarmPreferredType::Melody) {
    if (_copyAlarmMelodyData(checkStopCallback, param)) {
      return;
    }
  }
  _copyAlarmBeepData(checkStopCallback);
  #else 
  if (preferType == AlarmPreferredType::Melody) {
    if (_soundAlarmMelodyWithBuzzer(checkStopCallback, param)) {
      return;
    }
  }
  _soundAlarmBeepWithBuzzer(checkStopCallback);
  #endif  
}
#endif  




void soundSetup() {
#if defined(ES8311_PA)  

  AudioDriverLogger.begin(Serial,AudioDriverLogLevel::Info); 

  // add i2c codec pins: scl, sda, port, frequency
  audioPins.addI2C(PinFunction::CODEC, ES8311_I2C_SCL, ES8311_I2C_SDA);
  // add i2s pins: mclk, bck, ws,data_out, data_in ,(port)
  audioPins.addI2S(PinFunction::CODEC, ES8311_I2S_MCLK, ES8311_I2S_BCK, ES8311_I2S_WS, ES8311_I2S_DOUT, ES8311_I2S_DIN, ES8311_I2S_PORT);
  // example add other pins: PA on gpio 21
  audioPins.addPin(PinFunction::PA, ES8311_PA, PinLogic::Output);    

  // configure codec
  CodecConfig cfg;
  cfg.input_device = ADC_INPUT_LINE1;
  cfg.output_device = DAC_OUTPUT_ALL;
  cfg.i2s.bits = BIT_LENGTH_24BITS;
  cfg.i2s.rate = RATE_44K;
  // cfg.i2s.fmt = I2S_NORMAL;
  // cfg.i2s.mode = MODE_SLAVE;
  audioBoard.begin(cfg); 
#elif defined(BUZZER_PIN)
  pinMode(BUZZER_PIN, OUTPUT);
#endif
}

