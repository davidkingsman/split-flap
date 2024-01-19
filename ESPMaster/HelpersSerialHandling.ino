//Offers option to call these methods to do serial output and not have to do 
//definition checks with every call
template <typename T>
void SerialPrint(T value) {
#if SERIAL_ENABLE == true
    Serial.print(value);
#endif
}

template <typename T>
void SerialPrintf(const char* message, T value) {
#if SERIAL_ENABLE == true
    Serial.printf(message, value);
#endif
}

template <typename T>
void SerialPrintln(T value) {
#if SERIAL_ENABLE == true
    Serial.println(value);
#endif
}
