const { DataTypes } = require('sequelize');
const sequelize = require('../config/db');

const Habito = sequelize.define('tb_habitos', {
    id: {
        type: DataTypes.INTEGER,
        autoIncrement: true,
        primaryKey: true
    },
    date_tracker: {
        type: DataTypes.DATE,
        allowNull: false
    },
    contador_refeicao: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    contador_exercicio: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    contador_agua: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
    contador_pomodoro: {
        type: DataTypes.INTEGER,
        allowNull: false
    },
}, {
    tableName: 'tb_habitos',
    timestamps: false
  }
);

module.exports = Habito;
