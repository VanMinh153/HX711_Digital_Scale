function doGet(e) { 
  Logger.log(JSON.stringify(e));
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    // ID Google Sheet và tên sheet mặc định (danh sách học sinh)
    var sheet_id = '1SgEfCjz_7kMSvQw1k_3l_AM2aDRieO2iUTaTsRO6tQA';
    var sheet_name = "DANHSACH";  // Sheet chứa danh sách học sinh

    var sheet_open = SpreadsheetApp.openById(sheet_id);
    var sheet_target = sheet_open.getSheetByName(sheet_name);

    var newRow = sheet_target.getLastRow();

    var rowDataLog = [];

    // Lấy ngày và giờ hiện tại
    var Curr_Date = Utilities.formatDate(new Date(), "Asia/Jakarta", 'dd/MM/yyyy');
    rowDataLog[0] = Curr_Date;  // Cột A: Ngày

    var Curr_Time = Utilities.formatDate(new Date(), "Asia/Jakarta", 'HH:mm:ss');
    rowDataLog[1] = Curr_Time;  // Cột B: Giờ

    var sts_val = '';

    // Xử lý các tham số truyền lên từ ESP32
    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'sts':
          sts_val = value;
          break;
        case 'uid':
          rowDataLog[2] = value;  // Cột C: UID học sinh
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
          result += ", unsupported parameter";
      }
    }

    // Ghi UID mới nhất vào ô F1 nếu sts=writeuid
    if (sts_val == 'writeuid') {
      Logger.log(JSON.stringify(rowDataLog));
      if (Array.isArray(rowDataLog) && rowDataLog.length > 2) {
        var RangeDataLatest = sheet_target.getRange('F1');
        RangeDataLatest.setValue(rowDataLog[2]);
        return ContentService.createTextOutput('Success');
      } else {
        Logger.log('Error: rowDataLog is not valid');
        return ContentService.createTextOutput('Error: Invalid data');
      }
    }
    // Ghi log cân nặng vào sheet CANNANG nếu sts=writelog
    if (sts_val == 'writelog') {
      sheet_name = "CANNANG";  // Sheet ghi log cân nặng
      sheet_target = sheet_open.getSheetByName(sheet_name);
      Logger.log(JSON.stringify(rowDataLog));
      sheet_target.insertRows(2);
      var newRangeDataLog = sheet_target.getRange(2,1,1, rowDataLog.length);
      newRangeDataLog.setValues([rowDataLog]);
      return ContentService.createTextOutput(result);
    }
    // Đọc danh sách học sinh nếu sts=read
    if (sts_val == 'read') {
      sheet_name = "DANHSACH";  // Sheet chứa danh sách học sinh
      sheet_target = sheet_open.getSheetByName(sheet_name);
      var all_Data = sheet_target.getRange('A2:C11').getDisplayValues();
      return ContentService.createTextOutput(all_Data);
    }
  }
}
// Hàm xóa dữ liệu thừa (không dùng cho WeightControl)
function maxRowData(allRowsAfter) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet()
                              .getSheetByName('DATA')
  sheet.getRange(allRowsAfter+1, 1, sheet.getLastRow()-allRowsAfter, sheet.getLastColumn())
       .clearContent()
}
// Hàm loại bỏ dấu nháy khỏi chuỗi
function stripQuotes( value ) {
  return value.replace(/^['"]|['"]$/g, "");
}
//________________