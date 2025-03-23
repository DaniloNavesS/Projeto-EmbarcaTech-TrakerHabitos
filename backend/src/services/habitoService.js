const db = require('../config/db');
const Habito = require('../models/habitoModel');

const sicronizarHabito = async ({data_recebida ,contador_agua, contador_exercicio, contador_alimentacao, contador_sessao_pomodoro}) => {
    if (!data_recebida) {
        data_recebida = new Date().toISOString().split('T')[0];
    }

    try {
        let habito = await Habito.findOne({ where: { date_tracker: data_recebida } });
      
        if (habito) {
          await habito.increment({
            contador_agua: contador_agua,
            contador_exercicio: contador_exercicio,
            contador_refeicao: contador_alimentacao,
            contador_pomodoro: contador_sessao_pomodoro
          });
          habito = await habito.reload();
          return habito;
        } else {
          habito = await Habito.create({
            date_tracker: data_recebida,
            contador_agua: contador_agua,
            contador_exercicio: contador_exercicio,
            contador_refeicao: contador_alimentacao,
            contador_pomodoro: contador_sessao_pomodoro
          });
          return habito;
        }
      } catch (error) {
        throw error;
      }
}

const listarHabito = async () => {
  try {
    const habitos = await Habito.findAll({
      order: [['date_tracker', 'ASC']]
    });
    return habitos;
  } catch (error) {
    console.error('Erro ao listar hábitos:', error);
    throw error;
  }
};


module.exports = { sicronizarHabito, listarHabito};
