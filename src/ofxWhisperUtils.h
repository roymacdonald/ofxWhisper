//
//  ofxWhisperUtils.h
//  example
//
//  Created by Shadow Tuner on 12-09-23.
//

#pragma once
#include <vector>
#include <deque>

namespace ofxWhisper{
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

struct BoundedText{
    BoundedText(string t):text(t){
        static ofBitmapFont bf;
        this->boundingBox = bf.getBoundingBox(t, 0,0);
    }
    string text;
    ofRectangle boundingBox;
};

static inline void updateTextPositions(std::deque<BoundedText>& queue, bool removeIfOffscreen, float startYPos , float spacing = 8, float bottomMargin = 20){
    if(queue.size() > 0){
        float h = 0;
        
        for(auto& t : queue){
            t.boundingBox.y = h + startYPos;
            h += (t.boundingBox.height + spacing);
            
        }
        
        float h_max = ofGetHeight() - bottomMargin - startYPos;
        
        if(h > h_max){
            if(removeIfOffscreen){
                while(h > h_max){
                    h -= queue.front().boundingBox.y + spacing;
                    queue.pop_front();
                }
                //update again the positions
                updateTextPositions(queue, false, startYPos, spacing, bottomMargin);
            }else{
                float y_offset = h - h_max;
                updateTextPositions(queue, false, startYPos - y_offset, spacing, bottomMargin);
            }
        }
    }
}


}
