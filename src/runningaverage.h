#pragma once
#ifndef RUNNINGAVERAGE_H

#define maxSample 25
class RunningAverage {
public:
    RunningAverage() : numValues_(0), sum_(0) {}

    unsigned int addValue(int value) {
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

        return sum_ / numValues_;
    }

private:
    unsigned long values_[maxSample];
    int numValues_;
    unsigned long sum_;
};
#endif