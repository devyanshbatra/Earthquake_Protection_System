function doPost(e) {
  var lock = LockService.getDocumentLock();
  lock.acquire();
  try {
    var sheet = SpreadsheetApp.getActiveSheet();
    var timestamp = new Date();
    var sensorValue = e.parameter.sensorReading;
    var id = sheet.getLastRow() + 1; // Generate ID based on last row
    sheet.appendRow([id, timestamp, sensorValue]);
  } finally {
    lock.release();
  }
}