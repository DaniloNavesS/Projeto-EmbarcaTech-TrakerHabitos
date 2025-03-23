require('dotenv').config(); // Importa as variáveis de ambiente do .env

const app = require('./app');
const sequelize = require('./config/db');

const PORT = process.env.PORT || 3000;

const startServer = async () => {
  try {
      await sequelize.authenticate();
      console.log('Conexão com o banco de dados estabelecida com sucesso.');

      app.listen(PORT, '0.0.0.0', () => console.log(`Servidor rodando na porta ${PORT}`));
  } catch (error) {
      console.error('Erro ao conectar ao banco:', error);
  }
};

startServer();

