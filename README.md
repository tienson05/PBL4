# PBL4 - Qt + libpcap Network Tool

## Build
```bash
git clone https://github.com/<your-name>/pbl4.git
cd pbl4
mkdir build && cd build
cmake ..
make -j$(nproc)

./src/AppController/App

Dependencies
Qt 6.5+
libpcap (install via sudo apt-get install libpcap-dev)


---

## ✅ Kết quả cuối

Khi bạn làm xong 3 phần trên:
- Bạn push code lên GitHub (kèm thư viện trong `third_party/libpcap/`)
- Người khác clone về, mở trong Qt Creator, nhấn **Run** là chạy ✅  
→ Không cần cài đặt thủ công gì thêm.

---
