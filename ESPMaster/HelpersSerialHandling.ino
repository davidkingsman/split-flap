//Offers option to call these methods to do serial output and not have to do 
//definition checks with every call
template <typename T>
void SerialPrint(T value) {
#ifdef SERIAL_ENABLE
    Serial.print(value);
#endif
}

template <typename T>
void SerialPrintf(const char* message, T value) {
#ifdef SERIAL_ENABLE
    Serial.printf(message, value);
#endif
}

template <typename T>
void SerialPrintln(T value) {
#ifdef SERIAL_ENABLE
    Serial.println(value);
#endif
}
