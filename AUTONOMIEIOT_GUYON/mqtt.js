import { connect } from "mqtt";
const client = connect("mqtt://test.mosquitto.org:1883");
import { readFile, writeFile } from "fs";

client.on("connect", () => {
  console.log("MQTT enabled!");
  client.subscribe("ynov-lyon-2023/esp32/in", function (err) {
    if (!err) {
      console.log("Subscribe done!");
    }
  });
});

client.on("message", (topic, message) => {
  if (topic == "ynov-lyon-2023/esp32/in") {
    const jsonString = message.toString();
    const jsonObject = JSON.parse(jsonString);

    if (jsonObject.id) {
      readFile("./temp.json", "utf8", (err, data) => {
        if (err) {
          console.error("Error reading file:", err);
        }

        let fileData;
        try {
          fileData = JSON.parse(data);
        } catch (error) {
          console.error("Error parsing JSON:", error);
        }

        const existingItemIndex = fileData.findIndex(
          (item) => item.id === jsonObject.id
        );
        if (existingItemIndex !== -1) {
          fileData[existingItemIndex] = jsonObject;
        } else {
          fileData.push(jsonObject);
        }

        writeFile("./temp.json", JSON.stringify(fileData), "utf8", (err) => {
          if (err) {
            console.error("Error writing file:", err);
          }

          console.log("Item added/updated successfully.");
        });
      });
    } else if (jsonObject.name) {
      readFile("./temp.json", "utf8", (err, data) => {
        if (err) {
          console.error("Error reading file:", err);
        }

        const fileData = JSON.parse(data);
        const esp = fileData.find((item) => item.id === jsonObject.name);

        if (esp) {
          const config = esp.body.config;
          console.log(config);
          client.publish("ynov-lyon-2023/esp32/out", config);
        } else {
          console.error("Cannot find esp.");
        }
      });
    }
  }
});
