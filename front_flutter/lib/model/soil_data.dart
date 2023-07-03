class SoilData {
  final String id;
  final String deviceId;
  final double tempSoil;
  final double conductSoil;
  final double waterSoil;
  final String date;
  final int timestamp;

  SoilData({
    required this.id,
    required this.deviceId,
    required this.tempSoil,
    required this.conductSoil,
    required this.waterSoil,
    required this.date,
    required this.timestamp,
  });

  factory SoilData.fromJson(Map<String, dynamic> json) {
    DateTime dateTime = DateTime.parse(json["date"]);
    String formattedDate =
        "${dateTime.day.toString().padLeft(2, '0')}/${dateTime.month.toString().padLeft(2, '0')}/${dateTime.year} ${dateTime.hour.toString().padLeft(2, '0')}:${dateTime.minute.toString().padLeft(2, '0')}:${dateTime.second.toString().padLeft(2, '0')}";

    return SoilData(
      id: json['_id'],
      deviceId: json['device_id'],
      tempSoil: double.tryParse(json['temp_SOIL'].toString()) ?? 0.0,
      conductSoil: double.tryParse(json['conduct_SOIL'].toString()) ?? 0.0,
      waterSoil: double.tryParse(json['water_SOIL'].toString()) ?? 0.0,
      date: formattedDate,
      timestamp: int.tryParse(json['timestamp'].toString()) ?? 0,
    );
  }
}
