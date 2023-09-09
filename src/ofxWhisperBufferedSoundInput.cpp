#include "ofxWhisperBufferedSoundInput.h"
#include "whisper.h"

ofxWhisperBufferedSoundInput::ofxWhisperBufferedSoundInput():
_isSetup(0),
_rms(0),
_peak(0)
{

}

ofxWhisperBufferedSoundInput::~ofxWhisperBufferedSoundInput(){
    m_soundStream.stop();
    m_soundStream.close();
}

template<typename T>
static T getMaxValue(const std::vector<T>& values){
    T mx = 0;
    for(const auto & v: values){
        if((v > mx) || (v*-1) > mx){
            mx = (v > 0)?v:(v*-1);
        }
    }
    return mx;
}

void ofxWhisperBufferedSoundInput::draw(const ofRectangle& rect){
    
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
    
    stringstream ss;
    
    ss << "rms:\n " << _rms.load() << "\npeak:\n" << _peak.load();
    
    ofDrawBitmapStringHighlight(ss.str(), rect.x, rect.getMaxX() + 20);
    
}

//void ofxWhisperBufferedSoundInput::callback(uint8_t * stream, int len) {
void ofxWhisperBufferedSoundInput::audioIn( ofSoundBuffer& buffer ) {
    if(_isSetup.load()){
         if(ringBuffer){
             if(!sampleRateConverter){
                 sampleRateConverter = make_unique<ofxSamplerate>() ;
             }
             
             ofSoundBuffer converted;
             
             sampleRateConverter->changeSampleRate(buffer, converted, WHISPER_SAMPLE_RATE);
             
             ringBuffer->writeFromBuffer(converted);
             _rms = converted.getRMSAmplitude();
             _peak = getMaxValue(converted.getBuffer());
         }
     }
 }


void ofxWhisperBufferedSoundInput::get(int ms, std::vector<float> & result) {
    if(!_isSetup.load() ) return;
    
    size_t n_samples = WHISPER_SAMPLE_RATE * ms / 1000.0f;

    result.resize(n_samples);

    if(ringBuffer){
        ringBuffer->readIntoVector(result);
    }
}

bool ofxWhisperBufferedSoundInput::setup(int deviceIndex , int inSampleRate, int bufferSize, ofSoundDevice::Api api){
    
    auto devices = m_soundStream.getDeviceList(api);
        if ( deviceIndex < devices.size()) {
            int i = deviceIndex;
            ofSoundStreamSettings m_soundSettings;
            
            m_soundSettings.setInDevice(devices[i]);
            m_soundSettings.setInListener(this);
            m_soundSettings.numInputChannels = devices[i].inputChannels;
            
            //You can pass 0 as the value for inSampleRate and then the value will be the highest one available in the chosen device
            m_soundSettings.sampleRate = (inSampleRate > 0)?inSampleRate:getMaxValue(devices[i].sampleRates);
            m_soundSettings.numBuffers = 2;
            m_soundSettings.bufferSize = bufferSize;
            
//            ofLogNotice("BufferedAudioInput::setup") << "setting audio device " << i << ": " << devices[i].name << " sampleRate: " << m_soundSettings.sampleRate;
            
            
            size_t outBufferSize = bufferSize * WHISPER_SAMPLE_RATE/(float)m_soundSettings.sampleRate;
            
            ringBuffer = make_unique<LockFreeRingBuffer>(m_soundSettings.numInputChannels * outBufferSize * 100);// make the ring buffer 10 times larger than the input buffers. This size is enough to perform the most commmon sampleRate convertions, etc
            
            m_soundStream.setup(m_soundSettings);
                    
            _isSetup = true;
            return true;
        }
    
    _isSetup = false;
    return false;
}
