# m5_ros2_multi_ultrasonic
### 概要
M5Atomとmicro-ROSを使って、複数の超音波センサー情報を配信するROS2プログラムです。

### 概要図、作成例

### 動作確認環境
- PlatformIOを使ったビルド環境、micro-ROSエージェント実行環境  
  - PC: TRIGKEY: Green G5  
  - OS: Ubuntu 22.04.3  
  - ROS2 : Humble  
  - Software: PlatformIO, VSCode, docker
<br/>

- micro-ROSクライアント実行環境と周辺ユニット  
  - M5Atom Lite
  - M5Stack用Port.A I2C拡張ハブユニット(PaHUB2 Unit)  
  - M5Stack用超音波測距ユニット I2C 3つ  
<br/>

- micro-ROSクライアントとmicro-ROSエージェントの接続方法  
  - USBケーブルを使ったシリアル通信  
<br/>

### micro-ROSエージェントを起動方法(docker利用時)
シリアル通信のデバイスファイルを確認  
```
$ ls -l /dev/ttyACM* /dev/ttyUSB*
ls: '/dev/ttyACM*' にアクセスできません: そのようなファイルやディレクトリはありません
crw-rw---- 1 root dialout 188, 0  2月 23 20:38  /dev/ttyUSB0
```
<br/>

M5Atomが /dev/ttyUSB0 で接続している場合、下記コマンドでmicro-ROSエージェント起動。  
```
docker run -it --rm -v /dev:/dev -v /dev/shm:/dev/shm --privileged --net=host microros/micro-ros-agent:$ROS_DISTRO serial --dev  /dev/ttyUSB0 -v6
```
<br/>

### 起動順序
エージェント → クライアントの順で起動する必要があります。  
エージェントを起動したら、M5Atomを一度リセットすることでエージェント → クライアントの順になります。  
<br/>

### 動作確認
正しく起動できたら、/ultrasonic0, /ultrasonic1, /ultrasonic2というトピックが作成されます。  
```
$ ros2 topic list
省略
/ultrasonic0
/ultrasonic1
/ultrasonic2
```

Topicの内容
```
$ ros2 topic echo /ultrasonic0
header:
  stamp:
    sec: 0
    nanosec: 0
  frame_id: /ultrasonic0
radiation_type: 0
field_of_view: 0.5235999822616577
min_range: 0.019999999552965164
max_range: 4.5
range: 0.036396000534296036
```

Topicの送信頻度
```
$ ros2 topic hz /ultrasonic0
average rate: 2.381
	min: 0.420s max: 0.420s std dev: 0.00011s window: 4
average rate: 2.381
	min: 0.420s max: 0.420s std dev: 0.00020s window: 7
```
<br/>


### リンク
[m5_ros2_multi_ultrasonic](https://github.com/sato-susumu/m5_ros2_multi_ultrasonic)  
今回作ったプログラム  
<br/>
[micro-ROS for PlatformIO](https://github.com/micro-ROS/micro_ros_platformio)  
[micro-ROS for Arduinoのサンプル](https://github.com/micro-ROS/micro_ros_arduino/tree/iron/examples)  
micro-ROS for PlatformIOにはサンプルが少ないので、こちらが参考になる。micro-ROS for PlatformIOで使うには微妙に修正が必要。  
<br/>
[PaHUB2のドキュメント](https://docs.m5stack.com/en/unit/pahub2)  
[M5ATOM Liteのドキュメント](https://docs.m5stack.com/en/core/atom_lite)  
[M5Stack用超音波測距ユニットのドキュメント](https://docs.m5stack.com/en/unit/sonic.i2c)  

