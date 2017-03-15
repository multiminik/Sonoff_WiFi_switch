/*
    Для использования вкладки требуется добавить в заголовке скетча следуюший код
  #include <PubSubClient.h>
  Глобальные переменные
  //Mqtt
  String mqtt_server = "cloudmqtt.com"; // Имя сервера MQTT
  int mqtt_port = 15535; // Порт для подключения к серверу MQTT
  String mqtt_user = ""; // Логи от сервер
  String mqtt_pass =""; // Пароль от сервера
  bool LedState = false;
  int tm = 0;
  float temp = 0;
  WiFiClient wclient;
  PubSubClient client(wclient);
*/


// Функция получения данных от сервера

void callback(const MQTT::Publish& pub)
{
  Serial.print(pub.topic()); // выводим в сериал порт название топика
  Serial.print(" => ");
  Serial.print(pub.payload_string()); // выводим в сериал порт значение полученных данных
  Serial.println();
  String payload = pub.payload_string();
  Serial.println(payload);

  if (String(pub.topic()) == chipID + "/RELE_1") // проверяем из нужного ли нам топика пришли данные
  {
    //Serial.println();
    int stled = payload.toInt(); // преобразуем полученные данные в тип integer
    if (stled != state0) {
      chaing = 1;
    }
    Serial.println(stled);
  }
}

void MQTT_init() {
  Serial.println("MQTT");
  chipID += String( ESP.getChipId() ) + "-" + String( ESP.getFlashChipId() );
  Serial.println(chipID);
  mqtt_ConfigJSON();
  pinMode(LED_PIN, OUTPUT);
  HTTP.on("/mqtt", handle_Set_MQTT);
  modulesReg("mqtt");
  MQTT_Pablush();
}

void MQTT_Pablush() {
  client.set_server(mqtt_server, mqtt_port);
  // подключаемся к MQTT серверу
  if (WiFi.status() == WL_CONNECTED) {
    if (!client.connected()) {
      Serial.println("Connecting to MQTT server");
      if (client.connect(MQTT::Connect(chipID)
                         .set_auth(mqtt_user, mqtt_pass))) {
        Serial.println("Connected to MQTT server");
        client.set_callback(callback);
        client.subscribe(chipID + "/RELE_1"); // подписывааемся по топик с данными для светодиода
        } else {
        Serial.println("Could not connect to MQTT server");
      }
    }
  }
}

void  handleMQTT() {
  if (client.connected()) client.loop();
  //if (tm) {
  //  float temp = analogRead(A0);
   // client.publish(chipID + "/RELE_1", String(temp)); // отправляем в топик для термодатчика значение температуры
   // tm = 0;
  //}
}

//Установка параметров  http://192.168.0.101/mqtt?server=m13.cloudmqtt.com&port=15535&user=cxluynva&pass=4cje5WEkzqvR
void handle_Set_MQTT() {              //
  mqtt_server = HTTP.arg("server");         // Получаем значение server из запроса сохраняем в глобальной переменной
  mqtt_port = HTTP.arg("port").toInt(); // Получаем значение port из запроса сохраняем в глобальной переменной
  mqtt_user = HTTP.arg("user"); // Получаем значение user из запроса сохраняем в глобальной переменной
  mqtt_pass = HTTP.arg("pass"); // Получаем значение pass из запроса сохраняем в глобальной переменной
  saveConfig();                         // Функция сохранения данных во Flash
  mqtt_ConfigJSON();
  client.disconnect();
  MQTT_Pablush();
  HTTP.send(200, "text/plain", "OK");   // отправляем ответ о выполнении
}

void mqtt_ConfigJSON() {
  // Резервируем память для json обекта буфер может рости по мере необходимти, предпочтительно для ESP8266
  DynamicJsonBuffer jsonBuffer;
  //  вызовите парсер JSON через экземпляр jsonBuffer
  JsonObject& json = jsonBuffer.parseObject(jsonConfig);
  // Заполняем поля json

  json["mqttServer"] = mqtt_server;
  json["mqttPort"] = mqtt_port;
  json["mqttUser"] = mqtt_user;
  json["mqttPass"] = mqtt_pass;

  // Помещаем созданный json в переменную root
  jsonConfig = "";
  json.printTo(jsonConfig);
  HTTP.send(200, "text/json", jsonConfig);
}

