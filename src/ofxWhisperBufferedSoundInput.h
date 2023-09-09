#pragma once
#include "ofMain.h"

#include "ofxSamplerate.h"
#include <atomic>
#include "LockFreeRingBuffer.h"

class ofxWhisperBufferedSoundInput:
public ofBaseSoundInput {
public:
    ofxWhisperBufferedSoundInput();
    virtual ~ofxWhisperBufferedSoundInput();
       

    // get audio data from the circular buffer
    void get(int ms, std::vector<float> & audio);

    virtual void audioIn( ofSoundBuffer& buffer ) override;

    void draw(const ofRectangle& rect);
    
    bool setup(int deviceIndex , int inSampleRate, int bufferSize = 256 , ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED);
    ofSoundStream m_soundStream;
    
    
protected:
    std::unique_ptr<ofxSamplerate> sampleRateConverter = nullptr;
    std::unique_ptr<LockFreeRingBuffer> ringBuffer = nullptr;
private:
   
    std::atomic<bool> _isSetup;
    std::atomic<float> _rms;
    std::atomic<float> _peak;
};

