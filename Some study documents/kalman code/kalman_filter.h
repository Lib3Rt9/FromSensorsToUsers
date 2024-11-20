#ifndef KALMAN_FILTER_H
#define KALMAN_FILTER_H

class KalmanFilter
{
private:
    float estimate;    // Ước tính hiện tại
    float errEstimate; // Sai số ước tính hiện tại
    float errMeasure;  // Sai số của phép đo
    float q;           // Nhiễu quá trình
    float kalmanGain;  // Hệ số Kalman

public:
    /**
     * @brief Constructor để khởi tạo bộ lọc Kalman với các giá trị cụ thể.
     *
     * Constructor này khởi tạo bộ lọc Kalman với các giá trị đầu vào cụ thể để cài đặt các tham số cần thiết cho bộ lọc.
     *
     * @param initialEstimate Ước lượng ban đầu cho trạng thái mà bộ lọc sẽ theo dõi. Đây là giá trị khởi điểm của bộ lọc.
     * @param initialErrEstimate Sai số ước lượng ban đầu, thể hiện độ không chắc chắn ban đầu về ước lượng của trạng thái.
     * @param measurementError Sai số của phép đo, thường dựa trên độ chính xác của cảm biến được dùng để đo lường.
     * @param processNoise Nhiễu quá trình, thể hiện sự không chắc chắn trong mô hình hoặc trong quá trình đo.
     * Việc hiểu và điều chỉnh nhiễu quá trình phù hợp với mỗi môi trường,... là chìa khóa để tối ưu hóa hiệu quả của
     * bộ lọc Kalman, giúp đảm bảo rằng dữ liệu đầu ra là chính xác và đáng tin cậy nhất có thể.
     */
    KalmanFilter(float initialEstimate, float initialErrEstimate, float measurementError, float processNoise);

    // Phương thức cập nhật dựa trên giá trị đo mới
    float update(float measurement);
};

#endif // KALMAN_FILTER_H
