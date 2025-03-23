import 'dart:convert';
import 'package:http/http.dart' as http;
import '../model/habitos_model.dart';

class HabitoService {
  final String baseUrl;

  HabitoService({required this.baseUrl});

  // Função que faz uma requisição GET para listar todos os hábitos
  Future<List<Habito>> listarHabitos() async {
    final response = await http.get(Uri.parse('$baseUrl/habitos/listar'));
    
    if (response.statusCode == 200) {
      // Supondo que a resposta seja uma lista JSON
      final List<dynamic> jsonList = json.decode(response.body);
      return jsonList.map((jsonItem) => Habito.fromJson(jsonItem)).toList();
    } else {
      throw Exception('Falha ao carregar hábitos');
    }
  }
}
