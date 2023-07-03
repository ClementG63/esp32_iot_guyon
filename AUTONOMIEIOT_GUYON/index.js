import express from "express";
import morgan from "morgan";
import axios from "axios";
import cron from "node-cron";
import shitConverter from "./shitConverter.js";
import mongoose from "mongoose";
import cors from "cors";

const app = express();
app.use(morgan("dev"));
app.use(cors());

const SensorData = mongoose.model(
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

const fetchData = async () => {
  try {
    const response = await axios.get(
      "https://eu1.cloud.thethings.network/api/v3/as/applications/soulmbengue-app-lorawansrv-1/packages/storage/uplink_message",
      {
        headers: {
          Authorization:
            "Bearer NNSXS.AFXIMSE6QXHFGBFXSYHMQQ6XFXJKDAOKRNFGHHI.N4WWBDZ7B7TNJA4IKJ6DGZAS6PNSRQBXZSWPFZT5ZSON52NGJW2A",
        },
      }
    );

    const data = shitConverter(response.data);

    data.forEach(async (obj) => {
      if (
        obj.json &&
        obj.json.result &&
        obj.json.result.end_device_ids.device_id === "eui-a8404194a1875ff3"
      ) {
        const sensorData = new SensorData({
          device_id: obj.json.result.end_device_ids.device_id,
          temp_SOIL: obj.json.result.uplink_message.decoded_payload.temp_SOIL,
          conduct_SOIL:
            obj.json.result.uplink_message.decoded_payload.conduct_SOIL,
          water_SOIL: obj.json.result.uplink_message.decoded_payload.water_SOIL,
          timestamp: new Date(obj.json.result.received_at).getTime(),
          date: new Date(obj.json.result.received_at), // Change this line
        });

        await sensorData.save();
        console.log(`Sauvegarde réussie pour ${sensorData}`);
      }
    });
  } catch (error) {
    console.error(error);
  }
};

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
