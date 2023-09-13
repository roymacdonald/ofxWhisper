//
//  ofxWhisperUtils.h
//  example
//
//  Created by Shadow Tuner on 12-09-23.
//

#pragma once
#include <vector>


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
