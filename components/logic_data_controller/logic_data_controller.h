#pragma once

#include "esphome.h"

namespace esphome {
namespace logic_data_controller {

enum LDCOperation : uint8_t {
    OPERATION_IDLE = 0,
    OPERATION_RAISING,
    OPERATION_LOWERING,
};

const char *ldc_operation_to_str(LDCOperation op);

enum LDCEndpoint : uint8_t {
    ENDPOINT_NONE = 0,
    ENDPOINT_UP,
    ENDPOINT_DOWN,
};

const char *ldc_endpoint_to_str(LDCEndpoint ep);

class LogicDataController : public Component, public sensor::Sensor {
    public:
        float get_setup_priority() const override { return setup_priority::LATE; }
        void setup() override;
        void loop() override;
        void dump_config() override;

        void set_height_sensor(sensor::Sensor *sensor) { this->height_sensor_ = sensor; }
        void set_pin_motor_up(GPIOPin *pin) { this->pin_motor_up = pin; }
        void set_pin_motor_down(GPIOPin *pin) { this->pin_motor_down = pin; }
        void set_pin_sensor(InternalGPIOPin *pin) { this->pin_sensor = pin; }
        void set_min_height(uint8_t height) { this->min_table_height_ = height; }
        void set_max_height(uint8_t height) { this->max_table_height_ = height; }

        void read_height();
        void move_up();
        void move_down();
        void move_stop();
        void move_to(int target_pos);

        LDCOperation current_operation{OPERATION_IDLE};
        LDCEndpoint current_endpoint{ENDPOINT_NONE};

    private:
        uint8_t min_table_height_{0};
        uint8_t max_table_height_{0};

    protected:
        sensor::Sensor *height_sensor_{nullptr};
        GPIOPin *pin_motor_up{nullptr};
        GPIOPin *pin_motor_down{nullptr};
        InternalGPIOPin *pin_sensor{nullptr};
        uint64_t request_time_{0};
        uint8_t current_pos_{0};
        int target_pos_{-1};
        HighFrequencyLoopRequester high_freq_;
};

} // namespace logic_data_controller
} // namespace esphome