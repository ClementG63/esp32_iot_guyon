import 'package:http/http.dart';

Future<Response> fetchSensorsData() {
  final dateNow = DateTime.now();
  return get(
    Uri.parse('http://localhost:3000/sensors?date=${dateNow.day}/${dateNow.month}/${dateNow.year}'),
    headers: {
      "Content-Type": "application/json",
      "Access-Control_Allow_Origin": "*",
    },
  );
}
