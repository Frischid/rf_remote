#ifndef ADVANCED_PAD
void send_sequence(uint8_t sequence) {
  switch (sequence) {
    case 0:
    case 1:
    case 2:
    case 3:
      it.send(sequence + 2);
      break;
    case HIGHEST_SEQUENCE: // all off
      it.send(0);
      break;
    case 4:
    case 5:
      sl.send();
      break;
  }
}
#else
#define send_sequence(sequence) send_sequence_adv(sequence)
volatile uint8_t last = 0;
void send_sequence_adv(uint8_t sequence) {
  switch (sequence) {
    case 0:
    case 1:
    case 2:
    case 3:
      last = sequence % 2 == 1 ? sequence - 1 : sequence + 1;
      Remote::blinkPin();
      break;
    case HIGHEST_SEQUENCE: // all off
      it.send(0);
      break;
    case 4:
      if (last == 3)
        sl.send();
      else
        it.send(); //2 * last + sequence % 4 + 2
      break;
    case 5:
      if (last == 3)
        sl.send();
      else
        sl.send();
      break;
  }
}
#endif
