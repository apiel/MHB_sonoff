const WebSocket = require('ws');
 
const wss = new WebSocket.Server({ port: 8080 });
console.log('websocket server is running port 8080'); 

// on(event: 'error', cb: (error: Error) => void): this;
// on(event: 'headers', cb: (headers: string[], request: http.IncomingMessage) => void): this;
// on(event: 'listening', cb: () => void): this;

wss.on('error', function (data) {
  console.log('wss error', data);
});

wss.on('headers', function (data) {
  console.log('wss headers', data);
});

wss.on('listening', function () {
  console.log('wss listening');
});

wss.on('connection', function connection(ws) {
  console.log('new connection');
  ws.on('message', function incoming(message) {
    console.log('received: %s', message);
  });

  ws.on('close', function (data) {
    console.log('close', data);
  });

  ws.on('error', function (data) {
    console.log('error', data);
  });

  ws.on('headers', function (data) {
    console.log('headers', data);
  });

  ws.on('open', function () {
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