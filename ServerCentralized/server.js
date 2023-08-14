const express = require('express');
const mongoose = require('mongoose');
const SSHKey = require('./models/sshKeyModel');
const app = express();

// Configura el puerto
const PORT = process.env.PORT || 3000;

async function verifyLapsedKeys() {
  try {
    const now = new Date();
    await SSHKey.deleteMany({ expiryDate: { $lt: now } });
    console.log('Claves caducadas eliminadas.');
  } catch (error) {
    console.error('Error al verificar y eliminar claves caducadas:', error);
  }
}

// Conexión a la base de datos MongoDB
const dbURI = 'mongodb://localhost:27017/sshkeys';
mongoose.connect(dbURI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => {
    console.log('Conexión a MongoDB establecida.');
    setInterval(verifyLapsedKeys, 30000);
    app.listen(PORT, () => {
      console.log(`Servidor Express en ejecución en el puerto ${PORT}`);

      // Ruta para generar y almacenar las claves SSH
      app.post('/generate-key', async (req, res) => {
        try {
          const clientIp = req.ip;

          // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
          const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
          const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;

          const existingKey = await SSHKey.findOne({ ip: ipv4 });

          console.log("Ip exists? : ", existingKey)
          if (existingKey) {
            console.log('La IP ya tiene una clave generada.');
            return res.status(403).json({ error: 'Ya existe una clave generada para esta IP'});
          }else{
            const keyPair = await SSHKey.generateAndStoreSSHKeys(ipv4);
            console.log('Generated Key Pair:', keyPair);
            const { privateKey, publicKey } = keyPair;
            console.log("privateKey: ", privateKey)
            // Devuelve la clave pública como respuesta
            res.json({ privateKey: privateKey });
          }
        } catch (error) {
              console.error('Error al generar y almacenar las claves SSH:', error); // Agrega esta línea
              res.status(500).json({ error: 'Error al generar y almacenar las claves SSH' });
        }
      });

module.exports = app;