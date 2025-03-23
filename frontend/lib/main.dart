import 'package:flutter/material.dart';
import 'package:frontend/screens/home/home_view.dart';
import 'package:intl/date_symbol_data_local.dart';
import 'package:frontend/services/habitos_service.dart';
import 'package:provider/provider.dart';
import 'package:frontend/screens/home/home_view_model.dart';

Future<void> main() async {
  final habitoService = HabitoService(baseUrl: 'http://192.168.15.14:3000');
  WidgetsFlutterBinding.ensureInitialized();
  await initializeDateFormatting('pt_BR', null);
  runApp(
    ChangeNotifierProvider(
      create:
          (_) =>
              HabitoViewModel(habitoService: habitoService)..carregarHabitos(),
      child: const MyApp(),
    ),
  );
}

class MyApp extends StatelessWidget {
  const MyApp({super.key});

  @override
  Widget build(BuildContext context) {
    return FutureBuilder(
      future: initializeDateFormatting('pt_BR', null),
      builder: (context, snapshot) {
        if (snapshot.connectionState == ConnectionState.done) {
          return MaterialApp(
            debugShowCheckedModeBanner: false,
            title: 'Tracker Hábitos',
            theme: ThemeData(
              brightness: Brightness.light,
              primaryColor: const Color(0xFF222222),
              fontFamily: 'Poppins',
            ),
            home: const HomeView(),
          );
        } else {
          return MaterialApp(
            home: Scaffold(body: Center(child: CircularProgressIndicator())),
          );
        }
      },
    );
  }
}
