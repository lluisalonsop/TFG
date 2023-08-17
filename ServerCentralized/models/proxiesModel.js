const mongoose = require('mongoose');
const util = require('util');
const exec = util.promisify(require('child_process').exec);
const fs = require('fs');

const proxyDbConnection = mongoose.createConnection('mongodb://localhost:27017/proxies');

const proxieSchema = new mongoose.Schema({
  ip: String,
   assigned: {
    type: Boolean,
    default: false // Establece el valor inicial de assigned como false
  }
});

proxieSchema.statics.storeNewProxy = async function (ip) {
  try {
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