/**
 * debug.c - Contains various debugging print/helper functions
 * Author: Brett Creeley
 */

#include "debug.h"
#include "../protocol.h"
#include <stdlib.h>

struct resp_type_strings {
	uint32_t type;
	const char *string;
};

struct resp_type_strings resp_type_str_arr[] = {
	{RESP_INVALID,				"RESP_INVALID"},
	{RESP_SUCCESS,				"RESP_SUCCESS"},
	{RESP_INVALID_LOGIN,			"RESP_INVALID_LOGIN"},
	{RESP_INVALID_CHANNEL_NAME,		"RESP_INVALID_CHANNEL_NAME"},
	{RESP_NOT_IN_CHANNEL,			"RESP_NOT_IN_CHANNEL"},
	{RESP_ALREADY_IN_CHANNEL,		"RESP_ALREADY_IN_CHANNEL"},
	{RESP_SERVER_HAS_NO_CHANNELS,		"RESP_SERVER_HAS_NO_CHANNELS"},
	{RESP_CANNOT_GET_USERS,			"RESP_CANNOT_GET_USERS"},
	{RESP_RECV_MSG_FAILED,			"RESP_RECV_MSG_FAILED"},
	{RESP_MEMORY_ALLOC,			"RESP_MEMORY_ALLOC"},
	{RESP_CANNOT_ADD_CHANNEL,		"RESP_CANNOT_ADD_CHANNEL"},
	{RESP_CANNOT_ADD_USER_TO_CHANNEL,	"RESP_CANNOT_ADD_USER_TO_CHANNEL"},
	{RESP_DONE_SENDING_CHANNELS,		"RESP_DONE_SENDING_CHANNELS"},
	{RESP_LIST_CHANNELS_IN_PROGRESS,	"RESP_LIST_CHANNELS_IN_PROGRESS"},
	{RESP_CANNOT_FIND_CHANNEL,		"RESP_CANNOT_FIND_CHANNEL"},
	{RESP_CANNOT_LIST_CHANNELS,		"RESP_CANNOT_LIST_CHANNELS"},
	/* Last entry requires NULL string for looping purposes */
	{0 , NULL},
};

const char *resp_type_to_str(uint32_t resp_type)
{
	int i = 0;

	while (resp_type_str_arr[i].string != NULL) {
		if (resp_type == resp_type_str_arr[i].type)
			return resp_type_str_arr[i].string;
		++i;
	}

	return "RESP_UNKNOWN";
}

struct msg_type_strings {
	uint8_t type;
	const char *string;
};

static struct msg_type_strings msg_type_str_arr[] = {
	{MSG_TYPE_INVALID,	"MSG_TYPE_INVALID"},
	{ERROR,			"MSG_TYPE_ERROR"},
	{LOGIN,			"MSG_TYPE_LOGIN"},
	{JOIN,			"MSG_TYPE_JOIN"},
	{LEAVE,			"MSG_TYPE_LEAVE"},
	{CHAT,			"MSG_TYPE_CHAT"},
	{LIST_CHANNELS,		"MSG_TYPE_LIST_CHANNELS"},
	{LIST_USERS,		"MSG_TYPE_LIST_USERS"},
	/* Last entry requires NULL string for looping purposes */
	{0 , NULL},
};

const char *msg_type_to_str(uint8_t msg_type)
{
	int i = 0;

	while (msg_type_str_arr[i].string != NULL) {
		if (msg_type == msg_type_str_arr[i].type)
			return msg_type_str_arr[i].string;
		++i;
	}

	return "MSG_TYPE_UNKNOWN";
}

