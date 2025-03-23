class Habito {
  final int id;
  final DateTime dateTracker;
  final int contadorRefeicao;
  final int contadorExercicio;
  final int contadorAgua;
  final int contadorPomodoro;

  Habito({
    required this.id,
    required this.dateTracker,
    required this.contadorRefeicao,
    required this.contadorExercicio,
    required this.contadorAgua,
    required this.contadorPomodoro,
  });

  // Converte um Map (JSON) para uma instância de Habito
  factory Habito.fromJson(Map<String, dynamic> json) {
    return Habito(
      id: json['id'],
      dateTracker: DateTime.parse(json['date_tracker']),
      contadorRefeicao: json['contador_refeicao'],
      contadorExercicio: json['contador_exercicio'],
      contadorAgua: json['contador_agua'],
      contadorPomodoro: json['contador_pomodoro'],
    );
  }

  // Converte a instância de Habito para um Map (JSON)
  Map<String, dynamic> toJson() {
    return {
      'id': id,
      'date_tracker': dateTracker.toIso8601String().split('T')[0],
      'contador_refeicao': contadorRefeicao,
      'contador_exercicio': contadorExercicio,
      'contador_agua': contadorAgua,
      'contador_pomodoro': contadorPomodoro,
    };
  }
}