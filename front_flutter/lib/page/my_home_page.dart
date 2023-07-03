import 'dart:convert';

import 'package:flutter/material.dart';
import 'package:front_flutter/model/fetch_api.dart';
import 'package:front_flutter/model/soil_data.dart';

class MyHomePage extends StatefulWidget {
  const MyHomePage({super.key, required this.title});

  final String title;

  @override
  State<MyHomePage> createState() => _MyHomePageState();
}

class _MyHomePageState extends State<MyHomePage> {
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      body: Center(
        child: FutureBuilder(
          future: fetchSensorsData(),
          builder: (context, snapshot) {
            if (snapshot.hasData) {
              final list = jsonDecode(snapshot.data!.body) as List;
              final mappedList = list.map((e) => SoilData.fromJson(e)).toList()
                ..sort((a, b) => b.timestamp.compareTo(a.timestamp));

              return SingleChildScrollView(
                child: DataTable(
                  columns: const [
                    DataColumn(
                      label: Expanded(
                        child: Text(
                          'ID',
                          style: TextStyle(fontStyle: FontStyle.italic),
                        ),
                      ),
                    ),
                    DataColumn(
                      label: Expanded(
                        child: Text(
                          'temp_SOIL',
                          style: TextStyle(fontStyle: FontStyle.italic),
                        ),
                      ),
                    ),
                    DataColumn(
                      label: Expanded(
                        child: Text(
                          'conduct_SOIL',
                          style: TextStyle(fontStyle: FontStyle.italic),
                        ),
                      ),
                    ),
                    DataColumn(
                      label: Expanded(
                        child: Text(
                          'water_SOIL',
                          style: TextStyle(fontStyle: FontStyle.italic),
                        ),
                      ),
                    ),
                    DataColumn(
                      label: Expanded(
                        child: Text(
                          'date',
                          style: TextStyle(fontStyle: FontStyle.italic),
                        ),
                      ),
                    ),
                  ],
                  rows: [
                    ...mappedList.map((e) {
                      return DataRow(
                        cells: [
                          DataCell(Text(e.id)),
                          DataCell(Text(e.tempSoil.toString())),
                          DataCell(Text(e.conductSoil.toString())),
                          DataCell(Text(e.waterSoil.toString())),
                          DataCell(Text(e.date)),
                        ],
                      );
                    })
                  ],
                ),
              );
            }
            if (snapshot.hasError) {
              return Center(
                child: SelectableText(snapshot.error!.toString()),
              );
            } else {
              return const Center(
                child: CircularProgressIndicator(),
              );
            }
          },
        ),
      ),
    );
  }
}
