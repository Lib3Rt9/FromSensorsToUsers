#include "kalman_filter.h"
#include <cmath>

KalmanFilter::KalmanFilter(float initialEstimate, float initialErrEstimate, float measurementError, float processNoise)
    : estimate(initialEstimate), errEstimate(initialErrEstimate), errMeasure(measurementError), q(processNoise) {}

float KalmanFilter::update(float measurement)
{
    // Tính toán hệ số Kalman
    kalmanGain = errEstimate / (errEstimate + errMeasure);

    // Cập nhật ước tính hiện tại
    estimate = estimate + kalmanGain * (measurement - estimate);

    // Cập nhật sai số ước tính
    errEstimate = (1.0 - kalmanGain) * errEstimate + fabs(measurement - estimate) * q;

    // Trả về giá trị ước tính mới
    return estimate;
}
