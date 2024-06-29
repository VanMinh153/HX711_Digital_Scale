#if defined(CONFIG_IDF_TARGET_ESP32C3)
const float CONFIG_MICRO_SECOND = (float)CONFIG_ESP32C3_DEFAULT_CPU_FREQ_MHZ/31;
#elif defined(CONFIG_IDF_TARGET_ESP32S2)
const float CONFIG_MICRO_SECOND = (float)CONFIG_ESP32S2_DEFAULT_CPU_FREQ_MHZ/45;
#endif

void setup() {
  Serial.begin(115200);
  Serial.println("Welcome");
  Serial.println(CONFIG_MICRO_SECOND);
}

unsigned long start, finish, duration;

void loop() {
  start = esp_timer_get_time();
  finish = esp_timer_get_time();
  duration = finish - start;
  Serial.print(String(duration) + ", ");

  start = esp_timer_get_time();
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  delay_us(1);
  finish = esp_timer_get_time();
  duration = finish - start;
  Serial.print(String(duration) + ", ");

  start = esp_timer_get_time();
  delay_us(1000);
  finish = esp_timer_get_time();
  duration = finish - start;
  Serial.print(String(duration) + "\n");
  
  delay(100);
}

FORCE_INLINE_ATTR void delay_us(int us) {
  for (int i = 0; i < us*CONFIG_MICRO_SECOND; i++)
    NOP();
}