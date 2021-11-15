var client = {
  id: false,
  alignment: "left", // left|center|right
  speedSlider: 50, // 1-100
  deviceMode: "text", // text|date|clock
  inputText: ""
};

var sendClient = function (message, data = false) {
  socket.emit("clientRequest", {
    message: message,
    data: data
  });
};

var getValues = function () {
  socket.emit("clientRequest", "getValues|all");
};

var setSpeedSlider = function (speedSlider) {
  socket.emit("clientRequest", "setSpeedSlider|" + speedSlider);
};

var setAlignment = function (alignment) {
  socket.emit("clientRequest", "setAlignment|" + alignment);
};

var setDeviceMode = function (deviceMode) {
  socket.emit("clientRequest", "setDeviceMode|" + deviceMode);
};

var setInputText = function (inputText) {
  socket.emit("clientRequest", "setInputText|" + inputText);
};

$(document).ready(function () {
  socket.emit("identifyAdmin", "jos");

  socket.on("message", function (payload) {
    console.log("message");
    console.log(payload);

    if (payload.message == "clientDetails") {
      client.id = payload.clientid;
      getValues();
    } else if (payload.message == "valuesUpdate") {
      client.alignment = payload.data.alignment;
      client.speedSlider = payload.data.speedSlider;
      client.deviceMode = payload.data.deviceMode;
      client.inputText = payload.data.inputText;
    }
  });

  //setDeviceMode("text");
  //setInputText("A");

  $("body").on("click", "#setText", function () {
    console.log("set text");
    setDeviceMode("text");
  });
  $("body").on("click", "#setK", function () {
    console.log("set K");
    setInputText("K");
  });  $("body").on("click", "#setJ", function () {
    console.log("set J");
    setInputText("J");
  });
  $("body").on("click", "#setClock", function () {
    console.log("set clock");
    setDeviceMode("clock");
  });
});