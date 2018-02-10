"use strict";

var _ws = require("ws");

if (process.argv.length <= 3) {
  console.log("Usage: " + __filename + " ssid password");
  process.exit();
}

var ssid = process.argv[2];
var password = process.argv[3];

console.log('Listen to set wifi:', ssid, password);

var server = new _ws.Server({ port: 8080 });
console.log('websocket server is running port 8080');

server.on('connection', function (ws, req) {
  console.log('new connection from', req.headers.device);

  ws.send("wifi/set " + ssid + " " + password);
});