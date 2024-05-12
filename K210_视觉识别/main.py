import sensor, image, time, lcd, gc, cmath, ustruct
from maix import KPU
from machine import UART
from fpioa_manager import fm
lcd.init()                          # Init lcd display
lcd.clear(lcd.RED)                  # Clear lcd screen.

# sensor.reset(dual_buff=True)      # improve fps
sensor.reset()                      # Reset and initialize the sensor.
sensor.set_pixformat(sensor.RGB565) # Set pixel format to RGB565 (or GRAYSCALE)
sensor.set_framesize(sensor.QVGA)   # Set frame size to QVGA (320x240)
sensor.set_vflip(False)              # 翻转摄像头
sensor.set_hmirror(False)            # 镜像摄像头
sensor.skip_frames(time = 1000)     # Wait for settings take effect.
clock = time.clock()                # Create a clock object to track the FPS.
uart=None
print("ready load model")
fm.register(6,fm.fpioa.UART1_RX)
fm.register(7,fm.fpioa.UART1_TX)
def sending_data(cc):
    global uart;
    data=bytearray(cc)
 #   print(type(data))
    uart = UART(UART.UART1, 115200, 8, 1, 0, timeout=1000, read_buf_len=4096)
    uart.write(data);   #必须要传入一个字节数组


kpu = KPU()
# 从sd或flash加载模型
kpu.load_kmodel(0x300000, 0x0D6000)
#kpu.load_kmodel(0x300000, MODEL_SIZE)
tabs = {'YLG':'can','KHS':'tissue','YH':'harmful','QT':'others','CY':'kitchen','K':'null','ZX':'recoverable'}
ids = {'YLG':'2','KHS':'1','YH':'3','QT':'6','CY':'4','K':'0','ZX':'5'}
labels = ['KHS', 'QT', 'CY', 'YH', 'K', 'ZX', 'YLG'] #类名称，按照label.txt顺序填写
#label = ['K', 'ZX', 'YLG', 'KHS', 'YH', 'QT', 'CY']
label=[0]*7
id=[0]*7
print(type(tabs))
for i in range(0,7):
    label[i]=tabs[labels[i]]
    id[i]=ids[labels[i]]
print(id)
img256256 = image.Image(size=(256,256))
ti=0
T=[0]*7
M=30
while(True):
    gc.collect()

    clock.tick()                    # Update the FPS clock.
    img = sensor.snapshot()         # Take a picture and return the image.

    img256240 = img.copy((32, 0, 256, 240))
    a = img256256.draw_image(img256240,0,8)
    del img256240

    img128128 = img256256.resize(128,128)
    img128128.pix_to_ai()

    result = kpu.run_with_output(img128128, getlist=True)
    del img128128
    #print(result)

    fps = clock.fps()

    result = kpu.softmax(result)
    #print(result)

    _max_val = max(result)
    _max_idx = result.index(_max_val)

   # print(labels[_max_idx], _max_val)
    img.draw_string(32, 0, label[_max_idx] + " %.3f"%_max_val,0xFFFF,2.0)

    img.draw_rectangle(32,0,256,240)
    a = img.draw_string(0, 240 - 32, "%2.1ffps" %(fps),color=(0,60,255),scale=2.0)
    lcd.display(img)
    ti=ti+1
    T[_max_idx]=T[_max_idx]+1
    if ti<M:
        continue
    ti=0
    m=0
    m_id=0
    for i in range(0,7):
        if T[i]>m:
            m=T[i]
            m_id=i
        T[i]=0
    if m/M*100<80:
        sending_data('0')
        print('fail')
        continue
    #sending_data(m_id)
    sending_data(id[m_id])
    #sending_data(label[m_id])
    #sending_data(label[_max_idx])
    print(label[m_id],m/M*100)
