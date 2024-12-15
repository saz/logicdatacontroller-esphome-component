#include "logic_data_controller.h"
#include "esphome/core/log.h"

int logicdata_rx_pin;

namespace esphome {
namespace logic_data_controller {

static const char *TAG = "logic_data_controller";

LogicData ld(-1);

void IRAM_ATTR logicDataPin_ISR() {
    ld.PinChange(HIGH == digitalRead(logicdata_rx_pin));
}

const char *ldc_operation_to_str(LDCOperation op) {
    switch (op) {
        case OPERATION_IDLE:
            return "IDLE";
        case OPERATION_RAISING:
            return "RAISING";
        case OPERATION_LOWERING:
            return "LOWERING";
        default:
            return "UNKNOWN";
    }
}

const char *ldc_endpoint_to_str(LDCEndpoint ep) {
    switch (ep) {
        case ENDPOINT_NONE:
            return "NONE";
        case ENDPOINT_UP:
            return "UP";
        case ENDPOINT_DOWN:
            return "DOWN";
        default:
            return "UNKNOWN";
    }
}

void LogicDataController::read_height() {
    static uint32_t prev = 0;
    uint32_t msg = ld.ReadTrace();
    char buf[80];
    if (msg) {
        uint32_t now = millis();
        sprintf(buf, "%6ums %s: %s", now - prev, ld.MsgType(msg), ld.Decode(msg));
        Serial.println(buf);
        prev=now;
    }

    if (ld.IsNumber(msg)) {
        auto new_height = ld.GetNumber(msg);
        if (new_height == this->current_pos_) {
            return;
        }
        this->current_pos_ = new_height;
    }
    if (msg)
        uint32_t last_signal = millis();
}

void LogicDataController::move_up() {
    if (this->pin_motor_down != nullptr)
        this->pin_motor_down->digital_write(false);
    if (this->pin_motor_up != nullptr)
        this->pin_motor_up->digital_write(true);
    this->current_operation = OPERATION_RAISING;
    this->current_endpoint = ENDPOINT_NONE;
}

void LogicDataController::move_down() {
    if (this->pin_motor_up != nullptr)
        this->pin_motor_up->digital_write(false);
    if (this->pin_motor_down != nullptr)
        this->pin_motor_down->digital_write(true);
    this->current_operation = OPERATION_LOWERING;
    this->current_endpoint = ENDPOINT_NONE;
}

void LogicDataController::move_stop() {
    if (this->pin_motor_up != nullptr)
        this->pin_motor_up->digital_write(false);
    if (this->pin_motor_down != nullptr)
        this->pin_motor_down->digital_write(false);
    if (this->current_operation == OPERATION_RAISING && this->current_pos_ == this->max_table_height_)
        this->current_endpoint = ENDPOINT_UP;
    if (this->current_operation == OPERATION_LOWERING && this->current_pos_ == this->min_table_height_)
        this->current_endpoint = ENDPOINT_DOWN;
    this->target_pos_ = -1;
    this->current_operation = OPERATION_IDLE;
}

void LogicDataController::move_to(int target_pos) {
    if (target_pos == this->current_pos_)
        return;
    if (target_pos > this->current_pos_) {
        move_up();
    } else if (target_pos < this->current_pos_) {
        move_down();
    }
    this->target_pos_ = target_pos;
}

void LogicDataController::setup() {
    this->high_freq_.start();
    // init movement as stopped
    move_stop();

    logicdata_rx_pin = this->pin_sensor->get_pin();
    logicDataPin_ISR();
    attachInterrupt(digitalPinToInterrupt(logicdata_rx_pin), logicDataPin_ISR, CHANGE);
    ld.Begin();

    // initialize height reading
    if (this->request_time_ == 0)  {
        delay(100);
        move_up();
        this->request_time_ = millis();
    }
}

void LogicDataController::loop() {
    read_height();

    if (this->request_time_ > 0 && this->current_pos_ > 0) {
        move_stop();
        this->request_time_ = 0;
        if (this->current_pos_ == this->max_table_height_)
            this->current_endpoint = ENDPOINT_UP;
        if (this->current_pos_ == this->min_table_height_)
            this->current_endpoint = ENDPOINT_DOWN;
    }

    if (this->height_sensor_ != nullptr && (!this->height_sensor_->has_state() || this->height_sensor_->state != this->current_pos_))
        this->height_sensor_->publish_state(this->current_pos_);

    if (this->current_operation == OPERATION_RAISING && this->current_pos_ == this->max_table_height_)
        move_stop();
    
    if (this->current_operation == OPERATION_LOWERING && this->current_pos_ == this->min_table_height_)
        move_stop();

    if (this->target_pos_ >= 0 && this->current_operation != OPERATION_IDLE && this->target_pos_ == this->current_pos_)
        move_stop();
}

void LogicDataController::dump_config() {
    ESP_LOGCONFIG(TAG, "LogicDataController desk:");
    LOG_SENSOR("", "Height", this->height_sensor_);
    LOG_PIN("Motor up pin: ", this->pin_motor_up);
    LOG_PIN("Motor down pin: ", this->pin_motor_down);
    LOG_PIN("Sensor pin: ", this->pin_sensor);
}

} // namespace logic_data_controller
} // namespace esphome