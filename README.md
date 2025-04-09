# 温度传感器

此项目实现通过客户端采集树莓派上连接的ds18b20温度传感器的温度信息，通过socket网络通信发送到服务器端并永久存入sqlite3数据库中，相关实现原理如下：

### 客户端(client)

客户端以socket_client.c作为主函数，通过调用clientinput.c实现命令行参数修改所需连接的服务器端IP(域名)、端口、定时采样时间，随后调用client_socket.c实现socket连接以及自动重连，建立温度读取子线程调用ds18b20.c实时读取当前温度信息，并在数据发送前判断当前是否与服务器端正常连接，如果正常连接则将数据发送到服务器端，如果连接断开则建立临时数据库将数据存入数据库中，并通过全局变量g_sock_time将断开信息返还给主线程。主线程通过循环不停检测g_sock_time结果，一旦发现断开连接立即执行自动重连函数，在自动重连函数中以每2的幂数秒进行一次自动重连，自动重连最大等待时间为16秒，一旦自动重连成功将立即把数据库中的信息发送到服务器端并删除数据库，同时更改g_sock_time的值为正常连接状态，随后结束函数，继续在主线程中判断g_sock_time的值。

客户端主要功能实现原理框图如下：

![温度传感器客户端原理框图](https://github.com/user-attachments/assets/5c4e00b5-0dc8-4bb9-bbf2-8a470ea08267)

![客户端子线程原理框图](https://github.com/user-attachments/assets/70ba488b-e99a-47f1-b923-a2f46b632aca)


### 服务器端(server)

服务器端以socket_server.c作为主函数，通过调用serverinput.c实现命令行修改提供连接的IP（不输入默认为主机的所有IP）和端口，随后调用serverinit.c实现socket连接，使用select多路复用技术实现多客户端并发上报，并可以将客户端接收到的数据存储到数据库中。

服务器端主要功能实现原理框图如下：

![服务器端原理框图](https://github.com/user-attachments/assets/60598b62-7ca6-4011-b704-798ed9315660)


### 使用说明：
##### 1.客户端与服务器端Makefile文件基本功能相同，make 默认以本系统动态库形式编译代码，make static 是以本系统静态库形式编译代码，make arm-linux-gnueabihf-gcc 是以ARM32位Linux系统交叉编译动态库形式代码， make arm-linux-gnueabihf-gcc-static 是以ARM32位Linux系统交叉编译静态库形式代码
##### 2.make 编译后将可执行文件放入bin目录下，运行需提前将`pwd`/../libs路径加入LD_LIBRARY_PATH下。
##### 3.客户端和服务器端默认日志文件产生于可执行文件所在目录(bin目录)下，默认命名为client.txt(服务器端同理)，如需更改需更改主目录下socket_client.c文件(服务器端同理)
##### 4.客户端和服务器端默认数据库产生于可执行文件所在目录(bin目录)下，默认命名为client.db(服务器端同理)，表名均为 temp ,如需更改数据库名需更改主目录下socket_client.c文件(服务器端同理)


