const mongoose = require('mongoose');
const util = require('util');
const exec = util.promisify(require('child_process').exec);
const tls = require('tls');
const fs = require('fs');
const { Client } = require('ssh2');
const net = require('net');
const jwts = require('./verificarJWT');
const sshKey = require('./sshKeyModel');

const proxyDbConnection = mongoose.createConnection('mongodb://localhost:27017/proxies');

const proxieSchema = new mongoose.Schema({
  ip: String,
  certificatePEM: String,
  randint: String,
  clientJwt: {
    type: String,
    default: ""
  },
   assigned: {
    type: Boolean,
    default: false // Establece el valor inicial de assigned como false
  }
});

function testSSHConnectionWithPrivateKey(host, privateKeyPath) {
  return new Promise((resolve, reject) => {
    const sshClient = new Client();

    sshClient.on('ready', () => {
      console.log('Conexión SSH exitosa');
      sshClient.end();
      resolve(true);
    });

    sshClient.on('error', (err) => {
      console.error('Error en la conexión SSH:', err.message);
      sshClient.end();
      reject(err);
    });

    const privateKey = require('fs').readFileSync(privateKeyPath);
    console.log("privateKey")
    const sshConfig = {
      host,
      port: 443,
      username :"ClientP2P",
      privateKey,
    };

    sshClient.connect(sshConfig);
  });
}

proxieSchema.statics.storeNewProxy = async function (ip, certificatePEM, token, randint) {
  try {
    await testSSHConnectionWithPrivateKey(ip, "./.ssh/id_rsa");
    console.log('Prueba de conexión SSH con clave privada exitosa');
    console.log("New Proxy ip: ", ip)
    console.log("Rand int got: ", randint)

    const proxyIp = new this({
      ip,
      certificatePEM,
      randint
    });

    await proxyIp.save();
    jwts.almacenarJWT(token);

    return true;
  } catch (error) {
    console.error('Error al almacenar proxyIp:', error);
    return false;
  }
};



proxieSchema.statics.assignProxyToIp = async function (ipClient,publicKey, token) {
  try {
    // Buscar un proxy aleatorio que no esté asignado
    const randomUnassignedProxy = await proxies.findOne({ assigned: false });
    if (!randomUnassignedProxy) {
      console.log('No se encontraron proxies no asignados.');
      return false;
    }
    const proxyIp = randomUnassignedProxy.ip;
    console.log('Proxy no asignado encontrado:', proxyIp);
    const randint = randomUnassignedProxy.randint;
    console.log('Rand int of proxy:', randint)
    // Realizar la prueba de conexión SSH
    try {
      const options = {
        host: proxyIp,
        port: 12345,
        rejectUnauthorized: false,
        secureProtocol: 'TLSv1_2_method', // Especifica la versión del protocolo aquí
      };

      const jsonToSend = {
        publicKey: publicKey,
        randint: randint,
      };
      const client = tls.connect(options, () => {
        console.log('Conectado al servidor');
        const jsonString = JSON.stringify(jsonToSend);
        client.write(jsonString);
      });
      client.on('data', (data) => {
        console.log('Datos recibidos:', data.toString());
        client.end();
      });
      client.on('error', (err) => {
        console.error('Error en la conexión TLS:', err);
      });

      // Marcar el proxy como asignado en la base de datos
      randomUnassignedProxy.assigned = true;
      randomUnassignedProxy.clientJwt = token;
      await randomUnassignedProxy.save();
      jwts.almacenarJWT(token);
      await sshKey.updateOne({ publicKey: publicKey }, { assigned: true });
      return proxyIp;
    } catch (error) {
      console.error('Prueba de conexión SSH con clave privada fallida:', error);
    }
    
  } catch (error) {
    console.error('Error al buscar proxy no asignado:', error);
  }
}

proxieSchema.statics.unassignProxy = async function (token) {
  try {
    // Buscar un proxy aleatorio que no esté asignado
    const assignedProxy = await proxies.findOne({ clientJwt: token});

    if (!assignedProxy) {
      console.log('No se encontraron proxies no asignados.');
      return false;
    }

    try {
      // Marcar el proxy como asignado en la base de datos
      assignedProxy.assigned = false;
      assignedProxy.clientJwt = "";
      await assignedProxy.save();
      return true;
    } catch (error) {
      console.error('Prueba de conexión SSH con clave privada fallida:', error);
    }
  } catch (error) {
    console.error('Error al buscar proxy no asignado:', error);
  }
}

const proxies = proxyDbConnection.model('proxies', proxieSchema);

module.exports = proxies;