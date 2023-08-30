const express = require('express');
const mongoose = require('mongoose');
const SSHKey = require('./models/sshKeyModel');
const proxies = require('./models/proxiesModel');
const fs = require('fs');
const bodyParser = require('body-parser');
const app = express();
app.use(bodyParser.json());

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
const dbURIProxies = 'mongodb://localhost:27017/proxies';
mongoose.connect(dbURI, { useNewUrlParser: true, useUnifiedTopology: true })
  .then(() => {
    console.log('Conexión a MongoDB establecida.');
    setInterval(verifyLapsedKeys, 30000);
    app.listen(PORT, () => {
      console.log(`Servidor Express en ejecución en el puerto ${PORT}`);

      // Ruta para generar y almacenar las claves SSH  DEPRECATED!!!!
      /*app.post('/generate-key', async (req, res) => {
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
      */
      app.post('/subscribe-proxy', async (req, res) => {
      try {
        const clientIp = req.ip;
        // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
        const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
        const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;
        const existingProxie = await proxies.findOne({ ip: ipv4 });
        console.log("Ip exists? : ", existingProxie)
        if (existingProxie) {
          console.log('El proxie ya esta suscrito');
          return res.status(403).json({ error: 'El proxie ya esta suscrito'});
        }else{
          const result = await proxies.storeNewProxy(ipv4);
          if (result == true){
             res.status(201).json({ message: 'Proxy suscrito exitosamente.' });
          }
          else{
             res.status(500).json({ message: 'Proxy not trusted' });
          }
        }
      } catch (error) {
        console.error('Error al generar y almacenar las claves SSH:', error); // Agrega esta línea
        res.status(500).json({ error: 'Error al generar y almacenar las claves SSH' });
      }
    });
    app.post('/unsubscribe-proxy', async (req, res) => {
      try {
        const clientIp = req.ip;
        // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
        const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
        const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;
        const result = await proxies.deleteOne({ ip: ipv4 });
        if (result.deletedCount === 1) {
          console.log('El proxie ha sido eliminado');
          return res.status(200).json({ message: 'Proxy successfully unsubscribed'});
        }else{
          res.status(500).json({ message: 'Proxy couldnt be unsubscribed!!!' });
        }
      } catch (error) {
        console.error('Error al generar y almacenar las claves SSH:', error); // Agrega esta línea
        res.status(500).json({ error: 'Error al generar y almacenar las claves SSH' });
      }
    });
    app.post('/get-server-public-key', (req, res) => {
        try {
            console.log("GOT PETITON OF PUBLIC KEY");
            const serverPublicKey = fs.readFileSync('./.ssh/id_rsa.pub', 'utf8');
            res.status(200).json({ serverPublicKey: serverPublicKey });
        } catch (error) {
            console.error('Error al obtener la clave pública del servidor:', error);
            res.status(500).json({ error: 'Error al obtener la clave pública del servidor' });
        }
    });
    app.post('/assign_proxy', (req, res) => {
      try {
        console.log("GOT ASK FOR PROXY");
        
        const publicKeyContent = req.body.public_key_content; // Accede al contenido del cuerpo JSON
        console.log('Public Key Content:', publicKeyContent);

        // Aquí puedes hacer lo que necesites con publicKeyContent
        
        res.status(200).json({ message: "test" });
      } catch (error) {
        console.error('Error obtaining pubkey of client', error);
        res.status(500).json({ error: 'Error obtaining pubkey of client' });
      }
    });
    })
  })
module.exports = app;