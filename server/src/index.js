import { Server } from 'ws';
import mqtt from 'mqtt';

const devices = {};

const client = mqtt.connect('mqtt://127.0.0.1')
const server = new Server({ port: 8080 });
console.log('websocket server is running port 8080'); 

client.on('connect', () => {
  // client.subscribe('presence')
  // client.publish('presence', 'Hello mqtt');
});
 
client.on('message', (topic, message) => {
    console.log('received msg:', topic, message.toString());
    const [device, key] = topic.split('/', 2);
    // console.log('uiui:', device, key);
    if (devices[device]) {
      devices[device].send(`${key} ${message.toString().trim()}`);
    }
});

server.on('error', (data) => {
  console.log('server error', data);
});

server.on('headers', (data) => {
  console.log('server headers', data);
});

server.on('listening', () => {
  console.log('server listening');
});

server.on('connection', (ws, req) => {
  console.log('new connection from', req.headers.device);
  ws.on('message', (message) => {
    console.log('received: %s', message);
    const [type, ...payload] = message.split(' ');
    if (type === '.') {
      const [topic, ...value] = payload;
      client.publish(`${req.headers.device}/${topic}`, value.join(" "));
      console.log('mqtt publish:', `${req.headers.device}/${topic}`, value);
    }
  });

  ws.on('ping', (data) => {
    ws.pong();
  });

  ws.on('close', (data) => {
      // console.log('close', data);
      if (devices[req.headers.device]) {
          delete devices[req.headers.device];
      }
  });

  ws.on('error', (data) => {
    console.log('error', data);
  });

  ws.on('headers', (data) => {
    console.log('headers', data);
  });

  ws.on('open', () => {
    console.log('open');
  });

  devices[req.headers.device] = ws;
  client.subscribe(`${req.headers.device}/#`);
  ws.send('get status');
});
