const express = require('express');
const mongoose = require('mongoose');
const morgan = require('morgan');

const app = express();
const port = 3000;

// Connexion à la base de données MongoDB
mongoose.connect('mongodb://root:password@localhost:27017', { 
  useNewUrlParser: true,
  authSource: "admin"
})
  .then(() => console.log('Connexion à MongoDB réussie !'))
  .catch(() => console.log('Connexion à MongoDB échouée !'));

// Définition du modèle de données pour le capteur
const capteurSchema = new mongoose.Schema({
  nom: String,
  temperature: Number
});
const Capteur = mongoose.model('Capteur', capteurSchema);

app.use(express.json());
app.use(morgan('combined'));

app.get('/capteurs', async (req, res) => {
    try {
      const capteurs = await Capteur.find();
      res.json(capteurs);
    } catch (err) {
      res.status(500).json({ message: 'Une erreur est survenue lors de la récupération des capteurs' });
    }
  });

  app.get('/capteurs/:id', async (req, res) => {
    const { id } = req.params;
    try {
      const capteur = await Capteur.findById(id);
      if (!capteur) {
        return res.status(404).json({ message: 'Aucun capteur trouvé avec cet ID' });
      }
      res.json(capteur);
    } catch (err) {
      res.status(500).json({ message: 'Une erreur est survenue lors de la récupération du capteur' });
    }
  });
  

  app.put('/capteurs/:id', async (req, res) => {
    const { id } = req.params;
    const { temperature } = req.body;
  
    if (!temperature) {
      return res.status(400).json({ message: 'La température est requise' });
    }
  
    // Recherche, mise à jour et sauvegarde du capteur dans la base de données
    try {
      const capteur = await Capteur.findById(id);
      if (!capteur) {
        return res.status(404).json({ message: 'Aucun capteur trouvé avec cet ID' });
      }
      capteur.temperature = temperature;
      await capteur.save();
      res.json({ message: 'Les données du capteur ont été mises à jour avec succès' });
    } catch (err) {
      res.status(500).json({ message: 'Une erreur est survenue lors de la mise à jour du capteur' });
    }
  });
  
app.listen(port, () => {
  console.log(`Serveur lancé sur http://localhost:${port}`);
});
