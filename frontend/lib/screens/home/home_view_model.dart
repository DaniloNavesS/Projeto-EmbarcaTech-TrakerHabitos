import 'package:flutter/material.dart';
import '../../model/habitos_model.dart';
import '../../services/habitos_service.dart';

class HabitoViewModel extends ChangeNotifier {
  final HabitoService habitoService;
  List<Habito> _habitos = [];

  List<Habito> get habitos => _habitos;

  HabitoViewModel({required this.habitoService});

  // Função para carregar os hábitos e notificar a view
  Future<void> carregarHabitos() async {
    try {
      _habitos = await habitoService.listarHabitos();
      notifyListeners();
    } catch (error) {
      print('Erro ao carregar hábitos: $error');
      // Você pode tratar o erro como preferir (ex.: setar uma variável de erro)
    }
  }
}
