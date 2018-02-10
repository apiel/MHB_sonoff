import { Server } from 'ws';

if (process.argv.length <= 3) {
  console.log("Usage: " + __filename + " ssid password");
  process.exit();
}

const ssid = process.argv[2];
const password = process.argv[3];

console.log('Listen to set wifi:', ssid, password);

const server = new Server({ port: 8080 });
console.log('websocket server is running port 8080'); 

server.on('connection', (ws, req) => {
  console.log('new connection from', req.headers.device);

  ws.send(`wifi/set ${ssid} ${password}`);
});
