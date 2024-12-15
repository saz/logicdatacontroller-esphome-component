import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import pins
from esphome.components import sensor, gpio
from esphome.const import CONF_ID, CONF_HEIGHT, ICON_GAUGE, UNIT_CENTIMETER

MULTI_CONF = True

AUTO_LOAD = ["sensor", "gpio"]
DEPENDENCIES = ["gpio"]

logic_data_controller_ns = cg.esphome_ns.namespace('logic_data_controller')
LogicDataController = logic_data_controller_ns.class_('LogicDataController', cg.Component)

CONF_PIN_MOTOR_UP = "pin_motor_up"
CONF_PIN_MOTOR_DOWN = "pin_motor_down"
CONF_PIN_SENSOR = "pin_sensor"
CONF_MIN_HEIGHT = "min_height"
CONF_MAX_HEIGHT = "max_height"

CONFIG_SCHEMA = cv.COMPONENT_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(LogicDataController),
    cv.Required(CONF_PIN_MOTOR_UP): pins.gpio_output_pin_schema,
    cv.Required(CONF_PIN_MOTOR_DOWN): pins.gpio_output_pin_schema,
    cv.Required(CONF_PIN_SENSOR): pins.gpio_input_pin_schema,
    cv.Required(CONF_HEIGHT): sensor.sensor_schema(
        unit_of_measurement=UNIT_CENTIMETER,
        icon=ICON_GAUGE,
        accuracy_decimals=0,
    ),
    cv.Required(CONF_MIN_HEIGHT): cv.positive_int,
    cv.Required(CONF_MAX_HEIGHT): cv.positive_int,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    if CONF_PIN_MOTOR_UP in config:
        pin = await cg.gpio_pin_expression(config[CONF_PIN_MOTOR_UP])
        cg.add(var.set_pin_motor_up(pin))

    if CONF_PIN_MOTOR_DOWN in config:
        pin = await cg.gpio_pin_expression(config[CONF_PIN_MOTOR_DOWN])
        cg.add(var.set_pin_motor_down(pin))

    if CONF_PIN_SENSOR in config:
        pin = await cg.gpio_pin_expression(config[CONF_PIN_SENSOR])
        cg.add(var.set_pin_sensor(pin))

    if CONF_HEIGHT in config:
        sens = await sensor.new_sensor(config[CONF_HEIGHT])
        cg.add(var.set_height_sensor(sens))

    if CONF_MIN_HEIGHT in config:
        cg.add(var.set_min_height(config[CONF_MIN_HEIGHT]))

    if CONF_MAX_HEIGHT in config:
        cg.add(var.set_max_height(config[CONF_MAX_HEIGHT]))