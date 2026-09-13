import processing.serial.*;

Serial myPort;
String portName = "";
boolean portSelected = false;

// Realtime Logic Analyzer Graph Buffers
int maxSamples = 600;
int[] sclHistory = new int[maxSamples];
int[] sdaHistory = new int[maxSamples];
int sampleIndex = 0;

// Log Messages Console
ArrayList<String> eventLog = new ArrayList<String>();
int maxLogLines = 25;

void setup() {
  size(1000, 700);
  surface.setTitle("I2C Passive Logic Analyzer & Sniffer - Processing GUI");

  // Initialize sample history with high line state
  for (int i = 0; i < maxSamples; i++) {
    sclHistory[i] = 1;
    sdaHistory[i] = 1;
  }

  // List available serial ports
  println("Available Serial Ports:");
  printArray(Serial.list());

  if (Serial.list().length > 0) {
    portName = Serial.list()[0]; // Default to first available port
  }
}

void draw() {
  background(20, 24, 30);

  if (!portSelected) {
    drawPortSelectionScreen();
  } else {
    readSerialData();
    drawHeader();
    drawWaveformGraphs();
    drawDecodedEventLog();
  }
}

void drawPortSelectionScreen() {
  fill(255);
  textAlign(CENTER, CENTER);
  textSize(22);
  text("I2C Passive Analyzer - Select USB Serial Port", width / 2, 80);

  textSize(14);
  text("Press keys 0-9 to select port, or click on a port below:", width / 2, 120);

  String[] ports = Serial.list();
  for (int i = 0; i < ports.length && i < 10; i++) {
    float y = 180 + i * 40;
    fill(40, 50, 65);
    stroke(0, 200, 255);
    rect(width / 2 - 200, y - 15, 400, 30, 8);

    fill(255);
    textAlign(CENTER, CENTER);
    text("[" + i + "]  " + ports[i], width / 2, y);
  }
}

void keyPressed() {
  if (!portSelected && key >= '0' && key <= '9') {
    int index = key - '0';
    String[] ports = Serial.list();
    if (index < ports.length) {
      connectToPort(ports[index]);
    }
  }
}

void mousePressed() {
  if (!portSelected) {
    String[] ports = Serial.list();
    for (int i = 0; i < ports.length && i < 10; i++) {
      float y = 180 + i * 40;
      if (mouseX > width / 2 - 200 && mouseX < width / 2 + 200 &&
          mouseY > y - 15 && mouseY < y + 15) {
        connectToPort(ports[i]);
        break;
      }
    }
  }
}

void connectToPort(String p) {
  try {
    portName = p;
    myPort = new Serial(this, portName, 500000);
    myPort.bufferUntil('\n');
    portSelected = true;
    eventLog.add("[SYSTEM] Connected to " + portName + " at 500000 baud.");
  } catch (Exception e) {
    println("Error opening port: " + e.getMessage());
  }
}

void readSerialData() {
  while (myPort != null && myPort.available() > 0) {
    String line = myPort.readStringUntil('\n');
    if (line != null) {
      line = trim(line);
      processIncomingLine(line);
    }
  }
}

void processIncomingLine(String line) {
  if (line.startsWith("L,")) {
    // Pin state update packet: L,SCL,SDA
    String[] parts = split(line, ',');
    if (parts.length >= 3) {
      int scl = int(parts[1]);
      int sda = int(parts[2]);

      sclHistory[sampleIndex] = scl;
      sdaHistory[sampleIndex] = sda;
      sampleIndex = (sampleIndex + 1) % maxSamples;
    }
  } else if (line.startsWith("E,")) {
    // Decoded I2C Event packet
    String eventStr = parseEventLine(line);
    if (eventStr.length() > 0) {
      eventLog.add(eventStr);
      if (eventLog.size() > maxLogLines) {
        eventLog.remove(0);
      }
    }
  }
}

