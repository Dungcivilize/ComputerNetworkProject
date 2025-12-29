# Ứng dụng mô phỏng điều khiển hệ thống chăm sóc vườn cây tự động

## 1. Mô tả
Hệ thống chăm sóc vườn cây tự động bao gồm các bộ phận:
- Các bộ cảm biến môi trường đo đạc độ ẩm, nồng độ các chất dinh dưỡng đạm(N), lân(P), kali(K) trong đất. Các cảm biến ghi nhận dữ liệu mỗi T phút.
- Bộ phận tưới nước tự động tưới đồng thời theo 2 cơ chế: tưới tự động vào các thời điểm cố định trong ngày do người dùng thiết lập nếu độ ẩm trong đất không vượt ngưỡng Hmax và tưới tự động nếu độ ẩm trong đất dưới ngưỡng Hmin. Các ngưỡng Hmax và Hmin do người dùng thiết lập.
- Bộ phận bón phân NPK tự động: khi nồng độ các chất dinh dưỡng dưới ngưỡng Nmin, Pmin, Kmin thì hệ thống này tự động thêm một lượng phân bón được hòa vào nước theo nồng độ C gam/lít và tưới một lượng V lít nước. Các thông số do người dùng thiết lập.
- Bộ phận đèn hỗ trợ quang hợp: phát sáng với công suất P theo thời gian được lập lịch bởi người dùng.
Xây dựng ứng dụng mô phỏng bao gồm 2 loại chương trình:
- Chương trình mô phỏng các loại cảm biến và thiết bị:
+ Mỗi thiết bị có một giá trị ID duy nhất
+ Lắng nghe các thông điệp quét trên một cổng ứng dụng nào đó.
+ Khi nhận được yêu cầu kết nối, kiểm tra mật khẩu trong thông điệp. Nếu mật khẩu trùng khớp thì gửi lại thông điệp có chứa các thông tin về thiết bị. Để quản lý các ứng dụng đã được xác thực, thiết bị tạo ra một giá trị token ngẫu nhiên gắn với ID của ứng dụng.
+ Thực hiện các lệnh điều khiển bật/tắt, thiết lập thông số hoạt động
- Chương trình điều khiển cho người dùng, gồm các chức năng:
+ Mỗi ứng dụng cài đặt được khởi tạo một giá trị ID ngẫu nhiên
+ Quản lý các vườn cây trồng
+ Khi thực hiện lệnh quét, chương trình quét các thiết bị và hiển thị loại thiết bị và ID cho người dùng. Người dùng thêm thiết bị mới này vào danh sách thiết bị trong vườn cây.
+ Người dùng yêu cầu kết nối tới thiết bị. Chương trình gửi mật khẩu do người dùng nhập và ID của ứng dụng.
+ Đổi mật khẩu của thiết bị
+ Người dùng có thể điều khiển bật/tắt, thiết lập thông số hoạt động

## 2. Yêu cầu
- Xử lý truyền dòng: 1 điểm x
- Cài đặt cơ chế vào/ra socket trên server: 2 điểm x
- Xác thực truy cập điều khiển: 1 điểm x
- Khởi tạo các thông số cho thiết bị: 1 điểm x
- Quét thiết bị: 2 điểm x
- Kết nối thiết bị: 2 điểm x 
- Đổi mật khẩu: 1 điểm x
- Điều khiển bật tắt: 1 điểm x
- Thiết lập thông số cho bộ phận tưới nước: 1 điểm
- Thiết lập thông số cho bộ phận bón phân: 2 điểm
- Thiết lập thông số cho đèn quang hợp: 1 điểm x/2
- Điều khiển trực tiếp (Bơm nước, bón phân, bật đèn) bên cạnh các cơ chế lập lịch: 1 điểm x
- Lấy thông tin, thông số mỗi loại thiết bị(cảm biến môi trường, tưới nước, bón phân, quang hợp): 2 điểm x
- Ghi log hoạt động trên mỗi thiết bị: 1 điểm x
- Quản lý thông tin vườn cây: 1 điểm

## 3. Thành viên
- Trần Hoàng Dũng - 20225708
- Bàng Tiến Thành - 20225669