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

Chương trình sử dụng thread SAMPLE với cơ chế sleep theo thời gian tuyệt đối (clock_nanosleep với TIMER_ABSTIME), trong đó nextTime được cộng dồn theo periodNs để bám theo timeline lý tưởng. Cách này giúp tránh sai số tích lũy và đảm bảo về mặt thuật toán rằng chu kỳ thực thi luôn hướng tới giá trị X mong muốn. Dữ liệu được đồng bộ qua mutex và condition variable, sau đó thread LOGGING tính interval bằng hiệu giữa hai timestamp liên tiếp.

Kết quả đo cho thấy khi chu kỳ X lớn (1000000 ns, 100000 ns), giá trị interval trung bình bám sát giá trị đặt và độ dao động nhỏ. Tuy nhiên, khi X giảm xuống 10000 ns, 1000 ns và 100 ns, giá trị trung bình bắt đầu lệch khỏi X và hội tụ về khoảng ~10000 ns, đồng thời xuất hiện nhiều spike lớn.
Nguyên nhân là do tổng chi phí thực thi của hệ thống (context switch, scheduler, system call, đồng bộ mutex và I/O) lớn hơn chu kỳ yêu cầu. 

Khi nextTime đến, thread không được chạy ngay mà bị trễ, khiến interval thực tế bị kéo dài. Khi X nhỏ hơn độ trễ tối thiểu của hệ thống, interval không còn phụ thuộc vào X mà bị giới hạn bởi độ trễ này, dẫn đến giá trị trung bình bị lệch và phân bố rộng.

=> hướng giải quyết : có thể cải thiện bằng cách giảm overhead (hạn chế ghi file, tối ưu đồng bộ), gán CPU affinity và sử dụng lập lịch real-time (SCHED_FIFO)

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

