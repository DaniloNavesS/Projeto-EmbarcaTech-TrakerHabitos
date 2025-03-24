import 'package:flutter/material.dart';
import 'package:date_picker_timeline/date_picker_timeline.dart';
import 'package:intl/intl.dart';
import 'package:provider/provider.dart';
import '../../model/habitos_model.dart';
import '../home/home_view_model.dart';

class HomeView extends StatefulWidget {
  const HomeView({super.key});

  @override
  State<HomeView> createState() => _HomeViewState();
}

class _HomeViewState extends State<HomeView> {
  DateTime _selectedValue = DateTime(DateTime.now().year, DateTime.now().month, 1);

  Widget _buildTile(String title, int value, Color color, IconData icon) {
    return ListTile(
      contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 4),
      tileColor: const Color(0xFF222222),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
      leading: Icon(icon, color: Colors.white),
      title: Text(
        title,
        style: const TextStyle(
          fontSize: 18,
          fontWeight: FontWeight.bold,
          color: Colors.white,
        ),
      ),
      trailing: CircleAvatar(
        radius: 20,
        backgroundColor: color,
        child: Text(
          value.toString(),
          style: const TextStyle(
            color: Colors.white,
            fontWeight: FontWeight.bold,
          ),
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: const Color(0xFFF9F9F9),
      body: Padding(
        padding: const EdgeInsets.all(34),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          children: <Widget>[
            const SizedBox(height: 20),
            DatePicker(
              DateTime(DateTime.now().year, DateTime.now().month, 1),
              initialSelectedDate: _selectedValue,
              selectionColor: const Color(0xFF222222),
              selectedTextColor: Colors.white,
              locale: "pt_BR",
              onDateChange: (date) {
                setState(() {
                  _selectedValue = date;
                });
              },
            ),
            const SizedBox(height: 20),
            Text(
              DateFormat(
                "dd 'de' MMMM 'de' yyyy",
                "pt_BR",
              ).format(_selectedValue),
              style: const TextStyle(fontSize: 18),
            ),
            const SizedBox(height: 20),
            // Consumer para atualizar os ListTile quando os dados mudarem
            Consumer<HabitoViewModel>(
              builder: (context, habitoViewModel, child) {
                final formattedDate = DateFormat(
                  "yyyy-MM-dd",
                ).format(_selectedValue);
                final currentHabit = habitoViewModel.habitos.firstWhere(
                  (h) =>
                      DateFormat("yyyy-MM-dd").format(h.dateTracker) ==
                      formattedDate,
                  orElse:
                      () => Habito(
                        id: 0,
                        dateTracker: DateTime.parse(formattedDate),
                        contadorRefeicao: 0,
                        contadorExercicio: 0,
                        contadorAgua: 0,
                        contadorPomodoro: 0,
                      ),
                );

                return Column(
                  children: [
                    _buildTile(
                      'Bebeu Água',
                      currentHabit.contadorAgua,
                      const Color(0xFF379392),
                      Icons.local_drink,
                    ),
                    const SizedBox(height: 10),
                    _buildTile(
                      'Sessões Pomodoro',
                      currentHabit.contadorPomodoro,
                      const Color(0xFFC30232),
                      Icons.timer,
                    ),
                    const SizedBox(height: 10),
                    _buildTile(
                      'Pausas para Exercícios',
                      currentHabit.contadorExercicio,
                      const Color(0xFF7067CF),
                      Icons.fitness_center,
                    ),
                    const SizedBox(height: 10),
                    _buildTile(
                      'Refeições Feitas',
                      currentHabit.contadorRefeicao,
                      const Color(0xFFFF8811),
                      Icons.fastfood,
                    ),
                  ],
                );
              },
            ),
          ],
        ),
      ),
    );
  }
}
