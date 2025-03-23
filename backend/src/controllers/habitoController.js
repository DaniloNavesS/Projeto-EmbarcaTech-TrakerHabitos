const habitoService = require('../services/habitoService');

const sicronizarHabito = async (req, res) => {
    try {
        // Data da requisicao
        const now = new Date();
        const data_recebida = now.toISOString().split('T')[0];

        const {contador_agua, contador_exercicio, contador_alimentacao, contador_sessao_pomodoro} = req.body;
        const response = await habitoService.sicronizarHabito({data_recebida ,contador_agua, contador_exercicio, contador_alimentacao, contador_sessao_pomodoro});
        res.json(response);
    } catch (error) {
        res.status(401).json({ error: error.message });
    }
};


const listarHabito = async (req, res) => {
    try {
        const response = await habitoService.listarHabito();
        res.json(response);
    } catch (error) {
        res.status(401).json({ error: error.message });
    }
}





module.exports = {sicronizarHabito, listarHabito};
