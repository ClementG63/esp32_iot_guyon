import express from "express";
import morgan from "morgan";
import cron from "node-cron";
import mongoose from "mongoose";
import cors from "cors";
import fetchData from "./fetchData.js";

const app = express();

// Création du modèle Mongoose pour la configuration
const Configuration = mongoose.model(
  "Configuration",
  new mongoose.Schema({
    type: String,
  })
);

// Définition du modèle de données pour le capteur
export const Sensor = mongoose.model(
  "Sensor",
  new mongoose.Schema({
    nom: String,
    temperature: Number,
  })
);

export const SensorData = mongoose.model(
  "SensorData",
  new mongoose.Schema({
    device_id: String,
    temp_SOIL: String,
    conduct_SOIL: String,
    water_SOIL: String,
    date: Date,
    timestamp: Number,
  })
);

app.use(morgan("dev"));
app.use(express.json());
app.use(cors());

mongoose
  .connect("mongodb://root:password@localhost:27017/", {
    useNewUrlParser: true,
    useUnifiedTopology: true,
  })
  .then(() => {
    console.log("Connexion à la base de données réussie !");

    app.listen(3000, () => {
      console.log("API en écoute sur http://localhost:3000");
    });

    // Endpoint pour sauvegarder une configuration
    app.post("/configuration", async (req, res) => {
      try {
        const configuration = new Configuration(req.body);
        await configuration.save();
        res.status(201).json(configuration);
      } catch (err) {
        res.status(500).json({ message: err.message });
      }
    });

    // Endpoint pour récupérer la configuration
    app.get("/configuration", async (_, res) => {
      try {
        const configuration = await Configuration.find();
        res.json(configuration[0]);
      } catch (err) {
        res.status(500).json({ message: err.message });
      }
    });

    app.get("/ttn-sensors", async (req, res) => {
      try {
        let filter = {};
        if (req.query.date) {
          let [day, month, year] = req.query.date.split("/");
          let date = new Date(year, month - 1, day);

          let startOfDay = new Date(date);
          startOfDay.setHours(0, 0, 0, 0);

          let endOfDay = new Date(date);
          endOfDay.setHours(23, 59, 59, 999);

          filter.timestamp = { $gte: startOfDay, $lte: endOfDay };
        }

        const sensorData = await SensorData.find(filter);
        res.json(sensorData);
      } catch (err) {
        res.status(500).json({ message: err.message });
      }
    });

    app.get("/sensors", async (req, res) => {
      try {
        const sensors = await Sensor.find();
        res.json(sensors);
      } catch (err) {
        res.status(500).json({
          message:
            "Une erreur est survenue lors de la récupération des capteurs",
        });
      }
    });

    app.get("/sensors/:id", async (req, res) => {
      const { id } = req.params;
      try {
        const sensor = await Sensor.findById(id);
        if (!sensor) {
          return res
            .status(404)
            .json({ message: "Aucun capteur trouvé avec cet ID" });
        }
        res.json(sensor);
      } catch (err) {
        res.status(500).json({
          message: "Une erreur est survenue lors de la récupération du capteur",
        });
      }
    });

    app.put("/sensors/:id", async (req, res) => {
      const { id } = req.params;
      const { temperature } = req.body;

      if (!temperature) {
        return res.status(400).json({ message: "La température est requise" });
      }

      try {
        let sensor = await Sensor.findOne({ nom: id });
        if (!sensor) {
          sensor = new Sensor({
            nom: id,
            temperature: temperature,
          });
          await sensor.save();
          return res.json({
            message: "Les données du capteur ont été créées avec succès",
          });
        }
        sensor.temperature = temperature;
        await sensor.save();
        res.json({
          message: "Les données du capteur ont été mises à jour avec succès",
        });
      } catch (err) {
        res.status(500).json({
          message: `Une erreur est survenue lors de la mise à jour du capteur ${err}`,
        });
      }
    });

    fetchData();

    cron.schedule("*/20 * * * *", fetchData);
  })
  .catch((error) => {
    console.error("Impossible de se connecter à la base de données !", error);
    process.exit(1);
  });
