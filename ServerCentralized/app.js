const express = require('express');
const bodyParser = require('body-parser');
const MongoClient = require('mongodb').MongoClient;

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
        const { username, key } = req.body;

        db.collection('keys').insertOne({ username, key }, (err, result) => {
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