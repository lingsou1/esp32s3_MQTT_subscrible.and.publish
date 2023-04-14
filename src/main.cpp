/*
接线说明:无

程序说明:开发板同时发布信息以及订阅消息,发布两个主题的信息,一个固定3秒发布一次,一个根据开发板按键来控制是否发布,同时订阅三个主题,开发板的灯会一直亮
        当按下按键后一段时间(600ms)开发板的灯会灭,程序建立了两对WiFiClient PubSubClient对象以实现发布和订阅消息,但是灯的控制不是很好,我也不是很想
        弄了,基本实现了在同一个开发板发布以及订阅消息


注意事项: 1)尽量不要在loop()函数中使用延时函数,可能会对MQTT服务有影响,所以使用 #include <Ticker.h> 这个库来定时执行任务,以替代延时
        2)开发板同时担任两个角色,需要两个名称
        3)发布消息时使用多级主题时其中的主题是不能使用通配符码

函数示例:无

作者:灵首

时间:2023_4_14

*/
#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <WiFiMulti.h>
#include <Ticker.h>

WiFiMulti wifi_multi; // 建立WiFiMulti 的对象,对象名称是 wifi_multi
WiFiClient wifiClient_sub;  //建立WiFiClient(在订阅端)
WiFiClient wifiClient_pub;  //建立WiFiClient(在发布端)
PubSubClient mqttClient_sub(wifiClient_sub);  //根据WiFiClient来建立PubSubClient对象,这是订阅端的对象
PubSubClient mqttClient_pub(wifiClient_pub);  //根据WiFiClient来建立PubSubClient对象,这是发布端的对象
Ticker ticker;    //建立定时对象,3秒的那个任务
Ticker ticker_1;    //建立定时对象,200ms的那个任务


const char* mqttServer = "test.ranye-iot.net";  //这是需要连接的MQTT服务器的网址,可更改
int count;  //用来实现延时的计数
int count_1;

#define LED_A 10
#define LED_B 11
#define BOOT  0


/**
* @brief 连接WiFi的函数
*
* @param 无
* @return 无
*/
void wifi_multi_con(void)
{
  int i = 0;
  while (wifi_multi.run() != WL_CONNECTED)
  {
    delay(1000);
    i++;
    Serial.print(i);
  }
}



/**
* @brief 写入自己要连接的WiFi名称及密码,之后会自动连接信号最强的WiFi
*
* @param 无
* @return 无
*/
void wifi_multi_init(void)
{
  wifi_multi.addAP("haoze2938", "12345678");
  wifi_multi.addAP("LINGSOU1029", "12345678");
  wifi_multi.addAP("haoze1029", "12345678"); // 通过 wifi_multi.addAP() 添加了多个WiFi的信息,当连接时会在这些WiFi中自动搜索最强信号的WiFi连接
}



/**
* @brief 发布MQTT信息,包含建立主题以及发布消息
*
* @param 无
* @return 无
*/
void pubMQTTmsg(){
  static int value; // 客户端发布信息用数字
 
  // 建立发布主题。主题名称以lingsou-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP32s3客户端名称各不相同，
  //同时建立主题后的两句程序是将 string 的数据转换为 char[] 字符数组类型
  //因为在之后的发布操作中只支持字符数组作为参数
  String topicString = "lingsou-" + WiFi.macAddress();  
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());  //将字符串数据 topicString 转换为字符数组类型的数据 publishTopic
 
  // 建立发布信息。信息内容以Hello World为起始，后面添加发布次数。
  String messageString = "Hello World " + String(value++); 
  char publishMsg[messageString.length() + 1];   
  strcpy(publishMsg, messageString.c_str());
  
  // 实现ESP32s3向主题发布信息
  if(mqttClient_pub.publish(publishTopic, publishMsg)){
    Serial.print("Publish Topic:");
    Serial.print(publishTopic);
    Serial.print("\n");
    Serial.print("Publish message:");
    Serial.print(publishMsg);
    Serial.print("\n");    
  } else {
    Serial.print("Message Publish Failed(this is publish and this msg).\n"); 
  }
}



