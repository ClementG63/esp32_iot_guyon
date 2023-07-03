export default function shitConverter(jsonString) {
    var jsonObjects = [];
    var remainingString = "";
    var depth = 0;
    var indexStart = 0;
    var indexEnd = 0;
    var pointer = 0;
  
    if (!jsonString.includes("{") && !jsonString.includes("}")) {
      jsonObjects.push({ pretext: jsonString, json: null });
    } else {
      for (var i = 0; i < jsonString.length; i++) {
        if (jsonString.charAt(i) === "{") {
          if (depth === 0) {
            indexStart = i;
          }
          depth++;
        } else if (jsonString.charAt(i) === "}") {
          depth--;
          if (depth === 0) {
            indexEnd = i;
  
            var finalJson = JSON.parse(
              jsonString.substring(indexStart, indexEnd + 1)
            );
            var gapText = jsonString.substring(pointer, indexStart);
            jsonObjects.push({ pretext: gapText, json: finalJson });
  
            pointer = indexEnd + 1;
            remainingString = jsonString.substring(pointer, jsonString.length);
  
            if (
              !remainingString.includes("{") &&
              !remainingString.includes("}")
            ) {
              jsonObjects.push({ pretext: remainingString, json: null });
            }
          }
        }
      }
    }
    return jsonObjects;
  }