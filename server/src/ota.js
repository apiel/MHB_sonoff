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
    if (err) throw err;

    function send(offset = null) {
      const lenRead = readSync(fd, buffer, 0, CHUNK_SIZE, offset);
      // console.log('lenRead', lenRead);
      const data = lenRead < CHUNK_SIZE ? buffer.slice(0, lenRead) : buffer;
      if (lenRead > 0) {
        ws.send(data);
        // process.stdout.write('.');
      } else {
        closeSync(fd);
        // console.log('finish to read');
        ws.send('ota end');     
      }
    }

    ws.on('message', (message) => {
      // console.log('received: %s', message);
      const [type, ...payload] = message.split(' ');
      if (type === '.') {
        const [topic, offset] = payload;
        if (topic === 'ota') {
          console.log('\x1B[F\x1B[K', `Upload: ${offset}`);
          if (offset % CHUNK_SIZE > 0) {
              console.log('finish to read');
              process.exit();
          } else {
              send(offset);
          }
        }
      }
    });

    ws.send('ota start');
  });
});
 