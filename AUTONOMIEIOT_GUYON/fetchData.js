import axios from "axios";
import shitConverter from "./shitConverter.js";
import { SensorData } from "./index.js";

export default async function fetchData() {
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
}