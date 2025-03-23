require('dotenv').config();
const bodyParser = require('body-parser');
const express = require('express');
const cors = require('cors');


const habitosRoutes = require('./routes/habitoRoutes');

const app = express();
app.use(cors());
app.use(express.json());
app.use(bodyParser.json());

// Rotas
app.use('/habitos', habitosRoutes);


app.post('/api', (req, res) => {
  res.send('Hello World');
});




module.exports = app;