String parseEventLine(String line) {
  String[] parts = split(line, ',');
  if (parts.length < 2) return "";

  String type = parts[1];
  if (type.equals("START")) return ">> [START CONDITION]";
  if (type.equals("STOP")) return "<< [STOP CONDITION]";
  if (type.equals("RESTART")) return ">> [REPEATED START]";

  if (type.equals("ADDR") && parts.length >= 5) {
    return "ADDR  : " + parts[2] + " (" + parts[3] + ") -> " + parts[4];
  }
  if (type.equals("DATA") && parts.length >= 4) {
    return "DATA  : " + parts[2] + " -> " + parts[3];
  }
  return line;
}

void drawHeader() {
  fill(30, 38, 48);
  noStroke();
  rect(0, 0, width, 50);

  fill(0, 220, 255);
  textAlign(LEFT, CENTER);
  textSize(18);
  text("I2C Passive Logic Analyzer & Sniffer", 20, 25);

  fill(180, 200, 220);
  textSize(12);
  textAlign(RIGHT, CENTER);
  text("Port: " + portName + " (500k Baud) | Passive Sniffing (SDA: Pin 2, SCL: Pin 3)", width - 20, 25);
}

void drawWaveformGraphs() {
  float graphX = 80;
  float graphW = width - 100;

  // SCL Waveform Panel
  stroke(50, 65, 80);
  fill(15, 20, 28);
  rect(graphX, 70, graphW, 110, 6);

  fill(255, 220, 0); // Yellow for SCL
  textAlign(RIGHT, CENTER);
  textSize(14);
  text("SCL (Pin 3)", graphX - 10, 125);

  drawSignalWaveform(sclHistory, graphX, 70, graphW, 110, color(255, 220, 0));

  // SDA Waveform Panel
  stroke(50, 65, 80);
  fill(15, 20, 28);
  rect(graphX, 200, graphW, 110, 6);

  fill(0, 220, 255); // Cyan for SDA
  textAlign(RIGHT, CENTER);
  text("SDA (Pin 2)", graphX - 10, 255);

  drawSignalWaveform(sdaHistory, graphX, 200, graphW, 110, color(0, 220, 255));
}

void drawSignalWaveform(int[] history, float x, float y, float w, float h, color c) {
  stroke(c);
  strokeWeight(2);
  noFill();

  float stepX = w / (float)(maxSamples - 1);
  float yHigh = y + 25;
  float yLow = y + h - 25;

  beginShape();
  for (int i = 0; i < maxSamples; i++) {
    int idx = (sampleIndex + i) % maxSamples;
    float px = x + i * stepX;
    float py = (history[idx] == 1) ? yHigh : yLow;

    if (i > 0) {
      int prevIdx = (sampleIndex + i - 1) % maxSamples;
      float prevPy = (history[prevIdx] == 1) ? yHigh : yLow;
      if (py != prevPy) {
        vertex(px, prevPy); // Vertical transition edge
      }
    }
    vertex(px, py);
  }
  endShape();
  strokeWeight(1);
}

void drawDecodedEventLog() {
  float logX = 20;
  float logY = 330;
  float logW = width - 40;
  float logH = height - logY - 20;

  stroke(50, 65, 80);
  fill(12, 16, 22);
  rect(logX, logY, logW, logH, 6);

  fill(0, 200, 255);
  textAlign(LEFT, TOP);
  textSize(14);
  text("Decoded I2C Bus Traffic & Data Frames:", logX + 15, logY + 12);

  stroke(40, 50, 65);
  line(logX + 15, logY + 35, logX + logW - 15, logY + 35);

  textFont(createFont("Monospaced", 13));
  float textY = logY + 45;

  for (int i = 0; i < eventLog.size(); i++) {
    String logLine = eventLog.get(i);

    if (logLine.contains("START")) fill(0, 255, 120);        // Green
    else if (logLine.contains("STOP")) fill(255, 100, 100);  // Red
    else if (logLine.contains("ADDR")) fill(255, 220, 0);    // Yellow
    else if (logLine.contains("DATA")) fill(0, 200, 255);    // Cyan
    else fill(180, 200, 220);

    text(logLine, logX + 15, textY);
    textY += 18;
  }
}
