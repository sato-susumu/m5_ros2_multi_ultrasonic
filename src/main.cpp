#include <M5Unified.h>
#include <micro_ros_platformio.h>

#include <Unit_Sonic.h>
#include "ClosedCube_TCA9548A.h"

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <micro_ros_utilities/string_utilities.h>
#include <sensor_msgs/msg/range.h>

#define RCCHECK(fn)                               \
  {                                               \
    rcl_ret_t temp_rc = fn;                       \
    if ((temp_rc != RCL_RET_OK))                  \
    {                                             \
      M5Controller::showError(__LINE__, temp_rc); \
    }                                             \
  }
#define RCSOFTCHECK(fn)          \
  {                              \
    rcl_ret_t temp_rc = fn;      \
    if ((temp_rc != RCL_RET_OK)) \
    {                            \
    }                            \
  }

#if USE_ATOMS3
#define MainSerial USBSerial
#else
#define MainSerial Serial
#endif

constexpr uint8_t PAHUB2_I2C_ADDRESS = 0x70;
constexpr uint8_t SONIC_SENSOR_NUM = 3;

class M5Controller
{
public:
  static void begin(const char *node_name)
  {
    M5.begin();
    M5.Lcd.setTextSize(2);
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("%s\n", node_name);
  }

  static void update()
  {
    M5.update();
  }

  static void showError(int line_no, int ret_code)
  {
    bool isBlack = true; // Flag to track the color of the screen

    while (1)
    {
      M5.Lcd.fillScreen(isBlack ? BLACK : RED); // Switch color depending on the condition
      M5.Lcd.setCursor(0, 0);
      M5.Lcd.print("ERROR\n");
      M5.Lcd.printf("line %d: %d\n", line_no, ret_code);
      delay(1000);
      isBlack = !isBlack; // Invert the flag for the next loop to switch color
    }
  }

  static void updateDisplay(const float *distance)
  {
    M5.Lcd.fillScreen(BLACK);
    M5.Lcd.setCursor(0, 0);
    M5.Lcd.setTextSize(4);

    for (uint8_t channel = 0; channel < SONIC_SENSOR_NUM; channel++)
    {
      M5.Lcd.printf("%3d\n", (int)distance[channel] / 10);
    }
  }
};

class MicroROSNode
{
public:
  MicroROSNode() : node(), publisher()
  {
    allocator = rcl_get_default_allocator();
  }

  void begin(const char *node_name)
  {
    MainSerial.begin(115200);
    set_microros_serial_transports(MainSerial);

    while(1) {
      auto result = rclc_support_init(&support, 0, NULL, &allocator);
      if(result == RCL_RET_OK) {
        break;
      }
      delay(2000);
    }
    RCCHECK(rclc_node_init_default(&node, node_name, "", &support));

    for (uint8_t channel = 0; channel < SONIC_SENSOR_NUM; channel++)
    {

      RCCHECK(rclc_publisher_init_default(
          &publisher[channel],
          &node,
          ROSIDL_GET_MSG_TYPE_SUPPORT(sensor_msgs, msg, Range),
          getTopicName(channel)));
    }
  }

  void spinSome()
  {
    // If there is an executor, call spin
  }

  void publishMessage(int channel, float distance)
  {
    // // Not support timestamp.
    //
    // struct timespec tv = {0};
    // clock_gettime(0, &tv);

    range_msg[channel].header.frame_id = micro_ros_string_utilities_set(range_msg[channel].header.frame_id, getTopicName(channel));
    // range_msg[channel].header.stamp.sec = tv.tv_sec;
    // range_msg[channel].header.stamp.nanosec = tv.tv_nsec;
    range_msg[channel].radiation_type = sensor_msgs__msg__Range__ULTRASOUND;
    range_msg[channel].min_range = 0.02;
    range_msg[channel].max_range = 4.5;
    range_msg[channel].field_of_view = 0.5236;  // 30 degrees = 0.5236 radians
    range_msg[channel].range = distance / 1000;  // mm -> m

    RCSOFTCHECK(rcl_publish(&publisher[channel], &range_msg[channel], NULL));
  }

private:
  rcl_node_t node;
  rclc_support_t support;
  rcl_allocator_t allocator;

  sensor_msgs__msg__Range range_msg[SONIC_SENSOR_NUM];
  rcl_publisher_t publisher[SONIC_SENSOR_NUM];

  const char *getTopicName(uint8_t channel)
  {
    static char topicName[20];
    snprintf(topicName, sizeof(topicName), "/ultrasonic%d", channel);
    return topicName;
  }
};

class UltrasonicSensorArray
{
public:
  UltrasonicSensorArray() : tca9548a(PAHUB2_I2C_ADDRESS), sensor()
  {
  }

  void begin()
  {
    sensor.begin();
  }

  float getDistance(uint8_t channel)
  {
    tca9548a.selectChannel(channel);
    return sensor.getDistance();
  }

private:
  ClosedCube::Wired::TCA9548A tca9548a;
  SONIC_I2C sensor;
};

MicroROSNode node;
UltrasonicSensorArray ultrasonicSensorArray;

void setup()
{
  const char *node_name = "m5_ros2_multi_ultrasonic";
  M5Controller::begin(node_name);
  node.begin(node_name);
  ultrasonicSensorArray.begin();
}

void loop()
{
  M5Controller::update();
  node.spinSome();

  float distance[SONIC_SENSOR_NUM];
  for (uint8_t channel = 0; channel < SONIC_SENSOR_NUM; channel++)
  {
    distance[channel] = ultrasonicSensorArray.getDistance(channel);
    node.publishMessage(channel, distance[channel]);
  }

  M5Controller::updateDisplay(distance);
  delay(10);
}