/**
* @brief 发布MQTT信息,包含建立主题以及发布消息,会根据传入的参数控制消息的发布内容
*
* @param int value :根据value这个值控制发布消息的内容,进一步控制开发板的LED
* @return 无
*/
void pubMQTTmsg_1(int value_1){
  int value;
  // 建立发布主题。主题名称以lingsou-为前缀，后面添加设备的MAC地址。
  // 这么做是为确保不同用户进行MQTT信息发布时，ESP32s3客户端名称各不相同，
  //同时建立主题后的两句程序是将 string 的数据转换为 char[] 字符数组类型
  //因为在之后的发布操作中只支持字符数组作为参数
  String topicString = "lingsou-" + WiFi.macAddress() + "/tongpeifu/data";  
  char publishTopic[topicString.length() + 1];  
  strcpy(publishTopic, topicString.c_str());  //将字符串数据 topicString 转换为字符数组类型的数据 publishTopic

  if(value_1 == 0){
    // 建立发布信息。当按键按下后发布 1 ,熄灭灯
    String messageString = "1"; 
    char publishMsg[messageString.length() + 1];   
    strcpy(publishMsg, messageString.c_str());
    // 实现ESP32s3向主题发布信息
    if(mqttClient_pub.publish(publishTopic, publishMsg)){
      Serial.print("Publish Topic:");
      Serial.print(publishTopic);
      Serial.print("\n");
      Serial.print("Publish message:");
      Serial.print(publishMsg);
      Serial.print("\n");    
    } else {
      Serial.print("Message Publish Failed(this is publish and this is msg1).\n"); 
    }
  }
  else{
    value++;
    // 建立发布信息。信息内容以hello,nihao!!! + :为起始，后面添加发布次数。
    String messageString = "hello,nihao!!! + :" + String(value); 
    char publishMsg[messageString.length() + 1];   
    strcpy(publishMsg, messageString.c_str());
    // 实现ESP32s3向主题发布信息
    if(mqttClient_pub.publish(publishTopic, publishMsg)){
      Serial.print("Publish Topic:");
      Serial.print(publishTopic);
      Serial.print("\n");
      Serial.print("Publish message:");
      Serial.print(publishMsg);
      Serial.print("\n");    
    } else {
      Serial.print("Message Publish Failed(this is publish and this is msg1).\n"); 
    }
  }
}



/**
* @brief 订阅相关的主题(一共订阅了3个主题),一个普通主题,一个使用单级通配符,一个使用多级通配符
*
* @param 无
* @return 无
*/
void subscribleTopic(){
  // 建立订阅主题1。主题名称根据发布消息的开发板决定
  // 这么做是为确保不同设备使用同一个MQTT服务器测试消息订阅时，所订阅的主题名称不同
  //需要将字符串转换为字符数组满足库的要求
  //这个主题由另一个开发板建立并发送
  String topicString1 = "lingsou-7C:DF:A1:F9:EB:AC";
  char subTopic1[topicString1.length() + 1];  
  strcpy(subTopic1, topicString1.c_str());
  
  // 建立订阅主题2,使用单级通配符
  //这个主题由MQTTfx建立并发送
  String topicString2 = "lingsou-" + WiFi.macAddress() + "/+/data";
  char subTopic2[topicString2.length() + 1];  
  strcpy(subTopic2, topicString2.c_str());

  // 建立订阅主题3,使用多级通配符
  //这个主题由MQTTfx建立并发送
  String topicString3 = "lingsou-" + WiFi.macAddress() + "/sensor/#";
  char subTopic3[topicString3.length() + 1];  
  strcpy(subTopic3, topicString3.c_str());
  
  // 通过串口监视器输出是否成功订阅主题1以及订阅的主题1名称
  if(mqttClient_sub.subscribe(subTopic1)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic1);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }  
 
  // 通过串口监视器输出是否成功订阅主题2以及订阅的主题2名称
  if(mqttClient_sub.subscribe(subTopic2)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic2);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }

  // 通过串口监视器输出是否成功订阅主题3以及订阅的主题3名称
  if(mqttClient_sub.subscribe(subTopic3)){
    Serial.println("Subscrib Topic:");
    Serial.println(subTopic3);
    Serial.print("\n");
  } else {
    Serial.print("Subscribe Fail...");
    Serial.print("\n");
  }
}



/**
* @brief 生成客户端名称(发布端)并连接服务器同时订阅主题并串口输出数据
*
* @param 无
* @return 无
*/
void connectMQTTServer_pub(){
  //生成客户端的名称(同一个服务器下不能存在两个相同的客户端名称)
  String clientId = "esp32s3-pub-" + WiFi.macAddress();

  //尝试连接服务器,并通过串口输出有关信息
  if(mqttClient_pub.connect(clientId.c_str())){
    Serial.println("MQTT Server Connect successfully!!!.(this is publish)\n");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.print("\n");
    Serial.println("ClientId:");
    Serial.println(clientId);
    Serial.print("\n");
  }
  else{
    Serial.print("MQTT Server Connect Failed(this is publish). Client State:");
    Serial.println(mqttClient_pub.state());
    Serial.print("\n");
    delay(3000);
  }

}



