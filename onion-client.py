#/usr/bin/python3

# Simple python script to make Onion Omega functions available via MQTT:
# - OLED
# - RGB LED
# - Push Button
#

import paho.mqtt.client as mqtt
from time import sleep
import signal
import sys

MQTT_HOST = "mqtt"
MQTT_TOPIC_BASE = "/outpostbeta/Onion/"

MQTT_TOPIC_OLED_TEXT = "OLED/text"
MQTT_TOPIC_OLED_IMAGE = "OLED/image"
MQTT_TOPIC_RGB = "rgb"
MQTT_TOPIC_PUSHBUTTON = "pb"


# OLED handling

def onion_oled_text(data):
    print("Oled Text: " + str(data))
    
    return

def onion_oled_image(data):
    print("Oled image.")
    
    return;

def onion_rgb(data):
    print("RBG: " + str(data))
    
    return;

def onion_check_pushbutton():
    return

# Sime MQTT client

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result code "+str(rc))
    
    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    #client.subscribe("$SYS/#")
    client.subscribe(MQTT_TOPIC_BASE+"#")

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    print(msg.topic+" "+str(msg.payload))
    
    topic = msg.topic
    payload = msg.payload
    
    if mqtt.topic_matches_sub(MQTT_TOPIC_BASE + MQTT_TOPIC_OLED_TEXT, topic):
        onion_oled_text(payload)
    
    if mqtt.topic_matches_sub(MQTT_TOPIC_BASE + MQTT_TOPIC_OLED_IMAGE, topic):
        onion_oled_image(payload)
    
    if mqtt.topic_matches_sub(MQTT_TOPIC_BASE + MQTT_TOPIC_RGB, topic):
        onion_rgb(payload)


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

client.connect(MQTT_HOST, 1883, 60)


# The Loop
global running
running = True

def sigint_handler(signal, frame):
    global running
    
    if running:
        print("SIGINT received. Stopping the queue.")
        running = False
    else:
        print("Receiving SIGINT the second time. Exit.");
        sys.exit(0)

signal.signal(signal.SIGINT, sigint_handler)

client.loop_start()

while running:
    onion_check_pushbutton()
    
    sleep(0.2)
# loop is quit by the SIGINT signal handler

client.loop_stop()



# kate: space-indent on; indent-width 4; mixedindent off; indent-mode python; indend-pasted-text false; remove-trailing-space off
