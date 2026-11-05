void runTests();

#ifdef ARDUINO
#include <Arduino.h>

void setup() {
  delay(2000);
  runTests();
}

void loop() {}
#else
int main() {
  runTests();
  return 0;
}
#endif
