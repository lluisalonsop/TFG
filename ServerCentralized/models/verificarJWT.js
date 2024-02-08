const jwt = require('jsonwebtoken');
const mongoose = require('mongoose');

const jwtDbConnection = mongoose.createConnection('mongodb://localhost:27017/JWT');

// Define un esquema para los JWT
const jwtSchema = new mongoose.Schema({
  ip: String,
  token: String,
});

const JWTModel = jwtDbConnection.model('JWT', jwtSchema)

function verificarJWT(req, res, next) {
  const token = obtenerTokenDeEncabezado(req.headers.authorization);

  if (!token) {
    return res.status(401).json({ error: 'Token de autorización no proporcionado.' });
  }

  jwt.verify(token, 'Sup3rS3cr3tK3yJWT', (err, decoded) => {
    if (err) {
      return res.status(403).json({ error: 'Token de autorización inválido.' });
    }

    req.usuario = decoded; // Guarda la información decodificada en la solicitud para uso posterior
    next(); // Continúa con la ejecución de la siguiente función en la cadena de middleware
  });
}

function obtenerTokenDeEncabezado(authorizationHeader) {
  if (!authorizationHeader) {
    return null;
  }

  const partes = authorizationHeader.split(' ');

  if (partes.length !== 2 || partes[0] !== 'Bearer') {
    return null;
  }

  return partes[1];
}

// Función para obtener un JWT por IP
async function obtenerJWTPorIP(ip) {
  try {
    const resultado = await JWTModel.findOne({ ip });
    return resultado ? resultado.token : null;
  } catch (error) {
    console.error('Error al obtener el JWT por IP:', error);
    throw error;
  }
}

// Función para almacenar o actualizar un JWT por IP
async function almacenarJWT(jwtToken) {
  try {
    const nuevoJWT = new JWTModel({token: jwtToken });
    // Guarda el documento en la base de datos
    await nuevoJWT.save();
  } catch (error) {
    console.error('Error al almacenar o actualizar el JWT por IP:', error);
    throw error;
  }
}

async function eliminarJWT(token) {
  try {
    // Busca el documento por el token y elimínalo de la base de datos
    console.log("Intenando elminar el JWT:", token)
    await JWTModel.findOneAndDelete({ token });
  } catch (error) {
    console.error('Error al eliminar el JWT:', error);
    throw error;
  }
}

module.exports = {verificarJWT, obtenerJWTPorIP, almacenarJWT, eliminarJWT, obtenerTokenDeEncabezado};