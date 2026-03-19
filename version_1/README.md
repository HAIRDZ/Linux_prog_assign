# Bài tập lập trình Linux – Đo độ chính xác thời gian

## 1. Mô tả
Chương trình xây dựng hệ thống đa luồng để đo khoảng thời gian giữa các lần lấy mẫu (interval) trên Linux. Chu kỳ lấy mẫu được thay đổi động để khảo sát độ chính xác của hệ thống.

---

## 2. Thiết kế hệ thống

Hệ thống gồm 3 luồng:

- SAMPLE: tạo timestamp theo chu kỳ X sử dụng thời gian tuyệt đối  
- LOGGING: tính khoảng thời gian giữa 2 lần lấy mẫu và ghi ra file  
- INPUT: đọc giá trị chu kỳ X từ file và cập nhật trong runtime  

---

## 3. Nguyên lý hoạt động

- Sử dụng cơ chế thời gian tuyệt đối để tránh sai số tích lũy  
- Chu kỳ X được thay đổi theo thời gian bằng script  
- Dữ liệu được ghi lại để phân tích độ chính xác  

Các giá trị X được khảo sát:

- 1,000,000 ns (1 ms)  
- 100,000 ns  
- 10,000 ns  
- 1,000 ns  
- 100 ns  

---

## 4. Phân tích kết quả

Dưới đây là bản **nhận xét đúng kiểu kỹ thuật, tập trung vào code và thuật toán**:

---

Chương trình được thiết kế với ba thread hoạt động song song: SAMPLE, INPUT và LOGGING. Thread SAMPLE sử dụng `clock_nanosleep()` với chế độ `TIMER_ABSTIME` trên `CLOCK_MONOTONIC`, cho phép lập lịch theo thời gian tuyệt đối. Cách này giúp hạn chế sai số tích lũy (drift) so với việc sleep tương đối, vì mỗi chu kỳ luôn được tính dựa trên mốc thời gian kế tiếp (`nextTime`) thay vì thời điểm hiện tại.

Trong mỗi vòng lặp, thread SAMPLE tăng `nextTime` thêm `periodNs`, chuẩn hóa lại `tv_nsec`, sau đó sleep đến đúng mốc thời gian này. Sau khi thức dậy, thời gian hiện tại được lấy bằng `clock_gettime(CLOCK_REALTIME)` và ghi vào biến dùng chung. Thread LOGGING chờ tín hiệu từ SAMPLE thông qua `pthread_cond_wait`, sau đó tính interval bằng hiệu giữa hai lần timestamp liên tiếp và ghi ra file. Cách sử dụng mutex và condition variable đảm bảo dữ liệu không bị race condition và tránh polling không cần thiết.

Thread INPUT định kỳ đọc file `freq.txt` để cập nhật lại `periodNs`. Việc cập nhật này có khóa mutex, nhưng không đồng bộ với biến `nextTime` trong SAMPLE. Khi `periodNs` thay đổi, `nextTime` vẫn tiếp tục cộng dồn theo giá trị cũ rồi mới chuyển sang giá trị mới, dẫn đến một số khoảng interval bất thường tại thời điểm chuyển pha.

Kết quả đo cho thấy với chu kỳ lớn (ví dụ 1e6 ns), interval dao động nhỏ quanh giá trị đặt, chứng tỏ cơ chế sleep tuyệt đối hoạt động ổn định. Khi giảm chu kỳ xuống các mức nhỏ hơn, độ dao động tăng lên và xuất hiện nhiều giá trị lệch lớn. Nguyên nhân chính là chi phí lập lịch của hệ điều hành, độ trễ context switch và thời gian xử lý trong user-space lớn hơn hoặc cùng bậc với chu kỳ yêu cầu. Với các giá trị rất nhỏ như 1000 ns hoặc 100 ns trong mỗi vòng lặp, thread SAMPLE không chỉ thực hiện sleep mà còn phải trải qua nhiều bước như chuyển trạng thái từ sleep sang running, thực hiện context switch, gọi clock_gettime, thao tác với mutex và gửi tín hiệu condition variable. Tổng thời gian cho các thao tác này thường nằm ở mức vài microsecond hoặc lớn hơn, vượt xa giá trị chu kỳ yêu cầu => dẫn đến interval phân tán rộng.


---

## 5. Cách chạy chương trình

```bash
make

mkdir -p data
echo 1000000 > data/freq.txt
touch data/timeAndInterval.txt

chmod +x scripts/changeFreq.sh

sudo ./scripts/changeFreq.sh &
sudo taskset -c 0 ./main

