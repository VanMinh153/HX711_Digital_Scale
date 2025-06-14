// Google Apps Script cho cân điện tử thông minh - Kết nối Google Sheet với thiết bị IoT (ESP32)
// Tác giả: Nguyễn Văn Minh
// Chức năng: Nhận/gửi dữ liệu cân, UID RFID, tên, cân nặng, lưu log, đọc danh sách học sinh
// File này đóng vai trò như một REST API đơn giản cho ESP32 giao tiếp với Google Sheet

function doGet(e) { 
  Logger.log(JSON.stringify(e)); // Log toàn bộ request nhận được để debug trên Apps Script
  var result = 'Ok'; // Chuỗi kết quả trả về cho client (ESP32)
  // Kiểm tra nếu không có tham số truyền lên
  if (e.parameter == 'undefined') {
    result = 'No Parameters'; // Trả về lỗi nếu không có tham số
  }
  else {
    // ID Google Sheet và tên sheet mặc định (danh sách học sinh)
    var sheet_id = '1SgEfCjz_7kMSvQw1k_3l_AM2aDRieO2iUTaTsRO6tQA'; // Thay bằng ID sheet thực tế của bạn
    var sheet_name = "DANHSACH";  // Sheet chứa danh sách học sinh

    // Mở file Google Sheet và lấy sheet mặc định
    var sheet_open = SpreadsheetApp.openById(sheet_id); // Kết nối tới file Google Sheet
    var sheet_target = sheet_open.getSheetByName(sheet_name); // Lấy sheet theo tên

    var newRow = sheet_target.getLastRow(); // Số dòng hiện tại (không dùng trong log, có thể dùng để ghi tiếp dữ liệu)

    var rowDataLog = []; // Mảng lưu dữ liệu log sẽ ghi vào sheet (ngày, giờ, UID, tên, cân nặng)

    // Lấy ngày và giờ hiện tại (theo múi giờ Asia/Jakarta, có thể đổi sang Asia/Ho_Chi_Minh nếu cần)
    var Curr_Date = Utilities.formatDate(new Date(), "Asia/Jakarta", 'dd/MM/yyyy');
    rowDataLog[0] = Curr_Date;  // Cột A: Ngày

    var Curr_Time = Utilities.formatDate(new Date(), "Asia/Jakarta", 'HH:mm:ss');
    rowDataLog[1] = Curr_Time;  // Cột B: Giờ

    var sts_val = ''; // Biến lưu trạng thái thao tác (read, writeuid, writelog)

    // Duyệt qua tất cả các tham số truyền lên từ ESP32 (uid, name, weight, sts...)
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param); // Log tên tham số
      var value = stripQuotes(e.parameter[param]); // Loại bỏ dấu nháy nếu có
      Logger.log(param + ':' + e.parameter[param]); // Log giá trị tham số
      switch (param) {
        case 'sts':
          sts_val = value; // Xác định loại thao tác (read, writeuid, writelog)
          break;
        case 'uid':
          rowDataLog[2] = value;  // Cột C: UID học sinh (mã thẻ RFID)
          result += ', UID Written';
          break;
        case 'name':
          rowDataLog[3] = value;  // Cột D: Tên học sinh
          result += ', Name Written';
          break; 
        case 'weight':
          rowDataLog[4] = value;  // Cột E: Cân nặng
          result += ', weight Written';
          break;       
        default:
          result += ", unsupported parameter"; // Nếu có tham số lạ sẽ báo không hỗ trợ
      }
    }

    // Ghi UID mới nhất vào ô F1 nếu sts=writeuid (dùng cho xác thực nhanh, ví dụ khi quét thẻ RFID)
    if (sts_val == 'writeuid') {
      Logger.log(JSON.stringify(rowDataLog)); // Log dữ liệu chuẩn bị ghi
      // Kiểm tra dữ liệu hợp lệ (phải có ít nhất UID)
      if (Array.isArray(rowDataLog) && rowDataLog.length > 2) {
        var RangeDataLatest = sheet_target.getRange('F1'); // Lấy ô F1
        RangeDataLatest.setValue(rowDataLog[2]); // Ghi UID vào F1
        return ContentService.createTextOutput('Success'); // Trả về thành công cho ESP32
      } else {
        Logger.log('Error: rowDataLog is not valid'); // Log lỗi nếu thiếu dữ liệu
        return ContentService.createTextOutput('Error: Invalid data'); // Trả về lỗi cho ESP32
      }
    }
    // Ghi log cân nặng vào sheet CANNANG nếu sts=writelog
    if (sts_val == 'writelog') {
      sheet_name = "CANNANG";  // Sheet ghi log cân nặng
      sheet_target = sheet_open.getSheetByName(sheet_name); // Lấy sheet log
      Logger.log(JSON.stringify(rowDataLog)); // Log dữ liệu chuẩn bị ghi
      sheet_target.insertRows(2); // Chèn dòng mới lên đầu (dòng 2), đẩy các log cũ xuống dưới
      var newRangeDataLog = sheet_target.getRange(2,1,1, rowDataLog.length); // Lấy vùng ghi dữ liệu
      newRangeDataLog.setValues([rowDataLog]); // Ghi dữ liệu log (ngày, giờ, UID, tên, cân nặng)
      return ContentService.createTextOutput(result); // Trả về kết quả cho ESP32
    }
    // Đọc danh sách học sinh nếu sts=read (ESP32 tải danh sách về bộ nhớ)
    if (sts_val == 'read') {
      sheet_name = "DANHSACH";  // Sheet chứa danh sách học sinh
      sheet_target = sheet_open.getSheetByName(sheet_name); // Lấy sheet
      var all_Data = sheet_target.getRange('A2:C11').getDisplayValues(); // Lấy danh sách từ A2:C11 (có thể mở rộng vùng này)
      return ContentService.createTextOutput(all_Data); // Trả về danh sách cho ESP32
    }
    // Nếu không khớp sts nào thì không làm gì, trả về Ok
  }
}
// Hàm xóa dữ liệu thừa (không dùng cho WeightControl, chỉ dùng cho sheet DATA)
// Có thể dùng để dọn dẹp dữ liệu cũ nếu cần
function maxRowData(allRowsAfter) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet()
                              .getSheetByName('DATA')
  sheet.getRange(allRowsAfter+1, 1, sheet.getLastRow()-allRowsAfter, sheet.getLastColumn())
       .clearContent()
}
// Hàm loại bỏ dấu nháy khỏi chuỗi (dùng để làm sạch dữ liệu nhận từ HTTP GET)
// Ví dụ: '12345' => 12345
function stripQuotes( value ) {
  return value.replace(/^['"]|['"]$/g, "");
}
//________________