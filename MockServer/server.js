const express = require('express');
const bodyParser = require('body-parser');
const cors = require('cors');
const path = require('path');

const app = express();
const PORT = 5000;

app.use(cors());
app.use(bodyParser.json({ limit: '10mb' }));
app.use(express.static('public'));

// 데이터 저장소 (인메모리)
let sensorData = {
    device_id: "unknown",
    temperature: 0,
    ec_value: 0,
    image_data: "",
    timestamp: ""
};

let pendingCommand = { direction: "none" };

// ESP32가 데이터를 보내는 엔드포인트
app.post('/api/sensor/data', (req, res) => {
    console.log('[ESP32 -> Server] Received sensor data');
    sensorData = {
        ...req.body,
        timestamp: new Date().toLocaleString()
    };
    res.status(200).json({ status: "success" });
});

// ESP32가 명령을 폴링하는 엔드포인트
app.get('/api/motor/command', (req, res) => {
    const deviceId = req.query.device_id;
    // console.log(`[ESP32 -> Server] Polling command for ${deviceId}`);

    // 명령이 있으면 응답하고 없애거나, 지속적으로 줄 수 있음
    // 여기서는 한 번 읽으면 none으로 리셋 (원하는 로직에 따라 수정 가능)
    const cmd = { ...pendingCommand };
    if (pendingCommand.direction !== "none") {
        console.log(`[Server -> ESP32] Sending command: ${cmd.direction}`);
        pendingCommand.direction = "none";
    }

    res.json(cmd);
});

// 대시보드가 현재 상태를 가져오는 엔드포인트
app.get('/api/status', (req, res) => {
    res.json({
        sensorData,
        pendingCommand
    });
});

// 대시보드가 명령을 내리는 엔드포인트
app.post('/api/command', (req, res) => {
    const { command } = req.body;
    console.log(`[Dashboard -> Server] Set command: ${command}`);

    // ESP32 로직상 "left"가 startInspection()을 호출함
    if (command === 'INSPECT') {
        pendingCommand = {
            command: "move",
            direction: "left",
            speed: 180,
            duration_ms: 1000
        };
    } else if (command === 'STOP') {
        pendingCommand = {
            command: "stop",
            direction: "stop",
            speed: 0,
            duration_ms: 0
        };
    }

    res.json({ status: "command_set", command });
});

app.listen(PORT, '0.0.0.0', () => {
    console.log(`
=========================================
 Mock Server is running!
 Local URL: http://localhost:${PORT}
 ESP32 URL: http://[YOUR_IP]:${PORT}/api/sensor/data
=========================================
    `);
});
