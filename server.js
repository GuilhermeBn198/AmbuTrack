const express = require("express");
const app = express();

// Configuração da área de interesse (mesmos valores definidos no ESP32)
const targetLat = 2.832636;  // Latitude alvo
const targetLon = -60.686386;  // Longitude alvo
const targetRadius = 250;       // Raio em metros

let gpsData = { lat: 0, lon: 0 }; // Última localização recebida
let routeHistory = [];           // Histórico da rota

app.get("/", (req, res) => {
  res.send(`<!DOCTYPE html>
<html>
  <head>
    <meta charset="utf-8">
    <title>Localização da Ambulância</title>
    <meta http-equiv="refresh" content="5">
    <link rel="stylesheet" href="https://unpkg.com/leaflet/dist/leaflet.css" />
    <script src="https://unpkg.com/leaflet/dist/leaflet.js"></script>
    <script>
      // Parâmetros da área de interesse
      var targetLat = ${targetLat};
      var targetLon = ${targetLon};
      var targetRadius = ${targetRadius};

      var map;
      var marker;
      var polyline;

      function initMap() {
        // Define a posição atual da ambulância
        var location = { lat: ${gpsData.lat}, lng: ${gpsData.lon} };
        var routeHistory = ${JSON.stringify(routeHistory)};

        // Inicializa o mapa centralizado na última posição conhecida
        map = L.map('map').setView(location, 15);

        // Adiciona a camada do OpenStreetMap
        L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
          attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
        }).addTo(map);

        // Desenha a área de interesse (círculo)
        L.circle([targetLat, targetLon], {
          color: 'blue',
          fillColor: '#30f',
          fillOpacity: 0.2,
          radius: targetRadius
        }).addTo(map).bindPopup("Área de Risco");

        // Se o marcador já existir, remove-o
        if (marker) {
          marker.remove();
        }

        // Adiciona o marcador da localização atual
        marker = L.marker(location).addTo(map)
          .bindPopup("<b>Localização Atual da Ambulância</b>")
          .openPopup();

        // Se houver histórico de rota, desenha uma linha
        if (routeHistory.length > 1) {
          polyline = L.polyline(routeHistory, { color: 'red' }).addTo(map);
        }
      }

      // Inicializa o mapa quando a página carregar
      window.onload = initMap;
    </script>
  </head>
  <body>
    <h1>Localização Atual</h1>
    <p>Latitude: ${gpsData.lat}</p>
    <p>Longitude: ${gpsData.lon}</p>
    <a href="https://www.openstreetmap.org/?mlat=${gpsData.lat}&mlon=${gpsData.lon}#map=15/${gpsData.lat}/${gpsData.lon}" target="_blank">Abrir no OpenStreetMap</a>
    <div id="map" style="width: 100%; height: 500px;"></div>
  </body>
</html>
  `);
});

app.get("/update", (req, res) => {
  if (req.query.lat && req.query.lon) {
    let lat = parseFloat(req.query.lat);
    let lon = parseFloat(req.query.lon);
    
    gpsData = { lat, lon }; // Atualiza a posição atual
    routeHistory.push([lat, lon]); // Adiciona ao histórico da rota

    console.log(`Nova localização: ${lat}, ${lon}`);
    res.send("Localização atualizada e armazenada na rota!");
  } else {
    res.send("Erro: Passe os parâmetros lat e lon.");
  }
});

const PORT = process.env.PORT || 3000;
app.listen(PORT, () => console.log(`Servidor rodando na porta ${PORT}`));