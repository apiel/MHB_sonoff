'use strict';

var _ws = require('ws');

var _fs = require('fs');

function _toArray(arr) { return Array.isArray(arr) ? arr : Array.from(arr); }

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


var server = new _ws.Server({ port: 8080 });
console.log('websocket server is running port 8080');

server.on('connection', function (ws, req) {
  console.log('new connection from', req.headers.device);

  // ws.send(`wifi/set ${ssid} ${password}`);

  // const CHUNK_SIZE = 100,
  var CHUNK_SIZE = 508,

  // const CHUNK_SIZE = 4096,
  // const CHUNK_SIZE = 1024,
  buffer = new Buffer(CHUNK_SIZE),
      filePath = '../firmware/firmware.bin';

  var lenRead = -1;
  function send() {
    var start = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : 0;

    var readStream = (0, _fs.createReadStream)(filePath, { start: start, end: start + CHUNK_SIZE - 1 });

    readStream.on('error', function (err) {
      console.log('file was open to read', filePath);
      if (err) throw err;
    });

    // Listen for data
    readStream.on('data', function (chunk) {
      ws.send(chunk);
    });

    readStream.on('close', function () {
      if (!readStream.bytesRead) {
        console.log('finish to read');
        ws.send('ota end', {}, function () {
          return process.exit();
        });
      }
    });
  }

  ws.on('message', function (message) {
    // console.log('received: %s', message);
    var _message$split = message.split(' '),
        _message$split2 = _toArray(_message$split),
        type = _message$split2[0],
        payload = _message$split2.slice(1);

    if (type === '.') {
      var _payload = _toArray(payload),
          topic = _payload[0],
          value = _payload[1],
          _message = _payload.slice(2);

      if (topic === 'ota') {
        if (value === 'error') {
          console.error('ota errror', _message);
        } else {
          console.log('\x1B[F\x1B[K', 'Upload: ' + value);
          send(parseInt(value, 10));
        }
      }
    }
  });

  ws.send('ota start');
});