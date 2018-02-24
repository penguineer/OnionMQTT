#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <syslog.h>

#include <mosquitto.h>

const char* MQTT_HOST 	= "localhost";
const int   MQTT_PORT 	= 1883;
const char* MQTT_TOPIC_OLED_TEXT 	= "OnionOmega/OLED/text";
const char* MQTT_TOPIC_OLED_IMG 	= "OnionOmega/OLED/img";
const char* MQTT_TOPIC_RGB 	= "OnionOmega/RGB";

const char* MQTT_TOPIC_BTN = "OnionOmega/PB";

#define MQTT_MSG_MAXLEN		  16
const char* MQTT_MSG_TEST = "test";

onion_oled_display_text(const char* text)
{
}

onion_oled_display_image(const void* img)
{
}

onion_rgb_display(const char* rgb)
{
}

void mqtt_send(struct mosquitto* mosq,
			   const char* mqtt_payload,
			   const char* mqtt_topic)
{
	// send MQTT message if there is payload
	if (mqtt_payload[0] && mosq) {
		int ret;
		int mid;
		ret = mosquitto_publish(
			mosq,
			&mid,
			mqtt_topic,
			strlen(mqtt_payload), mqtt_payload,
								2, /* qos */
						  true /* retain */
		);
		if (ret != MOSQ_ERR_SUCCESS)
			syslog(LOG_ERR, "MQTT error on message \"%s\": %d (%s)",
				   mqtt_payload,
		  ret,
		  mosquitto_strerror(ret));
		else
			syslog(LOG_INFO, "MQTT message \"%s\" sent with id %d.",
				   mqtt_payload, mid);
	}
}

int mqtt_subscribe(struct mosquitto* mosq,
				   const char* topic)
{
	if (mosq && topic) {
		int ret;
		
		ret = mosquitto_subscribe(mosq, NULL, topic, 0);
		if (ret == MOSQ_ERR_SUCCESS)
			syslog(LOG_INFO, 
				   "MQTT subscribtion to %s successfull.", 
				   topic);
		else {	
			// TODO error handling
		}
		
		return 1;
	}
	
	return 0;
}

void mqtt_message_callback(struct mosquitto *mosq,
						  void *obj,
						  const struct mosquitto_message *msg)
{
	int ret;
	bool match;
	
	/*
	 * Check OLED text topic
	 */
	ret = mosquitto_topic_matches_sub(MQTT_TOPIC_OLED_TEXT,
									  msg->topic,
								   &match);
	if (ret != MOSQ_ERR_SUCCESS) {
		// TODO error handling
		return;
	}
	if (match) {
		// handling for oled text messages
		onion_oled_display_text((char*)msg->payload);
		
		return;
	}

	/*
	 * Check OLED image topic
	 */
	ret = mosquitto_topic_matches_sub(MQTT_TOPIC_OLED_IMG,
									  msg->topic,
								   &match);
	if (ret != MOSQ_ERR_SUCCESS) {
		// TODO error handling
		return;
	}
	if (match) {
		// handling for oled image messages
		onion_oled_display_image(msg->payload);
		
		return;
	}
	
	/*
	 * Check RGB topic
	 */
	ret = mosquitto_topic_matches_sub(MQTT_TOPIC_RGB,
									  msg->topic,
								   &match);
	if (ret != MOSQ_ERR_SUCCESS) {
		// TODO error handling
		return;
	}
	if (match) {
		// handling for rgb messages
		onion_rgb_display((char*)msg->payload);

		return;
	}
	
	syslog(LOG_INFO, "MQTT message received on unexpected topic %s.", msg->topic);
}

int main(int argc, char *argv[]) {
	// initialize the system logging
	openlog("onionmqtt", LOG_CONS | LOG_PID, LOG_USER);
	syslog(LOG_INFO, "Starting Onion MQTT.");
	
	// initialize MQTT
	mosquitto_lib_init();
	
	void *mqtt_obj;
	struct mosquitto *mosq;
	mosq = mosquitto_new("onionmqtt", true, mqtt_obj);
	if ((int)mosq == ENOMEM) {
		syslog(LOG_ERR, "Not enough memory to create a new mosquitto session.");
		mosq = NULL;
	}
	if ((int)mosq == EINVAL) {
		syslog(LOG_ERR, "Invalid values for creating mosquitto session.");
		return -1;
	}
	
	if (mosq) {
		int ret;
		
		ret = mosquitto_connect(mosq, MQTT_HOST, MQTT_PORT, 30);
		if (ret == MOSQ_ERR_SUCCESS)
			syslog(LOG_INFO, "MQTT connection to %s established.", MQTT_HOST);
		
		// TODO error handling
	}
	
	// subscribe to Onion topics
	mqtt_subscribe(mosq, MQTT_TOPIC_OLED_TEXT);
	mqtt_subscribe(mosq, MQTT_TOPIC_OLED_IMG);
	mqtt_subscribe(mosq, MQTT_TOPIC_RGB);
	
	// set the message callback
	mosquitto_message_callback_set(mosq, mqtt_message_callback);
	
	char mqtt_payload[MQTT_MSG_MAXLEN];
	
	
	char run=1;
	while(run) {
		mqtt_payload[0] = 0;
		//strcpy(mqtt_payload, MQTT_MSG_TEST);
		
		// send MQTT messages if there is payload
		mqtt_send(mosq, mqtt_payload, MQTT_TOPIC_BTN);
		
		
		// call the mosquitto loop to process messages
		if (mosq) {
			int ret;
			ret = mosquitto_loop(mosq, 100, 1);
			// if failed, try to reconnect
			if (ret)
				mosquitto_reconnect(mosq);
		}
		
		if (sleep(1)) 
			break;
	}
	
	// clean-up MQTT
	if (mosq) {
		mosquitto_disconnect(mosq);
		mosquitto_destroy(mosq);
	}
	mosquitto_lib_cleanup();
	
	syslog(LOG_INFO, "Onion MQTT finished.");
	closelog();
	
	return 0;
}
