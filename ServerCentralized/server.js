const express = require('express');
const bodyParser = require('body-parser');
const { MongoClient } = require('mongodb');
const { generateKeyPairSync } = require('crypto'); // Importamos el módulo crypto

const app = express();
const port = 3000;

app.use(bodyParser.json());

// Conexión a la base de datos
MongoClient.connect('mongodb://localhost:27017', { useUnifiedTopology: true }, (err, client) => {
    if (err) {
        console.error('Error connecting to MongoDB:', err);
        return;
    }

    const db = client.db('sshkeys');

    // Ruta para generar y almacenar la clave SSH
    app.post('/generate-ssh-key', (req, res) => {
        const { username } = req.body;

        // Generar la clave SSH aquí (usando el módulo 'crypto')
        const { publicKey, privateKey } = generateKeyPairSync('rsa', {
            modulusLength: 4096,
            publicKeyEncoding: {
                type: 'spki',
                format: 'pem'
            },
            privateKeyEncoding: {
                type: 'pkcs8',
                format: 'pem'
            }
        });

        // Almacena las claves en la base de datos (aquí deberías agregar el código para hacerlo)
        db.collection('keys').insertOne({ username, publicKey, privateKey }, (err, result) => {
            if (err) {
                console.error('Error inserting key into database:', err);
                res.status(500).json({ error: 'Internal server error' });
            } else {
                res.json({ message: 'SSH key generated and stored successfully' });
            }
        });
    });

    app.listen(port, () => {
        console.log(`Server is running on port ${port}`);
    });
});