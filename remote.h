#ifndef __REMOTE_H_
#define __REMOTE_H_

class Remote {
  private:
    uint32_t _ID ;
    uint8_t _len;
    uint16_t _t_delay; // us
    bool _two_phase;
    const uint8_t* _addons;
    uint8_t _num_addons;
    uint8_t _addon_len;

    const uint8_t REPEAT = 5; // or 5 if better

    inline bool bool_from_char(uint32_t& str, uint8_t pos) {
      return str & (1 << pos);
    };

    void send_twoPhase(bool bit) {
      onOff();
      if (bit) {
        delayMicroseconds(4 * _t_delay);
        onOff();
      } else {
        onOff();
        delayMicroseconds(4 * _t_delay);
      }
    };

    void send_singlePhase(bool bit) {
      if (bit) {
        digitalWrite(_pin, 1);
        delayMicroseconds(3 * _t_delay);
        digitalWrite(_pin, 0);
        delayMicroseconds(_t_delay);
      } else {
        onOff();
        delayMicroseconds(2 * _t_delay);
      }
    };

    void onOff() {
      digitalWrite(_pin, 1);
      delayMicroseconds(_t_delay);
      digitalWrite(_pin, 0);
      delayMicroseconds(_t_delay);
    }

    static uint8_t _pin;

  public:
    Remote(uint32_t ID, uint8_t len , uint16_t t_delay, bool two_phase = false, const uint8_t* addons = NULL, uint8_t num_addons = 0, uint8_t addon_len = 0): _ID(ID), _len(len),
      _t_delay(t_delay), _two_phase(two_phase), _addons(addons),  _num_addons(num_addons), _addon_len(addon_len) {};

    void send(uint8_t addon_num = 0) {
      uint32_t tmp = _ID;
      if (addon_num >= _num_addons)
        addon_num = 0;
      if (_addons != 0 && _num_addons > addon_num && _addon_len > 0) {
        tmp <<= _addon_len;
        tmp += _addons[addon_num];
      }

      for (uint8_t a = 0; a < REPEAT; a += 1) {
        if (_two_phase) {
          digitalWrite(_pin, 1);
          delayMicroseconds(_t_delay);
          digitalWrite(_pin, 0);
          delayMicroseconds(9 * _t_delay);
        }
        for (int16_t b = _len + _addon_len - 1; b >= 0; b -= 1) {
          if (_two_phase) {
            send_twoPhase(bool_from_char(tmp, b));
          } else {
            send_singlePhase(bool_from_char(tmp, b));
          }
        }
        onOff();
        if (_two_phase)
          delayMicroseconds(39 * _t_delay);
        else
          delayMicroseconds(30 * _t_delay);

      }
      digitalWrite(_pin, 0);
    };

    static void setPin(uint8_t pin) {
      _pin = pin;
      pinMode(_pin, OUTPUT);
      digitalWrite(_pin, 0);
    };

    static void blinkPin() {
      digitalWrite(_pin, 1);
      delayMicroseconds(50);
      digitalWrite(_pin, 0);
    }
};
uint8_t Remote::_pin = 0; // only for compiler reasons - set with setPin()
#endif
