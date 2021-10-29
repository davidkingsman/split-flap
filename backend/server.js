var express = require("express");
let path = require("path");

require("dotenv").config();

let app = express();
app.use(express.static(path.join(__dirname, "public")));

app.get("/", function (req, res) {
  res.sendFile(path.join(__dirname, "public/index.html"));
});

var http = require("http").createServer(app);
var io = require("socket.io")(http);

io.on("connection", function (socket) {
  var connId = false;

  if (socket.conn) {
    connId = socket.conn.id;
  }

  var message = "a client connected: " + connId;
  console.log(message);

  socket.on("disconnect", function (msg) {
    var message = "a client disconnected: " + connId;
    console.log(message);
  });
});

var port = process.env.PORT || 3100;

http.listen(port, function () {
  console.log("listening on *:" + port);
});