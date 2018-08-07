from numpy import random, sin
import paho.mqtt.publish as publish
import time

print("Sending 0...")
group = 'wyostat'

ac_state = False
temp = 70
t = 0
while 1:
    if temp < 50:
        temp += random.randint(0, 2)
    elif temp < 85:
        temp += random.randint(-1, 2)
    else:
        temp += random.randint(-1, 1)
    temp = 70 + 10 * sin(t/10.)
    t += 1
    print(temp)
    publish.single(group + '.temp', str(int(temp)), hostname="localhost")
    time.sleep(5)
    if temp > 75 and ac_state == False:
        ac_state = True
        publish.single(group + '.ac_state', str(ac_state).lower(), hostname="localhost")
    elif temp < 73 and ac_state == True:
        ac_state = False
        publish.single(group + '.ac_state', str(ac_state).lower(), hostname="localhost")
            
            

# time.sleep(1)
# publish.multiple([{'topic':topic, 'payload':'payload1'}, {'topic':topic, 'payload':'payload2'}])
