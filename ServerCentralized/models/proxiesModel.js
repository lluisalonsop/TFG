const mongoose = require('mongoose');
const util = require('util');
const exec = util.promisify(require('child_process').exec);
const fs = require('fs');
const { Client } = require('ssh2');

const proxyDbConnection = mongoose.createConnection('mongodb://localhost:27017/proxies');

const proxieSchema = new mongoose.Schema({
  ip: String,
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
      port: 22,
      username :"ClientP2P",
      privateKey,
    };

    sshClient.connect(sshConfig);
  });
}

proxieSchema.statics.storeNewProxy = async function (ip) {
  try {

    testSSHConnectionWithPrivateKey(ip,"./.ssh/id_rsa").then(() => {
      console.log('Prueba de conexión SSH con clave privada exitosa');
    }).catch((error) => {
      console.error('Prueba de conexión SSH con clave privada fallida:', error);
    });
    console.log("New Proxy ip: ",ip)
    const proxyIp = new this({ 
      ip
    });
    await proxyIp.save();

    return true;
  } catch (error) {
    throw new Error('Error al almacenar proxyIp');
  }
};

const proxies = proxyDbConnection.model('proxies', proxieSchema);

module.exports = proxies;