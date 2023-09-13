#include "ofApp.h"
#include "ofxWhisperUtils.h"
//--------------------------------------------------------------
void ofApp::setup(){
    ofxWhisperSettings whisperSettings;
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
    whisperSettings.model =  "/Users/admin/openFrameworks/addons/ofxWhisper/libs/whisper_cpp/models/ggml-base.en.bin";
//    whisperSettings.model =  "/Users/roy/openFrameworks/addons/ofxWhisper/libs/whisper_cpp/models/ggml-large.bin";
    
   
    ofSoundStream soundStream;
    soundStream.printDeviceList();

    ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED;
    
#ifdef TARGET_WIN32
     api = ofSoundDevice::Api::MS_WASAPI;
#endif   

#ifdef USE_AUDIO_PLAYER
    auto res = ofSystemLoadDialog("Choose an audio file to play through Whisper");
    while(true){
        if(res.bSuccess){
            if(player.load(res.getPath())){
                player.play();
                break;
            }else{
                ofLogError("ofApp::setup") << "Failed loading audio file " << res.getPath();
            }
        }
    }
#endif
    
    // remember to choose the correct input device.
    int inputDeviceIndex = 1;
    int outputDeviceIndex = 2;
    int sampleRate = 44100;
    int bufferSize = 512;
#ifdef USE_AUDIO_PLAYER
    int i = outputDeviceIndex;
#else
    int i = inputDeviceIndex;
#endif
    
    
    auto devices = m_soundStream.getDeviceList(api);
    if ( i < devices.size()) {
        
        ofSoundStreamSettings m_soundSettings;
        m_soundSettings.numBuffers = 2;
        m_soundSettings.bufferSize = bufferSize;
#ifdef USE_AUDIO_PLAYER
        whisper.setup(whisperSettings, player.getNumChannels());
        m_soundSettings.setOutListener(&whisper);
        m_soundSettings.setOutDevice(devices[i]);
        m_soundSettings.numOutputChannels = devices[i].outputChannels;
        m_soundSettings.sampleRate = sampleRate;
        player.connectTo(whisper);
#else
        whisper.setup(whisperSettings, devices[i].inputChannels);
        m_soundSettings.setInDevice(devices[i]);
        m_soundSettings.setInListener(&input);
        m_soundSettings.numInputChannels = devices[i].inputChannels;
        //You can pass 0 as the value for inSampleRate and then the value will be the highest one available in the chosen device
        m_soundSettings.sampleRate = (sampleRate > 0)?sampleRate:getMaxValue(devices[i].sampleRates);
        input.connectTo(whisper);
#endif
           
        m_soundStream.setup(m_soundSettings);
        
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    string newText;
    ofBitmapFont bf;
    while(whisper.textChannel.tryReceive(newText)){
        textQueue.push_front({newText,
            bf.getBoundingBox(newText, 0, 0)
        });
    }
//    if(textQueue.size() > 0){
//
//
//        for(size_t i = textQueue.size(); i > 0; --i){
//            y += textQueue[i-1].boundingBox.height + 8;
//            if(y + 100 > ofGetHeight()){
//                popIndex = i-1;
//                bFound = true;
//                break;
//            }
//        }
//        if(bFound){
//            for(size_t i = 0; i < popIndex; i ++){
//                textQueue.pop_front();
//            }
//        }
//
//    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofRectangle r = whisper.draw();
    
    
    float y = r.y + 20;
    auto x = r.getMaxX() + 20;

    size_t i = 0;
    for(auto& t: textQueue){
        
        ofDrawBitmapStringHighlight(ofToString(i) + " - " + t.text , x, y );
        y += t.boundingBox.height + 8;
        i++;
        if(y+ 100 > ofGetHeight()){
            break;
        }
    }
    size_t n = textQueue.size();
    for(; i < n; i ++){
        textQueue.pop_back();
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
