#include "ofApp.h"
#include "ofxWhisperUtils.h"
//--------------------------------------------------------------
void ofApp::setup(){
    ofxWhisper::Settings whisperSettings;
    /// Using the default settings which are. Uncomment any line below and change value if needed
     whisperSettings.n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());        //n_threads "number of threads to use during computation\n", ;
     whisperSettings.step_ms    = 2000;     //step_ms "audio step size in milliseconds\n",             ;
     whisperSettings.length_ms  = 5000;     //length_ms "audio length in milliseconds\n",                ;
    // whisperSettings.keep_ms    = 200;     //keep_ms "audio to keep from previous step in ms\n",      ;
    // whisperSettings.capture_id = -1;    //capture_id "capture device ID\n",                           ;
    // whisperSettings.max_tokens = 32;    //max_tokens "maximum number of tokens per audio chunk\n",    ;
    // whisperSettings.audio_ctx  = 0;    //audio_ctx "audio context size (0 - all)\n",                ;
    // whisperSettings.vad_thold    = 0.6f; //"voice activity detection threshold\n"
    // whisperSettings.freq_thold   = 100.0f; //"high-pass frequency cutoff\n"
    // whisperSettings.speed_up      = false; //"speed up audio by x2 (reduced accuracy)\n",
    // whisperSettings.translate     = false; //"translate from source language to english\n",
    // whisperSettings.print_special = false; //"print special tokens\n",
    // whisperSettings.no_context    = true; //"keep context between audio chunks\n",
    // whisperSettings.no_timestamps = false; 
    // whisperSettings.language  = "en"; //"spoken language\n",
    // whisperSettings.model     = "models/ggml-base.en.bin"; //"model path\n",
    
    /// MAKE SURE YOU CHANGE THIS TO THE CORRECT PATH!
    whisperSettings.model =  "../../../../../addons/ofxWhisper/libs/whisper_cpp/models/ggml-base.en.bin";
//    whisperSettings.model =  "../../../../../addons/ofxWhisper/libs/whisper_cpp/models/ggml-large.bin";
    
   
    ofSoundStream soundStream;
    soundStream.printDeviceList();

    ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED;
    
#ifdef TARGET_WIN32
     api = ofSoundDevice::Api::MS_WASAPI;
#endif   


    // remember to choose the correct input device.
    int inputDeviceIndex = 1;
    int outputDeviceIndex = 2;
    int sampleRate = 44100;
    int bufferSize = 512;
    int i = inputDeviceIndex;
    
    
    auto devices = m_soundStream.getDeviceList(api);
    if ( i < devices.size() && outputDeviceIndex < devices.size()) {
        
        ofSoundStreamSettings m_soundSettings;
        m_soundSettings.numBuffers = 2;
        m_soundSettings.bufferSize = bufferSize;

        whisper.setup(whisperSettings, devices[i].inputChannels);
        m_soundSettings.setInDevice(devices[i]);
        m_soundSettings.setOutDevice(devices[outputDeviceIndex]);
        m_soundSettings.setInListener(&input);
        m_soundSettings.setOutListener(&output);
        m_soundSettings.numInputChannels = devices[i].inputChannels;
        m_soundSettings.numOutputChannels = m_soundSettings.numInputChannels;
        
        //You can pass 0 as the value for inSampleRate and then the value will be the highest one available in the chosen device
        m_soundSettings.sampleRate = (sampleRate > 0)?sampleRate:ofxWhisper::getMaxValue(devices[i].sampleRates);
        // because of the pull-through architecture of ofxSoundObjects you need to connect the whisper object to an output, so it "pulls" the audio data through it and gets processed.
        input.connectTo(whisper).connectTo(output);
           
        m_soundStream.setup(m_soundSettings);
        
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    string newText;
    
    bool bUpdatePositions = false;
    while(whisper.textChannel.tryReceive(newText)){
        textQueue.push_back({newText});
        bUpdatePositions = true;
    }
    if(bUpdatePositions){
        ofxWhisper::updateTextPositions(textQueue,
                            true, //bool removeIfOffscreen,
                            30, //float startYPos ,
                            8, //float spacing = 8,
                            20); // float bottomMargin = 20
    }

}

//--------------------------------------------------------------
void ofApp::draw(){
    ofRectangle r = whisper.draw();
    auto x = r.getMaxX() + 20;

    for(auto& t: textQueue){
        ofDrawBitmapStringHighlight( t.text , x, t.boundingBox.y );
    }
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    
}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
