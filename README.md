# ĐỒ ÁN MÔN HỌC

## Giới thiệu

### Tính năng

-  Cân khối lượng, hiển thị nhiệt độ môi trường.

-  Tinh chỉnh kết quả nếu cân chưa đúng, điều chỉnh cân về 0 kg.

-  Thay đổi đơn vị cân từ kilogram sang pound, đơn vị nhiệt độ từ celsius sang fahrenheit.

-  Lưu kết quả của các lượt cân gần nhất.

-  Tự động vào chế độ ngủ (tắt đèn nền, giảm tần suất đọc dữ liệu từ cảm biến).

### Mô phỏng sản phẩm

Cân với 1 bộ HX711: <https://wokwi.com/projects/403644391219010561>

Cân với 4 bộ HX711: <https://wokwi.com/projects/403649602321315841>

## Danh sách linh kiện

[//]: ## (Bill of Material)

| STT | Tên linh kiện | Số lượng | Link
| -- | -- | -- | --
| 1 | ESP32 | 1 |[ESP32 NodeMCU-32S 38 Pins](https://hshop.vn/products/kit-rf-thu-phat-wifi-ble-esp32-nodemcu-32s-ch340-ai-thinker)
| 2 | HX711 | 1 |[Mạch chuyển đổi ADC 24bit HX711](https://hshop.vn/products/mach-chuyen-doi-adc-24bit-loadcell-hx711)
| 3 | Loadcell | 4 | [Cảm biến khối lượng – loadcell 50Kg](https://hshop.vn/products/cam-bien-trong-luong-loadcell-50kg)
| 4 | OLED | 1 | [Màn hình OLED 1.3 inch I2C](https://hshop.vn/products/lcd-oled-trang-1-3-inch-giao-tiep-i2c)
| 5 | Cảm biến nhiệt độ | 1 | [Cảm biến nhiệt độ NTC Thermistor](https://hshop.vn/products/cam-bien-nhiet-do-ntc-thermistor)
| 6 | Công tắc bấm | 5 | [Công tắc nhấn – tact switch button](https://hshop.vn/products/bo-5-loai-cong-tac-nhan-thong-dung-5-kind-tact-switch-button)
| 7 | Module đọc thẻ RFID | 1| [Module RFID - MFRC522](https://hshop.vn/mach-rfid-rc522-nfc-13-56mhz)

## Sơ đồ nguyên lý

[//]: ## (Hardware Schematic)

<img height="300" src="https://github.com/user-attachments/assets/a4e3ccc9-dc0a-4a3d-9536-e529c1ef4c8b">

# Cách hoạt động của phần cứng

## Loadcell 50kg

<img width="300" src="https://github.com/user-attachments/assets/58c4d3c8-78c0-40c2-91b1-0202936a7ec2">

- Loadcell 50kg thông dụng trên thị trường có cấu tạo như hình trên. Gồm 2 điện trở có thể thay đổi giá trị dựa vào độ nén, giãn khi tác động lực lên loadcell. 

<img height="300" src="https://github.com/user-attachments/assets/33e14943-59b6-4384-9a6f-d72c5c744151">
<img height="300" src="https://github.com/user-attachments/assets/2d43dfc5-abc7-40af-ae1e-7530a6e32970">

- 4 loadcell 50kg có thể nối dây để tạo thành một mạch cầu. R1+ và R1- lần lượt là điện trở có giá trị tăng và giảm khi tác động lực lên loadcell 1. E+, E-, A+, A- là tên chân được đấu với HX711. Dựa vào công thức tính hiệu điện thế qua mạch điện gồm các điện trở nối tiếp. Ta tính được hiệu điện thế giữa A+ và A- như sau:

$$
V_{A+, A-} = 2 \times \left\lbrack \left( V_{E +} - \  V_{E -} \right) \left( \frac{1}{1 + \frac{R_{-}}{R_{+}}} - \frac{1}{2} \right) \right\rbrack
$$

## HX711

- Vì chêch lệch hiệu điện thế được tạo ra bởi loadcell là rất nhỏ (cỡ mV) nên để  để đọc được chính xác khối lượng trên các loadcell thì cần một bộ khuyếch đại tín hiệu như HX711. HX711 có hai tốc độ đọc là 10 Hz hoặc 80 Hz được thiết lập qua chân RATE.

<img height="200" src="https://github.com/user-attachments/assets/7737f3f8-cfcd-4b25-8363-30a1a931470c">

- Cách thức truyền gửi dữ liệu trên HX711([HX711 datasheet](https://cdn.sparkfun.com/datasheets/Sensors/ForceFlex/hx711_english.pdf))
1. Chờ tới khi (DOUT → LOW) thì bắt đầu đọc dữ liệu
2. Đọc 24 bit giá trị khuyếch đại. Mỗi khi phát một xung (LOW → HIGH) thì đọc 1 bit từ chân DOUT
3. Phát thêm một số xung tương ứng để set up hệ số khuyếch đại của HX711 cho lần đọc tiếp theo. Sau quá trình đọc dữ liệu, nếu DOUT không lên mức HIGH thì trả về một giá trị đã được định nghĩa để thông báo lỗi

# Thiết kế phần mềm
[//]: # (Software Concept)

## Cấu trúc mã nguồn

| File  | Nhiệm vụ 
|  -- | -- 
| config.h | Khai báo các thành phần phần cứng, các thông số phần cứng và các thông số của chương trình
| screen.cpp, screen.h | Định nghĩa các lớp dùng để giao tiếp với màn hình LCD1602 I2C, OLED SSD1306
| SOICT_HX711.cpp, SOICT_HX711.h | Định nghĩa các lớp dùng để giao tiếp với HX711
| utility.cpp, utility.h  | Định nghĩa các hàm tiện ích, các hàm xử lý ngắt
| gg_sheets.cpp, gg_sheets.h | Định nghĩa các hàm liên quan đến việc truyền gửi dữ liệu lên Google Sheets
| main.h | Khai báo các biến toàn cục
| main.ino | Chương trình chính

## Tính toán khối lượng

 - Khối lượng được tính toán bằng cách đọc dữ liệu từ bộ khuyếch đại HX711 trừ cho giá trị dữ liệu tương đương với 0kg; sau đó chia cho hệ số giữa dữ liệu thu được từ bộ khuyếch đại và khối lượng thực tế.

adc_data: Giá trị đọc được từ HX711 \
Tare: Giá trị của adc_data tương đương với khối lượng của vỏ cân \
m: Khối lượng bao gồm vỏ cân \
w: Khối lượng không tính vỏ cân

⟹ Scale = data / m \
⟹ w = (data – Tare) / Scale

Để tính toán được 2 tham số cần thiết là Scale và Tare. Ta xử lý như sau đối với tham số:

+ Scale: Lấy một vật có khối lượng chuẩn (m kg) đặt lên cân sau đó xem giá trị data tương ứng và tính toán thủ công Scale = data / m rồi chèn vào mã nguồn. Ngoài ra cân còn có 2 nút bấm hiệu chỉnh khối lượng, khi sử dụng 2 nút này sẽ làm thay đổi trực tiếp giá trị của tham số Scale.

+ Tare: Khi cân mới khởi động hoặc khi bấm nút Tare, chương trình sẽ đọc giá trị data từ HX711 và gán vào Tare.
