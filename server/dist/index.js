'use strict';

var _slicedToArray = function () { function sliceIterator(arr, i) { var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"]) _i["return"](); } finally { if (_d) throw _e; } } return _arr; } return function (arr, i) { if (Array.isArray(arr)) { return arr; } else if (Symbol.iterator in Object(arr)) { return sliceIterator(arr, i); } else { throw new TypeError("Invalid attempt to destructure non-iterable instance"); } }; }();

var _ws = require('ws');

var _mqtt = require('mqtt');

var _mqtt2 = _interopRequireDefault(_mqtt);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _toArray(arr) { return Array.isArray(arr) ? arr : Array.from(arr); }

var devices = {};

var client = _mqtt2.default.connect('mqtt://127.0.0.1');
var server = new _ws.Server({ port: 8080 });
console.log('websocket server is running port 8080');

client.on('connect', function () {
  // client.subscribe('presence')
  // client.publish('presence', 'Hello mqtt');
});

client.on('message', function (topic, message) {
  console.log('received msg:', topic, message.toString());

  var _topic$split = topic.split('/', 2),
      _topic$split2 = _slicedToArray(_topic$split, 2),
      device = _topic$split2[0],
      key = _topic$split2[1];
  // console.log('uiui:', device, key);


  if (devices[device]) {
    devices[device].send(key + ' ' + message.toString().trim());
  }
});

server.on('error', function (data) {
  console.log('server error', data);
});

server.on('headers', function (data) {
  console.log('server headers', data);
});

server.on('listening', function () {
  console.log('server listening');
});

server.on('connection', function (ws, req) {
  console.log('new connection from', req.headers.device);
  ws.on('message', function (message) {
    console.log('received: %s', message);

    var _message$split = message.split(' '),
        _message$split2 = _toArray(_message$split),
        type = _message$split2[0],
        payload = _message$split2.slice(1);

    if (type === '.') {
      var _payload = _toArray(payload),
          topic = _payload[0],
          value = _payload.slice(1);

      client.publish(req.headers.device + '/' + topic, value.join(" "));
      console.log('mqtt publish:', req.headers.device + '/' + topic, value);
    }
  });

  ws.on('close', function (data) {
    // console.log('close', data);
    if (devices[req.headers.device]) {
      delete devices[req.headers.device];
    }
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

  devices[req.headers.device] = ws;
  client.subscribe(req.headers.device + '/#');
  ws.send('get status');
});