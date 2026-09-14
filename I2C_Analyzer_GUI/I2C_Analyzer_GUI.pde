import processing.serial.*;

Serial myPort;
String portName = "";
boolean portSelected = false;
boolean isPaused = false;

// Realtime Logic Analyzer Waveform Graph Buffers
int maxSamples = 600;
int[] sclHistory = new int[maxSamples];
int[] sdaHistory = new int[maxSamples];
long[] timeHistory = new long[maxSamples];
int sampleIndex = 0;

// Log Messages Console
ArrayList<String> eventLog = new ArrayList<String>();
int maxLogLines = 20;

// Processing-Side I2C State Machine Decoder
int prevSCL = 1;
int prevSDA = 1;
boolean inFrame = false;
int currentByte = 0;
int bitCount = 0;
int frameByteIndex = 0;

void setup() {
  size(1000, 720);
  surface.setTitle("I2C Passive Logic Analyzer & Decoder - Processing GUI");

  for (int i = 0; i < maxSamples; i++) {
    sclHistory[i] = 1;
    sdaHistory[i] = 1;
    timeHistory[i] = 0;
  }

  println("Available Serial Ports:");
  printArray(Serial.list());

  if (Serial.list().length > 0) {
    portName = Serial.list()[0];
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
  text("I2C Passive Sniffer - Select USB Serial Port", width / 2, 80);

  textSize(14);
  text("Press keys 0-9 or click on a port below:", width / 2, 120);

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
  } else if (key == ' ') {
    isPaused = !isPaused; // Toggle Pause / Freeze
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
  } else {
    // Check Pause Button click
    if (mouseX > width - 160 && mouseX < width - 20 && mouseY > 12 && mouseY < 38) {
      isPaused = !isPaused;
    }
  }
}

void connectToPort(String p) {
  try {
    portName = p;
    myPort = new Serial(this, portName, 500000);
    portSelected = true;
    addLog("[SYSTEM] Connected to " + portName + " at 500,000 baud.");
  } catch (Exception e) {
    println("Error opening port: " + e.getMessage());
  }
}

void readSerialData() {
  while (myPort != null && myPort.available() >= 5) {
    int stateByte = myPort.read();
    if ((stateByte & 0x80) != 0) {
      int scl = (stateByte >> 1) & 0x01;
      int sda = stateByte & 0x01;

      long ts3 = myPort.read() & 0xFF;
      long ts2 = myPort.read() & 0xFF;
      long ts1 = myPort.read() & 0xFF;
      long ts0 = myPort.read() & 0xFF;
      long microTs = (ts3 << 24) | (ts2 << 16) | (ts1 << 8) | ts0;

      // Update Realtime Waveform Graph History
      sclHistory[sampleIndex] = scl;
      sdaHistory[sampleIndex] = sda;
      timeHistory[sampleIndex] = microTs;
      sampleIndex = (sampleIndex + 1) % maxSamples;

      // Perform Protocol Decoding in Processing
      decodeI2C(scl, sda, microTs);

      prevSCL = scl;
      prevSDA = sda;
    }
  }
}

void decodeI2C(int scl, int sda, long ts) {
  String timeStr = "[" + String.format("%08d", ts % 100000000L) + " us] ";

  // START Condition: SDA falls from 1 to 0 while SCL is HIGH
  if (prevSDA == 1 && sda == 0 && scl == 1 && prevSCL == 1) {
    if (!inFrame) {
      addLog(timeStr + ">> [START CONDITION]");
    } else {
      addLog(timeStr + ">> [REPEATED START]");
    }
    inFrame = true;
    currentByte = 0;
    bitCount = 0;
    frameByteIndex = 0;
    return;
  }

  // STOP Condition: SDA rises from 0 to 1 while SCL is HIGH
  if (prevSDA == 0 && sda == 1 && scl == 1 && prevSCL == 1) {
    if (inFrame) {
      addLog(timeStr + "<< [STOP CONDITION]");
      inFrame = false;
    }
    return;
  }

  // Sample Data Bits on SCL Rising Edge (0 to 1)
  if (prevSCL == 0 && scl == 1 && inFrame) {
    if (bitCount < 8) {
      currentByte = (currentByte << 1) | (sda & 0x01);
      bitCount++;
    } else if (bitCount == 8) {
      // 9th bit: ACK (0) or NACK (1)
      boolean ack = (sda == 0);
      frameByteIndex++;

      if (frameByteIndex == 1) {
        int addr = (currentByte >> 1) & 0x7F;
        boolean isRead = (currentByte & 0x01) != 0;
        addLog(timeStr + "   ADDR : 0x" + hex(addr, 2) + " (" + (isRead ? "READ" : "WRITE") + ") -> " + (ack ? "ACK" : "NACK"));
      } else {
        addLog(timeStr + "   DATA : 0x" + hex(currentByte, 2) + " -> " + (ack ? "ACK" : "NACK"));
      }

      currentByte = 0;
      bitCount = 0;
    }
  }
}

