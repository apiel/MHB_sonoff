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

var server = new _ws.Server({ port: 8080 });
console.log('websocket server is running port 8080');

server.on('connection', function (ws, req) {
  console.log('new connection from', req.headers.device);

  // ws.send(`wifi/set ${ssid} ${password}`);

  // const CHUNK_SIZE = 100,
  var CHUNK_SIZE = 508,
      buffer = new Buffer(CHUNK_SIZE),
      filePath = '../firmware/firmware.bin';

  (0, _fs.open)(filePath, 'r', function (err, fd) {
    console.log('file was open to read', filePath);
    if (err) throw err;

    var lenRead = -1;
    function send() {
      var value = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : null;

      lenRead = (0, _fs.readSync)(fd, buffer, 0, CHUNK_SIZE, value);
      // console.log('lenRead', lenRead);
      var data = lenRead < CHUNK_SIZE ? buffer.slice(0, lenRead) : buffer;
      console.log("bin[%d]: '%s'\n", lenRead, data);
      if (lenRead > 0) {
        ws.send(data);
        // process.stdout.write('.');
      } else {
        console.log('finish to read');
        (0, _fs.closeSync)(fd);
        ws.send('ota end', {}, function () {
          return process.exit();
        });
      }
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
            send(value);
          }
        }
      }
    });

    ws.send('ota start');
  });
});