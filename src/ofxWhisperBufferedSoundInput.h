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
//    bool get(int ms, std::vector<float> & audio);
    bool get(ofSoundBuffer& buffer);
    
    virtual void audioIn( ofSoundBuffer& buffer ) override;

    ofRectangle draw(const ofRectangle& rect);
    
    bool setup(int deviceIndex , int inSampleRate, int bufferSize = 256 ,  int waitDurationMs = 2000, ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED);
    ofSoundStream m_soundStream;
    
    
    bool hasBufferedMs(uint64_t millis);
    int getNumChannels(){return _numChannels.load();}
    
protected:
    std::unique_ptr<ofxSamplerate> sampleRateConverter = nullptr;
    std::unique_ptr<LockFreeRingBuffer> ringBuffer = nullptr;
private:
    size_t wait_duration;
    
    ofBitmapFont bf;
    
    std::atomic<int> _numChannels;
    std::atomic<bool> _isSetup;
    std::atomic<float> _rms;
    std::atomic<float> _peak;
};

