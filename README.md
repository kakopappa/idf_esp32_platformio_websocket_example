# esp32 idf platformio websocket example

I could not find library to connect to my websocket server using PlatformIO when I used IDF framework. So I copied the esp_websocket_client.c and esp_websocket_client.h from IDF components and made one. 

here is the nodejs server code

```

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
```
