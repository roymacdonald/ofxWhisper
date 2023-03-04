#pragma once
#include "ofMain.h"

#include "ofxSamplerate.h"

class ofxWhisperBufferedSoundInput: public ofBaseSoundInput {
public:
    ofxWhisperBufferedSoundInput(int length_ms);
    
    
    bool clear();


    // get audio data from the circular buffer
    void get(int ms, std::vector<float> & audio);
//protected:
        virtual void audioIn( ofSoundBuffer& buffer ) override;

private:

    ofxSamplerate sampleRateConverter;
    int m_len_ms = 0;
    int m_sample_rate = 0;

    // std::atomic_bool m_running;
    std::mutex       m_mutex;

    std::vector<float> m_audio;
    std::vector<float> m_audio_new;
    size_t             m_audio_pos = 0;
    size_t             m_audio_len = 0;
};

