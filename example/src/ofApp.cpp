#include "ofApp.h"

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
    whisperSettings.model =  "/Users/roy/openFrameworks/addons/ofxWhisper/libs/whisper_cpp/models/ggml-base.en.bin";
//    whisperSettings.model =  "/Users/roy/openFrameworks/addons/ofxWhisper/libs/whisper_cpp/models/ggml-large.bin";
    
   
   
    ofSoundStream soundStream;
    soundStream.printDeviceList();

    ofSoundDevice::Api api = ofSoundDevice::Api::UNSPECIFIED;
    
#ifdef TARGET_WIN32
    // set your device to the correct api. otherwise it might not work.
    //windows is a bit quirky about this.
     auto devices = soundStream.getDeviceList(ofSoundDevice::Api::MS_WASAPI);
     
     for (size_t i = 0; i < devices.size(); i++) {
         cout << i << "  : " << devices[i].name << endl;
     }
         
     api = ofSoundDevice::Api::MS_WASAPI;

#endif   

    
    
    // remember to choose the correct input device.
    int inputDeviceIndex = 3;
    int inSampleRate = 48000;
    int bufferSize = 512;

    
    whisper.setup(whisperSettings, inputDeviceIndex , inSampleRate, bufferSize , api);
    



}

//--------------------------------------------------------------
void ofApp::update(){
    string newText;
    while(whisper.textChannel.tryReceive(newText)){
        textQueue.push_back(newText);
    }
    while(textQueue.size() > 50){
        textQueue.pop_front();
    }
}

//--------------------------------------------------------------
void ofApp::draw(){
    
    ofRectangle r = whisper.draw();
    
    ofBitmapFont bf;
    float y = r.getMaxY() + 20;
    for(auto& t: textQueue){
        auto r= bf.getBoundingBox(t, 0, 0);
        ofDrawBitmapStringHighlight(t, 80, y );
        y += r.height + 8;	
    }
    
    
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){
    if(key == ' '){
        whisper.toggleWaveBypass();
    }
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
