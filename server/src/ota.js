import { Server } from 'ws';
import { open, readSync, closeSync } from 'fs';

// if (process.argv.length <= 3) {
//   console.log("Usage: " + __filename + " ssid password");
//   process.exit();
// }

// const ssid = process.argv[2];
// const password = process.argv[3];

// console.log('Listen to set wifi:', ssid, password);

const server = new Server({ port: 8080 });
console.log('websocket server is running port 8080'); 

server.on('connection', (ws, req) => {
  console.log('new connection from', req.headers.device);

  // ws.send(`wifi/set ${ssid} ${password}`);

  const CHUNK_SIZE = 100,
    buffer = new Buffer(CHUNK_SIZE),
    filePath = '../firmware/firmware.bin';

  open(filePath, 'r', (err, fd) => {
    console.log('file was open to read', filePath);
    // ws.send('file was open to read');
    if (err) throw err;

    function send(offset = null) {
      const lenRead = readSync(fd, buffer, 0, CHUNK_SIZE, offset);
      console.log('lenRead', lenRead);
      const data = lenRead < CHUNK_SIZE ? buffer.slice(0, lenRead) : buffer;
      if (lenRead > 0) {
        ws.send(data);
      } else {
        closeSync(fd);
        console.log('finish to read');
        ws.send('finish to read');        
      }
    }
    send();

    ws.on('message', (message) => {
      console.log('received: %s', message);
      const [type, ...payload] = message.split(' ');
      if (type === '.') {
        const [topic, offset] = payload;
        if (topic === 'ota') {
          send(offset);
        }
      }
    });
  });
});
