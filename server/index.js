const { createServer } = require("http");
const express = require("express");
const socketIo = require("socket.io");

const app = express();
const server = createServer(app);
const io = new socketIo.Server(server, {
  cors: {
    origin: "*",
  },
});

let espClient;
let state = {
  led_status: false,
  esp_status: false,
};

app.set("view engine", "ejs");
app.set("views", __dirname + "/views");
app.get("/", (_, res) => {
  res.render("main");
});

const onStateUpdate = (client) => {
  if (client) {
    client.emit("state_update", state);
  } else {
    io.emit("state_update", state);
  }
};

io.on("connection", (client) => {
  console.log(`Client ${client.id} connected!`);
  onStateUpdate(client);

  client.on("client", (data) => {
    if (data === "esp") {
      client.isEsp = true;
      espClient = client;
      state.esp_status = true;
      onStateUpdate();
    }
  });

  client.on("set_led", (data) => {
    state.led_status = data;
    onStateUpdate();
  });

  client.on("disconnect", () => {
    console.log(`Client ${client.id} disconnected!`);

    if (espClient?.id === client.id) {
      espClient = null;
      state.esp_status = false;
      onStateUpdate();
    }
  });
});

const HOST = "0.0.0.0";
const PORT = process.env.PORT || 8080;

server.listen(PORT, HOST, () => {
  console.log(`Server listening on ${HOST}:${PORT}`);
});
