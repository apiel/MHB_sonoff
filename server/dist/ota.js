'use strict';

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) _i["return"](); } finally { if (_d) throw _e; } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

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

  var CHUNK_SIZE = 100,
      buffer = new Buffer(CHUNK_SIZE),
      filePath = '../firmware/firmware.bin';

  (0, _fs.open)(filePath, 'r', function (err, fd) {
    console.log('file was open to read', filePath);
    if (err) throw err;

    function send() {
      var offset = arguments.length > 0 && arguments[0] !== undefined ? arguments[0] : null;

      var lenRead = (0, _fs.readSync)(fd, buffer, 0, CHUNK_SIZE, offset);
      // console.log('lenRead', lenRead);
      var data = lenRead < CHUNK_SIZE ? buffer.slice(0, lenRead) : buffer;
      if (lenRead > 0) {
        ws.send(data);
        // process.stdout.write('.');
      } else {
        (0, _fs.closeSync)(fd);
        // console.log('finish to read');
        ws.send('ota end');
      }
    }

    ws.on('message', function (message) {
      // console.log('received: %s', message);
      var _message$split = message.split(' '),
          _message$split2 = _toArray(_message$split),
          type = _message$split2[0],
          payload = _message$split2.slice(1);

      if (type === '.') {
        var _payload = _slicedToArray(payload, 2),
            topic = _payload[0],
            offset = _payload[1];

        if (topic === 'ota') {
          console.log('\x1B[F\x1B[K', 'Upload: ' + offset);
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