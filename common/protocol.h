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

#define USER_NAME_MAX_LEN 16
#define CHANNEL_NAME_MAX_LEN 32
#define CHAT_MSG_MAX_LEN 256

enum message_type {
	MSG_TYPE_INVALID = 0,
	JOIN		 = 1,
	LEAVE		 = 2,
	CHAT		 = 3,
	LIST_CHANNELS	 = 4,
	LIST_USERS	 = 5,	/* Per channel list */

	/* Do not put any new message types after MAX_MSG_NUM */
	MAX_MSG_NUM	 = 255
};

/* Make sure there is no padding in message structures */
#pragma pack(push, 1)

#if 0 /* new format -- more complicated but better design probably a bad choice */

struct message {
	uint8_t type;
	uint32_t length;

#define RESP_INVALID			0
#define RESP_SUCCESS			BIT(1)
#define RESP_FAIL			BIT(2)
#define RESP_USERNAME_TAKEN		BIT(3)
#define RESP_INVALID_USERNAME		BIT(4)
#define RESP_INVALID_CHANNEL_NAME	BIT(5)
/* Don't add any defines greater than BIT(31) */
#define MAX_MSG_RESP_NUM		BIT(31)
	uint32_t resp_code;

	void *payload;
}

struct channel_join {
	char src_user[USER_NAME_MAX_LEN];
	char channel_name[CHANNEL_NAME_MAX_LEN];
}

struct channel_leave {
	char src_user[USER_NAME_MAX_LEN];
	char channel_name[CHANNEL_NAME_MAX_LEN];
}

struct channel_chat {
	char src_user[USER_NAME_MAX_LEN];
	char channel_name[CHANNEL_NAME_MAX_LEN];
	char text[CHAT_MSG_MAX_LEN];
}

struct channel_list {
	char channels[MAX_CHANNEL_LIST_LEN];
}

struct channel_user_list {
	char users[MAX_CHANNEL_USER_LIST_LEN];
};


#endif

struct message {
	uint8_t type;
	union {
		/* When user sends join request to server */
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} join;
		struct {
			uint8_t result;
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} join_response;
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} leave;
		struct {
			uint8_t result;
			char channel_name[CHANNEL_NAME_MAX_LEN];
		} leave_response;
		/* Client sends message to channel on server */
		struct {
			char src_user[USER_NAME_MAX_LEN];
			char channel_name[CHANNEL_NAME_MAX_LEN];
			char text[CHAT_MSG_MAX_LEN];
		} chat;
	};
};
#define MSG_SIZE (sizeof(struct message))

/* Remove structure packing format */
#pragma pack(pop)

#endif /* _PROTOCOL_H */
