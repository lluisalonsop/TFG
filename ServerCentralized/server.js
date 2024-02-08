const express = require('express');
const mongoose = require('mongoose');
const SSHKey = require('./models/sshKeyModel');
const proxies = require('./models/proxiesModel');
const jwts = require('./models/verificarJWT');
const fs = require('fs');
const bodyParser = require('body-parser');
const https = require('https');
const jwt = require('jsonwebtoken');
const app = express();
app.use(bodyParser.json());

const PORT = process.env.PORT || 3000;
const secretJWTPass = "Sup3rS3cr3tK3yJWT";

// Lectura de las claves privadas y certificados
const privateKey = fs.readFileSync('/home/lluis/Documents/utils/key.pem', 'utf8');
const certificate = fs.readFileSync('/home/lluis/Documents/utils/cert.pem', 'utf8');
const credentials = { key: privateKey, cert: certificate };

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

    // Creación del servidor HTTPS
    const httpsServer = https.createServer(credentials, app);

    //app.listen(PORT, () => {
    //  console.log(`Servidor Express en ejecución en el puerto ${PORT}`);
    httpsServer.listen(PORT, () => {
      console.log(`Servidor Express en ejecución en el puerto ${PORT}`);
      app.post('/jwt', async (req, res) => {
        try {
          const clientIp = req.ip;
          // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
          const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
          const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;
          // Verificar si ya existe un JWT para la IP
          const jwtRecuperado = await jwts.obtenerJWTPorIP(ipv4);
          console.log('JWT Recuperado:', jwtRecuperado);
          if (jwtRecuperado) {
            // Devolver el JWT existente
            res.status(200).json({ jwt: jwtRecuperado });
          } else {
            // Generar un nuevo JWT
            const nuevoToken = jwt.sign({ ip: ipv4 }, secretJWTPass); // Cambia 'secreto' por tu clave secreta

            // Guardar o actualizar el JWT en tu lógica de almacenamiento (puedes usar una base de datos para esto)
            jwts.almacenarJWT(ipv4, nuevoToken);

            // Devolver el nuevo JWT
            res.status(200).json({ jwt: nuevoToken });
          }
        } catch (error) {
          console.error('Error al manejar la solicitud de JWT:', error);
          res.status(500).json({ error: 'Error al manejar la solicitud de JWT' });
        }
    });
    app.post('/subscribe-proxy', async (req, res) => {
      try {
        const certificatePem = req.body && req.body.certificatePem;
        if (!certificatePem) {
          return res.status(400).json({ error: 'Certificado PEM no proporcionado en el cuerpo de la solicitud.' });
        }
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
          const token = jwt.sign({ ip: ipv4, certificatePem, role: "proxy"}, secretJWTPass);
          const result = await proxies.storeNewProxy(ipv4,certificatePem,token,req.body.randint);
          if (result == true){
             res.status(201).json({ message: 'Proxy suscrito exitosamente.', tokenProxy: token });
          }
          else{
            console.log(result)
             res.status(500).json({ message: 'Proxy not trusted' });
          }
        }
      } catch (error) {
        console.error('Error al generar y almacenar las claves SSH:', error); // Agrega esta línea
        res.status(500).json({ error: 'Error al generar y almacenar las claves SSH' });
      }
    });
    app.post('/unsubscribe-proxy',jwts.verificarJWT, async (req, res) => {
      try {
        const clientIp = req.ip;
        // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
        const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
        const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;
        const result = await proxies.deleteOne({ ip: ipv4 });
        if (result.deletedCount === 1) {
          console.log('El proxie ha sido eliminado');
          const token = req.headers.authorization.split(' ')[1];
          jwts.eliminarJWT(token);
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
    app.post('/assign_proxy', async (req, res) => {
      try {
        console.log("GOT ASK FOR PROXY");
        
        const publicKeyContent = req.body && req.body.public_key_content; // Accede al contenido del cuerpo JSON
        if (!publicKeyContent){
          return res.status(400).json({error: 'Clave ssh pública no proporcionada'})
        }

        const clientIp = req.ip;

        // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
        const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
        const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;
        
        const existingKey = await SSHKey.findOne({ ip: ipv4, publicKey: publicKeyContent});
        if (existingKey) {
          if (existingKey.assigned == true){
            res.status(401).json({ error: "Error, client's key already in a a proxy..."});
          }
        }else{
          const result = await SSHKey.storeKey(ipv4 , publicKeyContent);
          if (!result){
            res.status(500).json({ error: 'Error storing Key' });
          }
        }
        const token = jwt.sign({ ip: ipv4, publicKeyContent, role: "client"}, secretJWTPass);
        const assigned = await proxies.assignProxyToIp(ipv4,publicKeyContent, token);
        if (assigned){
          res.status(200).json({message : "Proxy ip: "+ assigned,tokenClient: token})
        }else{
          res.status(503).json({message :"No proxy avaible sorry..."})
        }
      } catch (error) {
          console.error('Error obtaining pubkey of client', error);
          res.status(500).json({ error: 'Error obtaining pubkey of client' });
        }
      });
    app.post('/unassign_proxy',jwts.verificarJWT, async (req, res) => {
      try {
        const clientIp = req.ip;

        // Extracción de la dirección IPv4 de la cadena si está en formato IPv6 mapeado
        const ipv4Match = clientIp.match(/::ffff:(\d+\.\d+\.\d+\.\d+)/);
        const ipv4 = ipv4Match ? ipv4Match[1] : clientIp;

        token = jwts.obtenerTokenDeEncabezado(req.headers.authorization)

        const result = await proxies.unassignProxy(token);

        if (result){
          res.status(200).json({message : "Proxy successfully unassigned"})
        }else{
          res.status(400).json({message: "Proxy not FOUND!!!!"})
        }    
      } catch (error) {
          console.error('Error obtaining pubkey of client', error);
          res.status(500).json({ error: 'Error obtaining pubkey of client' });
        }
      })
    });
  })
module.exports = app;