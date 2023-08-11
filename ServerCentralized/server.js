const express = require('express');
const mongoose = require('mongoose');
const SSHKey = require('./models/sshKeyModel');
const Ejemplo = require('./models/ejemplo'); // Asegúrate de ajustar la ruta correcta
const app = express();

// Configura el puerto
const PORT = process.env.PORT || 3000;

// Conexión a la base de datos MongoDB
const dbURI = 'mongodb://localhost:27017/sshkeys';
mongoose.connect(dbURI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => {
    console.log('Conexión a MongoDB establecida.');
    app.listen(PORT, () => {
      console.log(`Servidor Express en ejecución en el puerto ${PORT}`);

      // Ruta para generar y almacenar las claves SSH
      app.post('/generar-claves', async (req, res) => {
        try {

          const keyPair = await SSHKey.generateAndStoreSSHKeys();
          console.log('Generated Key Pair:', keyPair);
          const { privateKey, publicKey } = keyPair;
          console.log("privateKey: ", privateKey)
          // Almacena las claves en la base de datos
          //const ejemplo = new Ejemplo({
          //  privateKey,
          //  publicKey,
          //});
          //await ejemplo.save();
          // Devuelve la clave pública como respuesta
          res.json({ publicKey });
        } catch (error) {
              console.error('Error al generar y almacenar las claves SSH:', error); // Agrega esta línea
              res.status(500).json({ error: 'Error al generar y almacenar las claves SSH' });
        }
      });
    });
  })
  .catch(err => console.error('Error de conexión a MongoDB:', err));

// Agrega las rutas a la aplicación
//app.use('/', routes);

// ... (middleware y configuraciones adicionales)

module.exports = app;