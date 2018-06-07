/**
 * debug.c - Contains various debugging print/helper functions
 * Author: Brett Creeley
 */

#include "debug.h"
#include "../protocol.h"

#define RESP_INVALID_STR 			"RESP_INVALID"
#define RESP_SUCCESS_STR 			"RESP_SUCCESS"
#define RESP_INVALID_LOGIN_STR 			"RESP_INVALID_LOGIN"
#define RESP_INVALID_CHANNEL_NAME_STR 		"RESP_INVALID_CHANNEL_NAME"
#define RESP_NOT_IN_CHANNEL_STR 		"RESP_NOT_IN_CHANNEL"
#define RESP_ALREADY_IN_CHANNEL_STR		"RESP_ALREADY_IN_CHANNEL"
#define RESP_SERVER_HAS_NO_CHANNELS_STR		"RESP_SERVER_HAS_NO_CHANNELS"
#define RESP_CANNOT_GET_USERS_STR		"RESP_CANNOT_GET_USERS_STR"
#define RESP_RECV_MSG_FAILED_STR		"RESP_RECV_MSG_FAILED"
#define RESP_MEMORY_ALLOC_STR			"RESP_MEMORY_ALLOC"
#define RESP_CANNOT_ADD_CHANNEL_STR		"RESP_CANNOT_ADD_CHANNEL"
#define RESP_CANNOT_ADD_USER_TO_CHANNEL_STR	"RESP_CANNOT_ADD_USER_TO_CHANNEL"

const char *resp_type_to_str(uint32_t resp)
{
	switch (resp) {
	case RESP_INVALID:
		return RESP_INVALID_STR;
	case RESP_SUCCESS:
		return RESP_SUCCESS_STR;
	case RESP_INVALID_LOGIN:
		return RESP_INVALID_LOGIN_STR;
	case RESP_INVALID_CHANNEL_NAME:
		return RESP_INVALID_CHANNEL_NAME_STR;
	case RESP_NOT_IN_CHANNEL:
		return RESP_NOT_IN_CHANNEL_STR;
	case RESP_ALREADY_IN_CHANNEL:
		return RESP_ALREADY_IN_CHANNEL_STR;
	case RESP_SERVER_HAS_NO_CHANNELS:
		return RESP_SERVER_HAS_NO_CHANNELS_STR;
	case RESP_CANNOT_GET_USERS:
		return RESP_CANNOT_GET_USERS_STR;
	case RESP_RECV_MSG_FAILED:
		return RESP_RECV_MSG_FAILED_STR;
	case RESP_MEMORY_ALLOC:
		return RESP_MEMORY_ALLOC_STR;
	case RESP_CANNOT_ADD_CHANNEL:
		return RESP_CANNOT_ADD_CHANNEL_STR;
	case RESP_CANNOT_ADD_USER_TO_CHANNEL:
		return RESP_CANNOT_ADD_USER_TO_CHANNEL_STR;
	default:
		return "Invalid Response Message";
	}
}

#define MSG_TYPE_INVALID_STR		"MSG_TYPE_INVALID"
#define MSG_TYPE_ERROR_STR		"MSG_TYPE_ERROR"
#define MSG_TYPE_LOGIN_STR		"MSG_TYPE_LOGIN"
#define MSG_TYPE_JOIN_STR		"MSG_TYPE_JOIN"
#define MSG_TYPE_LEAVE_STR		"MSG_TYPE_LEAVE"
#define MSG_TYPE_CHAT_STR		"MSG_TYPE_CHAT"
#define MSG_TYPE_LIST_CHANNELS_STR	"MSG_TYPE_LIST_CHANNELS"
#define MSG_TYPE_LIST_USERS_STR		"MSG_TYPE_LIST_USERS"

const char *msg_type_to_str(uint8_t type)
{ switch (type) {
	case MSG_TYPE_INVALID:
		return MSG_TYPE_INVALID_STR;
	case ERROR:
		return MSG_TYPE_ERROR_STR;
	case LOGIN:
		return MSG_TYPE_LOGIN_STR;
	case JOIN:
		return MSG_TYPE_JOIN_STR;
	case LEAVE:
		return MSG_TYPE_LEAVE_STR;
	case CHAT:
		return MSG_TYPE_CHAT_STR;
	case LIST_CHANNELS:
		return MSG_TYPE_LIST_CHANNELS_STR;
	case LIST_USERS:
		return MSG_TYPE_LIST_USERS_STR;
	default:
		return "Invalid Message Type";
	}
}