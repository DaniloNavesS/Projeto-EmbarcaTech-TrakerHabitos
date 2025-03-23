const express = require('express');
const router = express.Router();
const habitosController = require('../controllers/habitoController');

router.post('/sicronizar', habitosController.sicronizarHabito);
router.get('/listar', habitosController.listarHabito);

module.exports = router;
