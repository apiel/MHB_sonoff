import { Server } from 'ws';
import mqtt from 'mqtt';

// import yo from './lol';

const client = mqtt.connect('mqtt://127.0.0.1')
const server = new Server({ port: 8080 });
console.log('websocket server is running port 8080'); 

client.on('connect', () => {
  client.subscribe('presence')
  client.publish('presence', 'Hello mqtt');
});
 
client.on('message', (topic, message) => {
  console.log(topic, message.toString());
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

server.on('connection', (ws) => {
  console.log('new connection');
  ws.on('message', (message) => {
    console.log('received: %s', message);
  });

  ws.on('close', (data) => {
    console.log('close', data);
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
  // on(event: 'close', listener: (code: number, reason: string) => void): this;
  // on(event: 'error', listener: (err: Error) => void): this;
  // on(event: 'headers', listener: (headers: {}, request: http.IncomingMessage) => void): this;
  // on(event: 'message', listener: (data: WebSocket.Data) => void): this;
  // on(event: 'open' , listener: () => void): this;
  // on(event: 'ping' | 'pong', listener: (data: Buffer) => void): this;
  // on(event: 'unexpected-response', listener: (request: http.ClientRequest, response: http.IncomingMessage) => void): this;
 
  ws.send('something');
});