# Smart-SortingTrash  
## 开发工具：Ubuntu、MobaXterm、Vscode、FileZilla、Keil、STM32CubeMX、MaixPy     
## 开发板  ：MP157、STM32F427IIH、CanMV K210     
## 项目概述：使用视觉识别垃圾种类，通过 STM32F4通过RTOS控制舵机与电机实现垃圾倾倒，并将垃圾数据以及传感器数据发送给MP157，再由MP157使用socket编程上传数据到后端服务器并在网页可视化显示
# 整体演示效果


https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/e51e2eef-a9f9-4c59-a41f-b660b34638a8


https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/b1a2e5e5-e39e-42b0-95aa-204c8699c216



# 控制思路
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/c472ca50-cbfe-4a82-99dc-c2c7a53f6f2a)
# 电路控制
## 使用带编码器的直流无刷减速电机控制垃圾盖开合（https://item.taobao.com/item.htm?_u=c20c2kt5mcc139&id=658346700342&spm=a1z09.2.0.0.51af2e8d0zt2wc）      
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/c53d2ecb-9513-4cc6-87b6-0c42c9537596)
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/49fe4d62-1d00-42e5-99a5-19b137109937)


https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/14532bf4-9db5-4536-b94b-7363158e5b5d


## 使用CAN协议进行M2006大疆电机控制 配合PS2手柄可以做到远程控制垃圾桶前进、后退、加减速，自旋等
## PS2手柄调试记录：https://blog.csdn.net/m0_68048437/article/details/138734213?spm=1001.2014.3001.5502   
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/d1a56e0f-57ba-47a0-918e-88ab08847f2a)   

## MP157 Linux开发板实现获取传感器（GPS、陀螺仪、光照、温度）数据，用Qt显示在LCD并使用Socket联网上传数据     
https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/24c75af3-a79c-47ee-a72f-845ced1a41b4
# 视觉识别  
## 使用Maix Hub提供的训练平台(https://maixhub.com/model/zoo?feature=classification)训练出Kmodel模型,能够稳定识别6类垃圾（纸巾、易拉罐、有害、厨余、纸箱、其他）


https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/a069396e-c5cd-417b-929d-28526354f804



https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/bbafd81a-aa70-44ce-b52c-a59cef2b46e8


# 软件部署  
## 将Linux开发板上传的数据到后端进行解析，然后在前端进行可视化显示管理  
![250f0b09547eba5a7c552c91fbac9ba](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/37413b70-6f9c-4237-875f-cc12a6f485fc)
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/247a76bf-4cf7-4b34-8723-3e76bc1eb3f2)
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/08063782-8ddb-4ffe-b683-b3bf71761fc3)
# 机械设计
![image](https://github.com/Nosultion/Smart-SortingTrash/assets/116421163/3792e968-254d-4b4a-a489-03d8f48a3ef4)




