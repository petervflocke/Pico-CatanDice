#pragma once
#ifndef RUNNINGAVERAGE_H

/*
U(BatPin) = 3.3/4095 = U2
U(Volatege devider VBat-R1-BatPin-R2-GND)= ((R1+R2)/R2)*U2
R1=553K, R2=991K

Any way last correction can be done on live data measured im system
*/



#define maxSample 25
class RunningAverage {
public:
    RunningAverage() : numValues_(0), sum_(0) {}

    double addValue(int value) {
        double batVol;

        if (numValues_ < maxSample) {
            // Add value to array if there is space
            values_[numValues_] = value;
            numValues_++;
            sum_ += value;
        } else {
            // Shift all values to the left and add new value to the end
            unsigned long removed = values_[0];
            for (int i = 0; i < maxSample-1; i++) {
                values_[i] = values_[i + 1];
            }
            values_[maxSample-1] = value;
            sum_ = sum_ - removed + value;
        }
        batVol = ( (sum_ / numValues_) * 132.692871094)/100000.00;
        // if (batVol < 0.5) batVol = 0; 
        return batVol;
    }

private:
    unsigned long values_[maxSample];
    int numValues_;
    unsigned long sum_;
};
#endif