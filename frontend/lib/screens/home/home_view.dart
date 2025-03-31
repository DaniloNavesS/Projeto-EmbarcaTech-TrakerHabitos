import 'package:flutter/material.dart';
import 'package:intl/intl.dart';
import 'package:provider/provider.dart';
import '../../model/habitos_model.dart';
import '../home/home_view_model.dart';
import 'package:easy_date_timeline/easy_date_timeline.dart';
import 'package:material_symbols_icons/material_symbols_icons.dart';

class HomeView extends StatefulWidget {
  const HomeView({super.key});

  @override
  State<HomeView> createState() => _HomeViewState();
}

class _HomeViewState extends State<HomeView> {
  final EasyDatePickerController _controller = EasyDatePickerController();
  DateTime _selectedValue = DateTime(
    DateTime.now().year,
    DateTime.now().month,
    DateTime.now().day,
  );

  @override
  void initState() {
    super.initState();
    WidgetsBinding.instance.addPostFrameCallback((_) {
      _controller.jumpToCurrentDate(); // Centraliza a data atual
    });
  }

  Widget _buildTile(String title, int value, Color color, IconData icon) {
    return ListTile(
      contentPadding: const EdgeInsets.symmetric(horizontal: 16, vertical: 24),
      tileColor: Color(0xFFf5f5f5),
      shape: RoundedRectangleBorder(borderRadius: BorderRadius.circular(12)),
      title: Text(
        title,
        style: const TextStyle(
          fontSize: 16,
          fontWeight: FontWeight.bold,
          color: Color(0xFF222222),
        ),
      ),

      trailing: Container(
        width: 80,
        height: 40,
        alignment: Alignment.center,
        padding: const EdgeInsets.symmetric(horizontal: 8),
        decoration: BoxDecoration(
          color: Colors.transparent,
          borderRadius: BorderRadius.circular(8),
        ),
        child: Row(
          mainAxisAlignment: MainAxisAlignment.center,
          children: [
            Icon(icon, color: color, size: 20),
            SizedBox(width: 6),
            Text(
              value.toString(),
              style: TextStyle(
                fontSize: 18,
                color: color,
                fontWeight: FontWeight.w600,
              ),
            ),
          ],
        ),
      ),
    );
  }

