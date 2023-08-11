const mongoose = require('mongoose');

const Schema = mongoose.Schema;

const ejemploSchema = new Schema({
  privateKey: String,
  publicKey: String,
  // ... otros campos si es necesario
});

const Ejemplo = mongoose.model('Ejemplo', ejemploSchema);

module.exports = Ejemplo;