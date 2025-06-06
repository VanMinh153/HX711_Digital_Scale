function doGet(e) { 
  Logger.log(JSON.stringify(e));
  var result = 'Ok';
  if (e.parameter == 'undefined') {
    result = 'No Parameters';
  }
  else {
    var sheet_id = '1_ewZ39cee0FxRZO8mC_63_oOu7iiOoVyv8umdxcGn6A'; 	// Spreadsheet ID.
    var sheet_name = "DANHSACH";  // Sheet Name in Google Sheets.

    var sheet_open = SpreadsheetApp.openById(sheet_id);
    var sheet_target = sheet_open.getSheetByName(sheet_name);

    var newRow = sheet_target.getLastRow();

    var rowDataLog = [];

    var Curr_Date = Utilities.formatDate(new Date(), "Asia/Jakarta", 'dd/MM/yyyy');
    rowDataLog[0] = Curr_Date;  // Date will be written in column A (in the "DHT11 Sensor Data Logger" section).

    var Curr_Time = Utilities.formatDate(new Date(), "Asia/Jakarta", 'HH:mm:ss');
    rowDataLog[1] = Curr_Time;  // Time will be written in column B (in the "DHT11 Sensor Data Logger" section).

    var sts_val = '';

    for (var param in e.parameter) {
      Logger.log('In for loop, param=' + param);
      var value = stripQuotes(e.parameter[param]);
      Logger.log(param + ':' + e.parameter[param]);
      switch (param) {
        case 'sts':
          sts_val = value;
          break;

        case 'uid':
          rowDataLog[2] = value;  
          result += ', UID Written';
          break;

        case 'name':
          rowDataLog[3] = value; 
          result += ', Name Written';
          break; 

        case 'inout':
          rowDataLog[4] = value; 
          result += ', INOUT Written';
          break;       

        default:
          result += ", unsupported parameter";
      }
    }

    // Conditions for writing data received from ESP32 to Google Sheets.
    if (sts_val == 'writeuid') {
      // Writes data to the "DHT11 Sensor Data Logger" section.
      Logger.log(JSON.stringify(rowDataLog));
      
      // Ensure rowDataLog is an array and has at least 3 elements
      if (Array.isArray(rowDataLog) && rowDataLog.length > 2) {
        var RangeDataLatest = sheet_target.getRange('F1');
        RangeDataLatest.setValue(rowDataLog[2]);
        
        return ContentService.createTextOutput('Success');
      } else {
        Logger.log('Error: rowDataLog is not valid');
        return ContentService.createTextOutput('Error: Invalid data');
      }
    }
    
    // Conditions for writing data received from ESP32 to Google Sheets.
    if (sts_val == 'writelog') {
      sheet_name = "DIEMDANH";  // Sheet Name in Google Sheets.
      sheet_target = sheet_open.getSheetByName(sheet_name);
      // Writes data to the "DHT11 Sensor Data Logger" section.
      Logger.log(JSON.stringify(rowDataLog));
      // Insert a new row above the existing data.
      sheet_target.insertRows(2);
      var newRangeDataLog = sheet_target.getRange(2,1,1, rowDataLog.length);
      newRangeDataLog.setValues([rowDataLog]);
      //maxRowData(11);
      return ContentService.createTextOutput(result);
    }
    
    // Conditions for sending data to ESP32 when ESP32 reads data from Google Sheets.
    if (sts_val == 'read') {
      sheet_name = "DANHSACH";  // Sheet Name in Google Sheets.
      sheet_target = sheet_open.getSheetByName(sheet_name);

      // Use the line of code below if you want ESP32 to read data from columns I3 to O3 (Date,Time,Sensor Reading Status,Temperature,Humidity,Switch 1, Switch 2).
      var all_Data = sheet_target.getRange('A2:C11').getDisplayValues();
      
      // Use the line of code below if you want ESP32 to read data from columns K3 to O3 (Sensor Reading Status,Temperature,Humidity,Switch 1, Switch 2).
      //var all_Data = sheet_target.getRange('A2:C11').getValues();
      return ContentService.createTextOutput(all_Data);
    }
  }
}
function maxRowData(allRowsAfter) {
  const sheet = SpreadsheetApp.getActiveSpreadsheet()
                              .getSheetByName('DATA')
  
  sheet.getRange(allRowsAfter+1, 1, sheet.getLastRow()-allRowsAfter, sheet.getLastColumn())
       .clearContent()

}
function stripQuotes( value ) {
  return value.replace(/^["']|['"]$/g, "");
}
//________________