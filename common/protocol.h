/**
 * protocol.h - Defines a message protocol to be used for pdx irc
 * Author: Brett Creeley
 *
 * Note: Any new message type structures added muse have have the first member
 * 	 defined like the following struct new_type_msg:
 *
 * 	 struct new_type_msg {
 * 	 	uint8_t type;
 * 	 	...
 * 	 };
 *
 * 	 To make use of this when the server/client are recv()'ing a message
 * 	 they will fisrt make a call to recv(sockfd, buffer, 1, flags).  This
 * 	 will determine the type of message.  Based on this the size of the
 * 	 message can be determine by using sizeof(struct new_type_msg) for the
 * 	 following call(s) to recv() the data.
 */
#ifndef _PROTOCOL_H
#define _PROTOCOL_H
#include <stdint.h>
#include <assert.h>

#define CONNECTION_CLOSED 0
#define SUCCESS 0
#define FAIL 1

#define BIT(i) (1 << i)

#define USER_NAME_MAX_LEN	16
#define PW_MAX_LEN		16
#define CHANNEL_NAME_MAX_LEN	32
#define CHAT_MSG_MAX_LEN	256

enum message_type {
	MSG_TYPE_INVALID = 0,
	ERROR		 = 1,
	LOGIN		 = 2,
	JOIN		 = 3,
	LEAVE		 = 4,
	CHAT		 = 5,
	LIST_CHANNELS	 = 6,	/* channel names are separated by ":" */
	LIST_USERS	 = 7,	/* user names are separated by ":" */

	/* Do not put any new message types after MAX_MSG_NUM */
	MAX_MSG_NUM	 = 255
};


/* Make sure there is no padding in message structures */
#pragma pack(push, 1)

struct message {
	uint8_t type;

#define RESP_INVALID			0
#define RESP_SUCCESS			BIT(0)
#define RESP_INVALID_LOGIN		BIT(1)
#define RESP_INVALID_CHANNEL_NAME 	BIT(2)
#define RESP_NOT_IN_CHANNEL		BIT(3)
#define RESP_ALREADY_IN_CHANNEL		BIT(4)
#define RESP_SERVER_HAS_NO_CHANNELS 	BIT(5)
#define RESP_CANNOT_GET_USERS		BIT(6)
#define RESP_RECV_MSG_FAILED		BIT(7)
#define RESP_MEMORY_ALLOC		BIT(8)
#define RESP_CANNOT_ADD_CHANNEL		BIT(9)
#define RESP_CANNOT_ADD_USER_TO_CHANNEL	BIT(10)
#define RESP_STILL_CHANNELS_REMAINING	BIT(11)
#define RESP_DONE_SENDING_CHANNELS	BIT(12)
/* BIT(31) is the largest define with resposne being a 32-bit value */
	uint32_t response;
	uint8_t list_key;	/* Only used for list_channels */
	union {
		struct {
			char username[USER_NAME_MAX_LEN];
			char password[PW_MAX_LEN];
		} login;
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} join;
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} leave;
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
			char text[CHAT_MSG_MAX_LEN];
		} chat;
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} list_channels;
	};
};
#define MSG_SIZE (sizeof(struct message))

/* Remove structure packing format */
#pragma pack(pop)

#endif /* _PROTOCOL_H */
