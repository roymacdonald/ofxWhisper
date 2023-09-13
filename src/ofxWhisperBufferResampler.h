#pragma once
#include "ofMain.h"

#include "ofxSamplerate.h"
#include <atomic>
#include "LockFreeRingBuffer.h"

class ofxWhisperBuffered:
{
public:
    ofxWhisperBuffered();
    virtual ~ofxWhisperBuffered();
       
    bool get(ofSoundBuffer& buffer);
    
    void push( ofSoundBuffer& buffer );

    ofRectangle draw(const ofRectangle& rect);
        
    bool hasBufferedMs(uint64_t millis);
    int getNumChannels(){return _numChannels.load();}
    
protected:
    std::unique_ptr<ofxSamplerate> sampleRateConverter = nullptr;
    std::unique_ptr<LockFreeRingBuffer> ringBuffer = nullptr;
private:
    
    
    
    ofBitmapFont bf;
    
    std::atomic<int> _numChannels;
    
    std::atomic<float> _rms;
    std::atomic<float> _peak;
};

