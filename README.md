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

## Sơ đồ nguyên lý

[//]: ## (Hardware Schematic)

![{image](https://github.com/user-attachments/assets/a4e3ccc9-dc0a-4a3d-9536-e529c1ef4c8b)

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

- Vì chêch lệch hiệu điện thế được tạo ra bởi loadcell là rất nhỏ (cỡ mV) nên để  để đọc được chính xác khối lượng trên các loadcell thì cần một bộ khuyếch đại tín hiệu như HX711.

<img src="https://github.com/user-attachments/assets/7737f3f8-cfcd-4b25-8363-30a1a931470c">

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
| HEDSPI_HX711.cpp, HEDSPI_HX711.h | Định nghĩa các lớp dùng để giao tiếp với HX711
| utility.cpp, utility.h  | Định nghĩa các hàm tiện ích, các hàm xử lý ngắt
| main.h | Khai báo các biến toàn cục
| main.ino | Chương trình chính

## Tính toán khối lượng

- Khối lượng được tính toán bằng cách đọc dữ liệu từ bộ khuyếch đại HX711 trừ cho giá trị dữ liệu tương đương với 0kg; sau đó chia cho hệ số giữa dữ liệu thu được từ bộ khuyếch đại và khối lượng thực tế.

adc_data: Giá trị đọc được từ HX711
Tare: Giá trị của adc_data tương đương với khối lượng của vỏ cân
m: Khối lượng bao gồm vỏ cân
w: Khối lượng không tính vỏ cân
⟹ Scale = data / m
⟹ w = (data – Tare) / Scale

Để tính toán được 2 tham số cần thiết là Scale và Tare. Em xử lý như sau đối với tham số:

\+ Scale: Lấy một vật có khối lượng chuẩn (m kg) đặt lên cân sau đó xem giá trị data tương ứng và tính toán thủ công Scale = data / m rồi chèn vào mã nguồn. Ngoài ra cân còn có 2 nút bấm hiệu chỉnh khối lượng, khi sử dụng 2 nút này sẽ làm thay đổi trực tiếp giá trị của tham số Scale.

\+ Tare: Khi cân mới khởi động hoặc khi bấm nút Tare, chương trình sẽ đọc giá trị data từ HX711 và gán vào Tare.

## Xử lý ngắt

Hàm xử lý ngắt cho mỗi nút bấm có cấu trúc tương tự nhau. Để tránh hiện tượng tín hiệu không ổn định trong thời gian bấm nút, chương trình chỉ ghi nhận mỗi ngắt cách nhau một khoảng thời gian cố định DEBOUNCE_TIME (200ms). Khi ghi nhận ngắt của nút nào thì gán biến tương ứng bằng 1, tăng biến ngắt tổng của chương trình (interrupt) lên 1 đơn vị.

\- Xử lý ngắt cho nút UP (tương tự cho nút DOWN): nút UP được dùng để điều chỉnh kết quả hiện thị tăng lên nếu kết quả hiện thị đang bị lệch bằng cách giảm hệ số Scale xuống. Nếu người dùng nhấn nút 1 lần, khối lượng hiển thị sẽ chỉ tăng một ít. Nếu người dùng bấm giữ nút UP thì khối lượng hiển thị sẽ tăng nhanh hơn.

\- Xử lý ngắt cho nút TARE: đọc dữ liệu từ bộ khuyếch đại và gán cho biến Tare.

\- Xử lý ngắt cho nút MODE: thay đổi biến Mode từ (kg, °C) sang (lb, °F) bằng cách thay đổi giá trị biến Mode.

\- Xử lý ngắt cho nút RECORD: hiển thị lần lượt khối lượng của các lượt cân gần nhất

## Chế độ ngủ

Nếu sau một khoảng thời gian cố định, giá trị đọc được từ bộ khuyếch đại không thay đổi thì chương trình sẽ tự động vào chế độ ngủ. Trong chế độ ngủ, chương trình vẫn lắng nghe có ngắt xảy ra hay không, khối lượng có thay đổi hay không. Nếu phát hiện có ngắt hoặc thay đổi khối lượng, chương trình sẽ trở về luồng hoạt động chính. Ngoài ra khi vào chế độ ngủ, chương trình sẽ chuyển sang dùng GAIN_64 thay vì GAIN_128 để giảm thời gian đọc dữ liệu từ bộ khuyếch đại.

# Các vấn đề gặp phải

\- Đọc dữ liệu từ bộ khuyếch đại HX711 mất khá nhiều thời gian (với GAIN_128 thì trung bình mất 100ms cho mỗi lần đọc). Mà chương trình cần đọc nhiều lần liên tiếp để đảm bảo lấy được giá trị có độ chính xác cao. Vì thế khi chương trình vừa cần delay để hiển thị kết quả, vừa cần lắng nghe xem khối lượng có thay đổi không để hiển thị lại ngay thì thời gian mỗi lần delay chưa được chính xác.

\- Không thể đưa toàn bộ phần xử ngắt vào hàm xử lý ngắt vì trong hàm xử lý ngắt không thực hiện được hàm delay() và các hàm giao tiếp với màn hình lcd, oled.

Có thể là do trong khi thực hiện một hàm ngắt, chương trình tạm thời tắt tất cả các ngắt khác của hệ thống (bao gồm timer) nên trong hàm xử lý ngắt không thực hiện được những hàm sử dụng timer. Đồng thời có thể có sự tối ưu code trong quá trình biên dịch dẫn đến sai logic của người lập trình.
