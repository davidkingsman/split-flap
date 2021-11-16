var client = {
  deviceId: false,
  id: false,
  alignment: "left", // left|center|right
  speedSlider: 50, // 1-100
  deviceMode: "clock", // text|date|clock
  inputText: "HELLOWORLD"
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
  console.log("setSpeedSlider", speedSlider);
};

var setAlignment = function (alignment) {
  socket.emit("clientRequest", "setAlignment|" + alignment);
  console.log("setAlignment", alignment);
};

var setDeviceMode = function (deviceMode) {
  socket.emit("clientRequest", "setDeviceMode|" + deviceMode);
  console.log("setDeviceMode", deviceMode);
};

var setInputText = function (inputText) {
  socket.emit("clientRequest", "setInputText|" + inputText);
  console.log("setInputText", inputText);
};

var updateValues = function () {
  $("#inputText").val(client.inputText);

  $("#setAlignment").children().removeClass("active");
  $("#setAlignment #" + client.alignment).addClass("active");
  $("#setDeviceMode").children().removeClass("active");
  $("#setDeviceMode #" + client.deviceMode).addClass("active");
  $("#setSpeedSlider span").text(client.speedSlider);
};

var updateConnectionStatus = function () {
  if (client.id) {
    $("#controlBoard").removeClass("disconnected").addClass("connected");
  } else {
    $("#controlBoard").removeClass("connected").addClass("disconnected");
  }
};

var toggleControlBoard = function () {
  if (client.deviceId) {
    $("#currentDeviceId").text(client.deviceId);
    $("#controlBoard").show();
    $("#deviceAdmin").hide();
  } else {
    $("#currentDeviceId").text("");
    $("#deviceAdmin").show();
    $("#controlBoard").hide();
  }
};

$(document).ready(function () {
  socket.on("message", function (payload) {
    console.log("message");
    console.log(payload);

    if (payload.message == "clientDetails") {
      client.id = payload.clientId;
      updateConnectionStatus();
      getValues();
    } else if (payload.message == "valuesUpdate") {
      client.alignment = payload.data.alignment;
      client.speedSlider = payload.data.speedSlider;
      client.deviceMode = payload.data.deviceMode;
      client.inputText = payload.data.inputText;
      updateValues();
    } else if (payload.message == "disconnect") {
      client.id = false;
      updateConnectionStatus();
    }
  });

  $("body").on("click", "#connectDevice", function () {
    var deviceId = $("#deviceId").val().trim();
    if (deviceId) {
      client.deviceId = deviceId;
      toggleControlBoard();
      socket.emit("identifyAdmin", client.deviceId);
    }
  });

  $("body").on("click", "#disconnect", function () {
    client.deviceId = false;
    toggleControlBoard();
  });

  $("#setAlignment").on("click", "a", function () {
    var newAlignment = $(this).attr("id");
    setAlignment(newAlignment);
  });

  $("#setDeviceMode").on("click", "a", function () {
    var newDeviceMode = $(this).attr("id");
    setDeviceMode(newDeviceMode);
  });

  $("#setSpeedSlider").on("click", "a", function () {
    var newSpeedSlider = $(this).text();
    setSpeedSlider(newSpeedSlider);
  });

  $("body").on("click", "#setInputText", function () {
    var newInputText = $("#inputText").val();
    setInputText(newInputText);
    setDeviceMode("text");
  });

  $("#inputText").on("input", function () {
    var uppercase = $("#inputText").val().toUpperCase().replace(/[^A-Z0-9 :.\-?!]/g, "");
    $("#inputText").val(uppercase);
  });
});