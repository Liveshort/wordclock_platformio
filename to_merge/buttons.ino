volatile unsigned long last_interrupt_time = 0;


void IRAM_ATTR button22_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        button22_pressed = true;
        last_interrupt_time = interrupt_time;
    }
}


void IRAM_ATTR button23_isr() {
    unsigned long interrupt_time = millis();
    if (interrupt_time - last_interrupt_time > 200) {
        button23_pressed = true;
        last_interrupt_time = interrupt_time;
    }
}


void setup_buttons() {
    pinMode(22, INPUT_PULLUP);
    pinMode(23, INPUT_PULLUP);
    attachInterrupt(digitalPinToInterrupt(22), button22_isr, FALLING);
    attachInterrupt(digitalPinToInterrupt(23), button23_isr, FALLING);
}