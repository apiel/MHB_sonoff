import { Server } from 'ws';
import { open, readSync, closeSync, createReadStream } from 'fs';

// if (process.argv.length <= 3) {
//   console.log("Usage: " + __filename + " ssid password");
//   process.exit();
// }

// const ssid = process.argv[2];
// const password = process.argv[3];

// console.log('Listen to set wifi:', ssid, password);


// const readStream = createReadStream('../firmware/firmware.bin');

// readStream.on('error', err => {
//     // handle error
// });

// // Listen for data
// readStream.on('data', chunk => {
//     console.log('datatat', chunk.toString());
//     process.exit();
// });

// readStream.on('close', () => {
//     // Create a buffer of the image from the stream
// });





// const server = new Server({ port: 8080 });
// console.log('websocket server is running port 8080'); 

// server.on('connection', (ws, req) => {
//   console.log('new connection from', req.headers.device);

//   // ws.send(`wifi/set ${ssid} ${password}`);

//   // const CHUNK_SIZE = 100,
//   const CHUNK_SIZE = 508,
//     buffer = new Buffer(CHUNK_SIZE),
//     filePath = '../firmware/firmware.bin';

//   open(filePath, 'r', (err, fd) => {
//     console.log('file was open to read', filePath);
//     if (err) throw err;

//     let lenRead = -1;
//     function send(value = null) {
//       lenRead = readSync(fd, buffer, 0, CHUNK_SIZE, value);
//       // console.log('lenRead', lenRead);
//       const data = lenRead < CHUNK_SIZE ? buffer.slice(0, lenRead) : buffer;
//       console.log("bin[%d]: '%s'\n", lenRead, data);
//       if (lenRead > 0) {
//         ws.send(data);
//         // process.stdout.write('.');
//       } 
//       else { 
//         console.log('finish to read');
//         closeSync(fd);
//         ws.send('ota end', {}, () => process.exit());
//       }
//     }

//     ws.on('message', (message) => {
//       // console.log('received: %s', message);
//       const [type, ...payload] = message.split(' ');
//       if (type === '.') {
//         const [topic, value, ...message] = payload;
//         if (topic === 'ota') {
//           if (value === 'error') {
//             console.error('ota errror', message);
//           } else {
//             console.log('\x1B[F\x1B[K', `Upload: ${value}`);
//             send(value);
//           }
//         }
//       }
//     });

//     ws.send('ota start');
//   });
// });
 



const server = new Server({ port: 8080 });
console.log('websocket server is running port 8080'); 

server.on('connection', (ws, req) => {
  console.log('new connection from', req.headers.device);

  // ws.send(`wifi/set ${ssid} ${password}`);

  // const CHUNK_SIZE = 100,
  const CHUNK_SIZE = 508,
    buffer = new Buffer(CHUNK_SIZE),
    filePath = '../firmware/firmware.bin';

  let lenRead = -1;
  function send(start = 0) {
    const readStream = createReadStream(filePath, { start, end: start + CHUNK_SIZE -1 });

    readStream.on('error', err => {
      console.log('file was open to read', filePath);
      if (err) throw err;
    });

    // Listen for data
    readStream.on('data', chunk => {
        // console.log('datatat', chunk.toString());
        // process.exit();
        ws.send(chunk);

        // if chunk empty { 
        //   console.log('finish to read');
        //   closeSync(fd);
        //   ws.send('ota end', {}, () => process.exit());
        // }          
    });

    readStream.on('close', () => {
        // Create a buffer of the image from the stream
    });
  }

  ws.on('message', (message) => {
    // console.log('received: %s', message);
    const [type, ...payload] = message.split(' ');
    if (type === '.') {
      const [topic, value, ...message] = payload;
      if (topic === 'ota') {
        if (value === 'error') {
          console.error('ota errror', message);
        } else {
          console.log('\x1B[F\x1B[K', `Upload: ${value}`);
          send(parseInt(value, 10));
        }
      }
    }
  });

  ws.send('ota start');
});
 