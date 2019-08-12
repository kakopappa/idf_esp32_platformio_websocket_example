# idf_esp32_platformio_websocket_example

I could not find library to connect to my websocket server using PlatformIO IDF library. So I copied the esp_websocket_client.c and esp_websocket_client.h from IDF and made one.

`
here is the nodejs server code

const WebSocket = require('ws');

const wss = new WebSocket.Server({ port: 9090 });

wss.on('connection', function connection(ws) {
  ws.on('message', function incoming(data) {
        try {
          var buf = new Buffer(data, 'utf8');
          console.log(buf.toString());
        } catch (e) {
          
        }
        //console.log(buf);
  });
});

wss.on('connection', (ws) => {
  ws.on('error', console.log);
});
`
