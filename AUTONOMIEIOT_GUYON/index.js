import express from "express";
import morgan from "morgan";
import cron from "node-cron";
import mongoose from "mongoose";
import cors from "cors";
import fetchData from "./fetchData.js";

const app = express();

export const SensorData = mongoose.model(
  "SensorData",
  new mongoose.Schema({
    device_id: String,
    temp_SOIL: String,
    conduct_SOIL: String,
    water_SOIL: String,
    date: Date,
    timestamp: Number
  })
);

app.use(morgan("dev"));
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

    app.get("/sensors", async (req, res) => {
      try {
        let filter = {};
        if (req.query.date) {
          let [day, month, year] = req.query.date.split('/');
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

    fetchData();

    cron.schedule("*/20 * * * *", fetchData);
  })
  .catch((error) => {
    console.error("Impossible de se connecter à la base de données !", error);
    process.exit(1);
  });
