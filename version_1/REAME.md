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

## 4. Kết quả

- Khi X lớn: interval ổn định, sai số nhỏ  
- Khi X giảm: xuất hiện jitter  
- Khi X rất nhỏ: hệ thống mất ổn định  
- Có spike do hiện tượng deadline miss  

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