void addLog(String msg) {
  if (!isPaused) {
    eventLog.add(msg);
    if (eventLog.size() > maxLogLines) {
      eventLog.remove(0);
    }
  }
}

void drawHeader() {
  fill(30, 38, 48);
  noStroke();
  rect(0, 0, width, 50);

  fill(0, 220, 255);
  textAlign(LEFT, CENTER);
  textSize(18);
  text("I2C Passive Logic Analyzer & Decoder", 20, 25);

  // Pause / Freeze Button
  if (isPaused) {
    fill(255, 80, 80);
    rect(width - 160, 12, 140, 26, 6);
    fill(255);
    textAlign(CENTER, CENTER);
    textSize(13);
    text("PAUSED (Space)", width - 90, 25);
  } else {
    fill(40, 180, 100);
    rect(width - 160, 12, 140, 26, 6);
    fill(255);
    textAlign(CENTER, CENTER);
    textSize(13);
    text("RUNNING (Space)", width - 90, 25);
  }

  fill(180, 200, 220);
  textSize(12);
  textAlign(RIGHT, CENTER);
  text("Port: " + portName, width - 180, 25);
}

void drawWaveformGraphs() {
  float graphX = 80;
  float graphW = width - 100;

  // SCL Waveform Panel
  stroke(50, 65, 80);
  fill(15, 20, 28);
  rect(graphX, 65, graphW, 100, 6);

  fill(255, 220, 0); // Yellow for SCL
  textAlign(RIGHT, CENTER);
  textSize(13);
  text("SCL (Pin 3)", graphX - 10, 115);

  drawSignalWaveform(sclHistory, graphX, 65, graphW, 100, color(255, 220, 0));

  // SDA Waveform Panel
  stroke(50, 65, 80);
  fill(15, 20, 28);
  rect(graphX, 175, graphW, 100, 6);

  fill(0, 220, 255); // Cyan for SDA
  textAlign(RIGHT, CENTER);
  text("SDA (Pin 2)", graphX - 10, 225);

  drawSignalWaveform(sdaHistory, graphX, 175, graphW, 100, color(0, 220, 255));
}

void drawSignalWaveform(int[] history, float x, float y, float w, float h, color c) {
  stroke(c);
  strokeWeight(2);
  noFill();

  float stepX = w / (float)(maxSamples - 1);
  float yHigh = y + 20;
  float yLow = y + h - 20;

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
  float logY = 290;
  float logW = width - 40;
  float logH = height - logY - 20;

  stroke(50, 65, 80);
  fill(12, 16, 22);
  rect(logX, logY, logW, logH, 6);

  fill(0, 200, 255);
  textAlign(LEFT, TOP);
  textSize(14);
  text("Decoded I2C Bus Traffic (Processing Decoder, Microseconds, Hex Address & Data):", logX + 15, logY + 12);

  stroke(40, 50, 65);
  line(logX + 15, logY + 35, logX + logW - 15, logY + 35);

  textFont(createFont("Monospaced", 13));
  float textY = logY + 45;

  for (int i = 0; i < eventLog.size(); i++) {
    String logLine = eventLog.get(i);

    if (logLine.contains("ADDR")) fill(255, 220, 0);       // Yellow
    else if (logLine.contains("DATA")) fill(0, 220, 255);   // Cyan
    else if (logLine.contains("START")) fill(0, 255, 120);  // Green
    else if (logLine.contains("STOP")) fill(255, 100, 100);  // Red
    else fill(180, 200, 220);

    text(logLine, logX + 15, textY);
    textY += 18;
  }
}
