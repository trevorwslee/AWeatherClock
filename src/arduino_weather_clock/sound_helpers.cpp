#include "config.h"


#if defined(ES8311_PA) // ES8311  
  #include "AudioTools.h"
  #include "AudioTools/AudioLibs/I2SCodecStream.h"
  DriverPins audioPins;
  AudioBoard audioBoard(AudioDriverES8311, audioPins);
  AudioInfo audioInfo(44100, 2, 16);
  SineWaveGenerator<int16_t> audioSineWave(32000);
  GeneratedSoundStream<int16_t> audioSound(audioSineWave);
  I2SCodecStream audioOut(audioBoard);
  StreamCopy audioCopier(audioOut, audioSound);
#endif


void playTone(int freq, int duration) {
  #if defined(ES8311_PA)  
    float cycleCount = duration / 5.5;  // around 180 cycles per second
    audioSineWave.setFrequency(freq);
    for (int i = 0; i < cycleCount; i++) {
      audioCopier.copy();
    }
  #elif defined(BUZZER_PIN)
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


  // start I2S & codec with i2c and i2s configured above
  Serial.println("starting I2S...");
  auto config = audioOut.defaultConfig();
  config.copyFrom(audioInfo);
  audioOut.begin(config);

  // Setup sine wave
  audioSineWave.begin(audioInfo/*, N_B4*/);  

  #if defined(ES8311_VOLUME)
  audioBoard.setVolume(ES8311_VOLUME);
  #endif

#elif defined(BUZZER_PIN)
  pinMode(BUZZER_PIN, OUTPUT);
#endif
}

