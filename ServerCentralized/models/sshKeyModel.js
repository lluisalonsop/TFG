const mongoose = require('mongoose');
const util = require('util');
const exec = util.promisify(require('child_process').exec);
const fs = require('fs');
const path = require('path');

const sshKeySchema = new mongoose.Schema({
  publicKey: String,
  //privateKey: String,
  ip: String,
  expiryDate: Date,
  assigned: String,
});
/* ------------------------DEPRECTED------------------------
sshKeySchema.statics.generateAndStoreSSHKeys = async function (ip) {
  try {
    const tmpDir = '/tmp/ssh-keys'; // Directorio temporal para almacenar las claves

    // Crear el directorio temporal si no existe
    if (!fs.existsSync(tmpDir)) {
      fs.mkdirSync(tmpDir);
    }

    // Ejecutar el comando ssh-keygen para generar las claves
    const { stdout, stderr } = await exec(`ssh-keygen -t rsa -b 4096 -N "" -f ${path.join(tmpDir, 'id_rsa')}`);

    // Leer la clave pública generada
    const publicKey = fs.readFileSync(path.join(tmpDir, 'id_rsa.pub'), 'utf8');
    const privateKey  = fs.readFileSync(path.join(tmpDir, 'id_rsa'),'utf8');
    // Eliminar las claves cuando ya no sean necesarias
    fs.unlinkSync(path.join(tmpDir, 'id_rsa'));
    fs.unlinkSync(path.join(tmpDir, 'id_rsa.pub'));
    fs.rmdirSync(tmpDir);

    // Crear y guardar el documento de clave en la base de datos
    console.log("Client ip(a_secas): ",ip)
    const sshKey = new this({ 
      publicKey,
      privateKey,
      ip,
      // 30 days: expiryDate: new Date(Date.now() + 30 * 24 * 60 * 60 * 1000),
      expiryDate: new Date(Date.now() + 60 * 1000),
    });
    await sshKey.save();

    return sshKey;
  } catch (error) {
    throw new Error('Error al generar y almacenar las claves SSH');
  }
};
*/

sshKeySchema.statics.storeKey = async function (ip,publicKey) {
  try {
    const sshKey = new this({ 
      publicKey,
      ip,
      // 30 days: expiryDate: new Date(Date.now() + 30 * 24 * 60 * 60 * 1000),
      expiryDate: new Date(Date.now() + 60 * 1000),
      assigned: false
    });
    await sshKey.save();

    return true;
  } catch (error) {
    throw new Error('Error al generar y almacenar las claves SSH');
  }
};


const SSHKey = mongoose.model('SSHKey', sshKeySchema);

module.exports = SSHKey;