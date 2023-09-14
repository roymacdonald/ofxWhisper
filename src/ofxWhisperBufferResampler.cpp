#include "ofxWhisperBufferResampler.h"
#include "whisper.h"
#include "ofxWhisperUtils.h"

//---------------------------------------------------------------------------------------------------------
ofxWhisperBufferResampler::ofxWhisperBufferResampler():
_rms(0),
_peak(0),
_numChannels(0)
{

}

//---------------------------------------------------------------------------------------------------------
ofxWhisperBufferResampler::~ofxWhisperBufferResampler(){

}

void ofxWhisperBufferResampler::setup(int numInputChannels, int durationMs){
    ringBuffer = make_unique<LockFreeRingBuffer>(numInputChannels * WHISPER_SAMPLE_RATE * (durationMs/1000.0f));
}


//---------------------------------------------------------------------------------------------------------
ofRectangle ofxWhisperBufferResampler::draw(const ofRectangle& rect){
    
    ofSetColor(0);
    ofFill();
    ofDrawRectangle(rect);
    
    ofSetColor(ofColor::blue);
    float h = rect.height * _rms.load();
    ofDrawRectangle(rect.x, rect.height - h, rect.width, h );
    
    ofSetColor(ofColor:: yellow);
    float y = ofMap(_peak.load(), 0, 1, rect.getMaxY(), rect.getMinY());

    ofSetLineWidth(3);
    ofDrawLine(rect.getMinX(), y, rect.getMaxX(), y);
    
    ofSetColor(200);
    ofNoFill();
    ofDrawRectangle(rect);
    
    ofRectangle r = rect;
    
    stringstream ss;
    
    ss << "rms:\n " << _rms.load() << "\npeak:\n" << _peak.load();
    
    auto x = rect.x;
    y = rect.getMaxY() + 20;
    
    ofDrawBitmapStringHighlight(ss.str(), x, y);
    auto bb = bf.getBoundingBox(ss.str(), x, y);
    r.growToInclude(bb);
    return r;
}

//---------------------------------------------------------------------------------------------------------
void ofxWhisperBufferResampler::push( ofSoundBuffer& buffer){
         if(ringBuffer){
             if(!sampleRateConverter){
                 sampleRateConverter = make_unique<ofxSamplerate>() ;
             }
             
             _numChannels = buffer.getNumChannels();
             
             ofSoundBuffer converted;
             
             sampleRateConverter->changeSampleRate(buffer, converted, WHISPER_SAMPLE_RATE);
             
             ringBuffer->writeFromBuffer(converted);
             _rms = converted.getRMSAmplitude();
             _peak = ofxWhisper::getMaxValue(converted.getBuffer());
         }
     
}

//---------------------------------------------------------------------------------------------------------
bool ofxWhisperBufferResampler::hasBufferedMs(uint64_t millis){
    if(ringBuffer && _numChannels.load() > 0){
        
        return (ringBuffer->getNumReadableSamples() >= (_numChannels.load() * WHISPER_SAMPLE_RATE * millis)/1000.0f);
    }
    
    return false;
    
}

//---------------------------------------------------------------------------------------------------------
bool ofxWhisperBufferResampler::get(ofSoundBuffer& buffer){
    if( _numChannels.load() == 0) return false;
    
    if(ringBuffer){
        ringBuffer->readIntoBuffer(buffer);
        return true;
    }
    return false;
    
}