/**
* @brief 生成客户端(订阅端)名称并连接服务器同时串口输出数据
*
* @param 无
* @return 无
*/
void connectMQTTServer_sub(){
  //生成客户端的名称(同一个服务器下不能存在两个相同的客户端名称)
  String clientId = "esp32s3-sub-" + WiFi.macAddress();

  //尝试连接服务器,并通过串口输出有关信息
  if(mqttClient_sub.connect(clientId.c_str())){
    Serial.println("MQTT Server Connect successfully!!!.(this is subscrible)\n");
    Serial.println("Server Address: ");
    Serial.println(mqttServer);
    Serial.print("\n");
    Serial.println("ClientId:");
    Serial.println(clientId);
    Serial.print("\n");
    
    //连接服务器后订阅主题
    subscribleTopic();
  }
  else{
    Serial.print("MQTT Server Connect Failed. Client State:");
    Serial.println(mqttClient_sub.state());
    Serial.print("\n");
    delay(3000);
  }

}



/**
* @brief 这是一个回调函数,当订阅的主题有消息发布时就会调用该函数,参数是固定的(PunSubClient中固定的),不能自己修改
*
* @param char* topic :这是订阅的主题名
* @param byte* payload :这是传回的消息
* @param unsigned int length :这是消息长度
* @return 无
*/
void recieveCallback(char* topic, byte* payload, unsigned int length){
  //输出订阅的主题名称
  Serial.print("Message Received [");
  Serial.print(topic);
  Serial.print("] ");
  Serial.print("\n");

  //输出订阅的主题中的消息
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println("");
  Serial.print("Message Length(Bytes) ");
  Serial.println(length);
  Serial.print("\n");

 //根据主题信息控制LED灯的亮灭,实现按下按键点亮LED灯
  if ((char)payload[0] == '1') {     // 如果收到的信息以“0”为开始
    digitalWrite(LED_A, 0);  // 则熄灭LED。
    digitalWrite(LED_B, 0);  // 则熄灭LED。
  } else {                           
    digitalWrite(LED_A, 1); // 否则点亮LED。
    digitalWrite(LED_B, 1); // 否则点亮LED。
  }

}



/**
* @brief 计数,用来实现替代延时实现3秒发布一次MQTT信息
*
* @param 无
* @return 无
*/
void tickerCount(){
  count++;
}



/**
* @brief 计数,用来实现替代延时实现200ms对BOOT进行一次按键检测
*
* @param 无
* @return 无
*/
void ticker_1Count(){
  count_1++;
}



void setup() {
  // 连接串口
  Serial.begin(9600);
  Serial.print("serial is OK\n");

  //led灯设置
  pinMode(LED_A,OUTPUT);
  pinMode(LED_B,OUTPUT);
  digitalWrite(LED_A,0);
  digitalWrite(LED_B,0);

  //按键设置
  pinMode(BOOT,INPUT);

  // wifi 连接设置
  wifi_multi_init();
  wifi_multi_con();
  Serial.print("wifi connected!!!\n");

  // 输出连接信息(连接的WIFI名称及开发板的IP地址)
  Serial.print("\nconnect wifi:");
  Serial.print(WiFi.SSID());
  Serial.print("\n");
  Serial.print("\nIP address:");
  Serial.print(WiFi.localIP());
  Serial.print("\n");

  //设置连接的MQTT服务器
  mqttClient_pub.setServer(mqttServer,1883);
  mqttClient_sub.setServer(mqttServer,1883);

  //设置回调函数处理订阅消息
  mqttClient_sub.setCallback(recieveCallback);

  //连接MQTT服务器
  connectMQTTServer_sub();
  delay(1);
  connectMQTTServer_pub();

  //每一秒执行一次tickerCount函数,来配合MQTT发布消息
  ticker.attach(1,tickerCount);
  ticker_1.attach_ms(150,ticker_1Count);
}



void loop() {

  int BOOTValue = digitalRead(BOOT);

  //这是在发布端的操作
  //检查MQTT连接,若连接则保持心跳
  //若未服务器未连接则尝试重连
  if(mqttClient_pub.connected()){
    //3秒固定发布的MQTT信息
    if(count >= 3){
      pubMQTTmsg();
      count = 0;
    }
    if(count_1 >= 4){
      //检测按键状态
      if(BOOTValue == 0){
      pubMQTTmsg_1(BOOTValue);
      }
      if(BOOTValue == 1){
      pubMQTTmsg_1(BOOTValue);
      }
      count_1 = 0;
    }
    mqttClient_pub.loop();  //这是在保持客户端心跳
  }
  else{
    connectMQTTServer_pub();  //重连服务器
  }


  //这是在订阅端的操作
  if(mqttClient_sub.connected()){
    mqttClient_sub.loop();    //这是在保持客户端心跳
  }
  else{
    connectMQTTServer_sub();  //重连服务器
  }
}