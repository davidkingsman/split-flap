
String centerString(String message) {
    String emptySpace;
    int messageLength = message.length();
    int emptySpaceAmount = (UNITSAMOUNT - messageLength) / 2;
    for (int i = 0; i < emptySpaceAmount; i++) {
        emptySpace = " " + emptySpace;
    }
    message = emptySpace + message;
    message = cleanString(message);
    return message;
}

String rightString(String message) {
    message = cleanString(message);
    return message;
}

// aligns string on left side of array and fills empty chars with spaces
String leftString(String message) {
    message = cleanString(message);

    char leftAlignString[UNITSAMOUNT + 1];

    for (int i = 0; i < UNITSAMOUNT + 1; i++) {
        if (i < message.length()) {
            leftAlignString[i] = message[i];
        } else if (i == UNITSAMOUNT) {
            leftAlignString[i] = '\0';
        } else {
            leftAlignString[i] = ' ';
        }
    }
    // Serial.println(leftAlignString);
    return leftAlignString;
}

// converts input string to uppercase
String cleanString(String message) {
    message.toUpperCase();
    return message;
}

int convertSpeed(String speedSlider) {
    int speedSliderInt;
    speedSliderInt = speedSlider.toInt();
    speedSliderInt = map(speedSliderInt, 1, 100, MINSPEED, MAXSPEED);
    return speedSliderInt;
}
