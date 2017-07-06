// -----------------  Аналоговый вход A0
void initA0() {
  HTTP.on("/analog.json", HTTP_GET, []() {
    HTTP.send(200, "text/json", graf(analogRead(A0), 10, 3000));
  });
  modulesReg("analog");
}
// ----------------- Кнопка GPIO 0
void initTach() {
  // Кнопка будет работать по прерыванию
  uint8_t TACH_PIN = 0;
  attachInterrupt(TACH_PIN, Tach_0, RISING); //прерывание сработает, когда состояние вывода изменится с низкого уровня на высокий
  configJson = jsonWrite(configJson, "TachCommand", readArgsString());
}
// Выполняется при нажатии на кнопку
void Tach_0() {
  static unsigned long millis_prev;
  // Устроняем дребезг контакта
  if (millis() - 100 > millis_prev) {
    command = jsonRead(configJson, "TachCommand");
  }
  millis_prev = millis();
}

// -----------------  DHT
void initDHT() {
  dht.setup(readArgsInt());
  //delay(dht.getMinimumSamplingPeriod());
  delay (1000);

  String temp = "";
  temp += dht.getTemperature();
  if (temp != "nan") {
    Serial.println("ok");
    HTTP.on("/sensor.json", HTTP_GET, []() {
      float temp = dht.getTemperature();
      if (temp == 'NaN') {
        temp = 0;
      }
      HTTP.send(200, "text/json", graf(temp, 10, 3000));
    });
    modulesReg("temperature");
  }
}

// ---------- initD18B20
void initD18B20() {
  pinMode(d18b20PIN, INPUT_PULLUP);
  d18b20.begin();
  d18b20.requestTemperatures();
  float ok = d18b20.getTempCByIndex(0);
  Serial.println(ok);
  if (ok != -127) {
    HTTP.on("/sensor.json", HTTP_GET, []() {
      d18b20.requestTemperatures();
      float temp = d18b20.getTempCByIndex(0);
      if (temp == -127) {
        temp = 0;
      }
      HTTP.send(200, "text/json", graf(temp, 10, 3000));
    });
    modulesReg("temperature");
  }
}
// ----------------------Приемник на 433мГ
void initRCSwitch() {
  Serial.end();
  mySwitch.enableReceive(readArgsInt());
  // задача опрашивать RC код
  ts.add(3, 100, [&](void*) {
    RCRCreceiv();
  }, nullptr, true);
  modulesReg("RCreceivi");
}

void RCRCreceiv() {
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 0) {
      configJson = jsonWrite(configJson, "Received", 0);
    } else {
      Serial.print("Received ");
      configJson = jsonWrite(configJson, "Received", mySwitch.getReceivedValue());
    }
    mySwitch.resetAvailable();
  }
}