  @override
  Widget build(BuildContext context) {
    return Scaffold(
      backgroundColor: Color(0xFFFFFFFF),
      body: Padding(
        padding: const EdgeInsets.all(34),
        child: Column(
          crossAxisAlignment: CrossAxisAlignment.center,
          children: <Widget>[
            SizedBox(height: 20),
            AppBar(
              backgroundColor: Colors.transparent,
              elevation: 0,
              title: const Text(
                'Meus Hábitos',
                style: TextStyle(
                  color: Color(0xFF222222),
                  fontSize: 24,
                  fontWeight: FontWeight.w600,
                ),
              ),
              centerTitle: true,
            ),

            Theme(
              data: Theme.of(context).copyWith(
                colorScheme: ColorScheme.fromSwatch(
                  backgroundColor: Color(0xFFf5f5f5), // Cor de fundo do tema
                ).copyWith(primary: Color(0xFF222222)), //Cor primária do tema
              ),
              child: EasyTheme(
                data: EasyTheme.of(context).copyWith(
                  monthBackgroundColor: WidgetStateProperty.resolveWith((
                    states,
                  ) {
                    if (states.contains(WidgetState.selected)) {
                      return Color(
                        0xFF222222,
                      ); // Cor de fundo do mês selecionado
                    }
                    return Colors.transparent; // Cor de fundo padrão
                  }),

                  monthForegroundColor: WidgetStateProperty.resolveWith((
                    states,
                  ) {
                    if (states.contains(WidgetState.selected)) {
                      return Colors.white; // Cor do texto do mês selecionado
                    }
                    return Color(0xFF222222); // Cor do texto padrão
                  }),

                  monthBorder: WidgetStateProperty.resolveWith((states) {
                    if (states.contains(WidgetState.selected)) {
                      return const BorderSide(color: Colors.transparent); 
                    }
                    return const BorderSide(color: Colors.transparent);
                  }),

                  currentMonthBackgroundColor: WidgetStateProperty.resolveWith((
                    states,
                  ) {
                    if (states.contains(WidgetState.selected)) {
                      return Color(0xFF222222); // Cor de fundo do mês atual
                    }
                    return Colors.transparent; // Cor de fundo padrão
                  }),

                  currentMonthForegroundColor: WidgetStateProperty.resolveWith((
                    states,
                  ) {
                    if (states.contains(WidgetState.selected)) {
                      return Colors.white; // Cor do texto do mês selecionado
                    }
                    return Color(0xFF222222); // Cor do texto padrão
                  }),
                  currentMonthBorder: WidgetStateProperty.all(
                    const BorderSide(color: Color(0xFF222222)), // Cor da borda do mês atual
                  ),

                  dayBackgroundColor: WidgetStateProperty.resolveWith((states) {
                    if (states.contains(WidgetState.selected)) {
                      return Color(0xFF222222); // Cor de fundo do dia selecionado
                    } else if (states.contains(WidgetState.disabled)) {
                      return Color(0xFFf5f5f5); // Cor de fundo do dia desabilitado
                    }
                    return Color(0xFFf5f5f5); // Cor de fundo padrão
                  }),
                  dayForegroundColor: WidgetStateProperty.resolveWith((states) {
                    if (states.contains(WidgetState.selected)) {
                      return Colors.white;
                    }
                    return Color(0xFF222222); // Cor do texto padrão
                  }),
                  dayBorder: WidgetStateProperty.resolveWith((states) {
                    if (states.contains(WidgetState.selected)) {
                      return const BorderSide(color: Colors.transparent);
                    }
                    return const BorderSide(color: Colors.transparent);
                  }),
                  dayShape: WidgetStateProperty.all(
                    RoundedRectangleBorder(
                      borderRadius: BorderRadius.circular(8),
                    ),
                  ),
                  currentDayBackgroundColor: WidgetStateProperty.resolveWith((
                    states,
                  ) {
                    if (states.contains(WidgetState.selected)) {
                      return Color(0xFF222222); // Cor de fundo do dia atual
                    }
                    return Colors.transparent;
                  }),
                  currentDayForegroundColor: WidgetStateProperty.resolveWith((
                    states,
                  ) {
                    if (states.contains(WidgetState.selected)) {
                      return Colors.white; // Cor do texto do dia atual
                    }
                    return Color(0xFF222222); // Cor do texto padrão
                  }),
                  currentDayBorder: WidgetStateProperty.all(
                    const BorderSide(color: Color(0xFF222222)), // Cor da borda do dia atual
                  ),
                ),
                child: EasyDateTimeLinePicker(
                  controller: _controller,
                  locale: const Locale("pt", "BR"),
                  focusedDate: _selectedValue,
                  firstDate: DateTime(
                    DateTime.now().year,
                    DateTime.now().month,
                    1,
                  ),
                  lastDate: DateTime(2030, 3, 18),
                  onDateChange: (date) {
                    setState(() {
                      _selectedValue = date;
                    });
                  },
                ),
              ),
            ),

            const SizedBox(height: 20),
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
                    SizedBox(height: 20),
                    _buildTile(
                      'Copos de Água Bebidos',
                      currentHabit.contadorAgua,
                      Color(0xFF3A86FF),
                      Symbols.water_drop,
                    ),
                    SizedBox(height: 20),
                    _buildTile(
                      'Sessões Pomodoro Concluídas',
                      currentHabit.contadorPomodoro,
                      Color(0xFFE63946),
                      Symbols.alarm,
                    ),
                    SizedBox(height: 20),
                    _buildTile(
                      'Pausas para Exercícios',
                      currentHabit.contadorExercicio,
                      Color(0xFF06D6A0),
                      Symbols.exercise,
                    ),
                    SizedBox(height: 20),
                    _buildTile(
                      'Refeições Saudáveis Feitas',
                      currentHabit.contadorRefeicao,
                      Color(0xFFF4A261),
                      Symbols.lunch_dining,
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
