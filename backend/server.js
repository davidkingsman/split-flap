var express = require("express");
let path = require("path");

require("dotenv").config();

let app = express();
app.use(express.static(path.join(__dirname, "public")));

app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname, "public/index.html"));
});

var port = process.env.PORT || 3100;

var http = require("http").createServer(app);
var io = require("socket.io")(http);

var webSockets = {};
var clients = {};
var admins = {};

// methods
var getKeyByValue = function (object, value) {
  return Object.keys(object).find(key => object[key] === value);
};

var getClientForClientConnId = function (clientConnId) {
  var client = getKeyByValue(clients, clientConnId);
  return client;
};

var getAdminForAdminConnId = function (adminConnId) {
  var admin = getKeyByValue(admins, adminConnId);
  return admin;
};

var getConnIdForClient = function (client) {
  var connId = clients[client];

  if (connId < 0) {
    connId = false;
  }

  console.log(connId, "for", client);

  return connId;
};

var getClientConnIdForAdminConnId = function (adminConnId) {
  var admin = getKeyByValue(admins, adminConnId);
  var connId = clients[admin];
  //console.log(adminConnId, admin, connId);
  return connId;
};

io.on("connection", function (webSocket) {
  var connId = false;

  if (webSocket.conn) {
    connId = webSocket.conn.id;
  }

  webSockets[connId] = webSocket;
  console.log("a client connected: " + connId);

  webSocket.on("disconnect", function (msg) {
    delete webSockets[connId];
    delete getClientForClientConnId(connId);
    delete getAdminForAdminConnId(connId);

    console.log("a client disconnected: " + connId);
  });

  webSocket.on("identify", function (data) {
    clients[data] = connId;

    if (admins[data]) {
      webSockets[admins[data]].send({message: "clientDetails", clientId: connId});
    }

    console.log("identify from", connId, ":", data);
  });

  webSocket.on("identifyAdmin", function (data) {
    admins[data] = connId;

    var clientId = clients[data]
      ? clients[data]
      : 0;

    webSockets[connId].send({message: "clientDetails", clientId: clientId});

    console.log("identify admin from", connId, ":", data);
  });

  webSocket.on("valuesUpdate", function (data) {
    console.log("valuesUpdate");
    console.log(data);
    var clientId = admins[getClientForClientConnId(connId)];

    if (clientId) {
      webSockets[clientId].send({message: "valuesUpdate", data: data});
    }
  });
  

  webSocket.on("clientRequest", function (data) {
    console.log("clientRequest");
    console.log(data);

    var admin = getAdminForAdminConnId(connId);

    if (clients[admin]) {
      webSockets[clients[admin]].send(data);
    }
  });
});

http.listen(port, function () {
  console.log("listening on *:" + port);
});