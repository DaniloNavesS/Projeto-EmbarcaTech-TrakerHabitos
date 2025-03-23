const { Sequelize } = require('sequelize');
require('dotenv').config();

// Inicializacao de banco de dados
const db = new Sequelize(process.env.DATABASE_DB, process.env.DATABASE_USER, process.env.DATABASE_PASSWORD, {
    host: process.env.DATABASE_HOST,
    dialect: 'postgres',
    logging: false
});

module.exports = db;
