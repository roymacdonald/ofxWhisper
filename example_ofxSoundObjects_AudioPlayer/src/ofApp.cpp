#include "ofApp.h"
#include "ofxWhisperUtils.h"
//--------------------------------------------------------------
void ofApp::setup(){
    ofxWhisperSettings whisperSettings;
    /// Using the default settings which are. Uncomment any line below and change value if needed
     whisperSettings.n_threads  = std::min(4, (int32_t) std::thread::hardware_concurrency());        //n_threads "number of threads to use during computation\n", ;
     whisperSettings.step_ms    = 2000;     //step_ms "audio step size in milliseconds\n",             ;
     whisperSettings.length_ms  = 5000;     //length_ms "audio length in milliseconds\n",                ;
     whisperSettings.keep_ms    = 300;     //keep_ms "audio to keep from previous step in ms\n",      ;
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

    whisperSettings.model =  "../../../../../addons/ofxWhisper/libs/whisper_cpp/models/ggml-base.bin";

    
    soundStream.printDeviceList();

    ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED;
    
#ifdef TARGET_WIN32
     api = ofSoundDevice::Api::MS_WASAPI;
#endif   


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

    
    // remember to choose the correct output device.
    int outputDeviceIndex = 2;
    int sampleRate = 44100;
    int bufferSize = 512;

    int i = outputDeviceIndex;
    
    
    auto devices = soundStream.getDeviceList(api);
    if ( i < devices.size()) {
        
        whisper.setup(whisperSettings, player.getNumChannels());
        
        // setup the sound stream output as usual.
        
        ofSoundStreamSettings m_soundSettings;
        m_soundSettings.numBuffers = 2;
        m_soundSettings.bufferSize = bufferSize;
        m_soundSettings.setOutListener(&output);
        
        
        m_soundSettings.setOutDevice(devices[i]);
        m_soundSettings.numOutputChannels = devices[i].outputChannels;
        m_soundSettings.sampleRate = sampleRate;
        soundStream.setup(m_soundSettings);
        
        
        
        // coonect the player object to whisper and then to the output.
        player.connectTo(whisper).connectTo(output);
        
    }
}

//--------------------------------------------------------------
void ofApp::update(){
    string newText;
    ofBitmapFont bf;
    while(whisper.textChannel.tryReceive(newText)){
        textQueue.push_back({newText,
            bf.getBoundingBox(newText, 0, 0)
        });
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofRectangle r = whisper.draw();
    
    
    float y = ofGetHeight() - 100;
    auto x = r.getMaxX() + 20;

    if(textQueue.size() > 0){
        // draw in Backwards order so we can remove elements from the queue that cant be shown..
        
        
        
        for( size_t i = textQueue.size() -1; i >= 0; i --){
            
            ofDrawBitmapStringHighlight( textQueue[i].text , x, y );
            y -= textQueue[i].boundingBox.height + 8;
            if(y < 100  ){
                break;
            }
        }
        size_t n = textQueue.size();
        for(size_t i = textQueue.size(); i < n; i ++){
            textQueue.pop_front();
        }
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
