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

#define USER_NAME_MAX_LEN 16
#define CHANNEL_NAME_MAX_LEN 32
#define CHAT_MSG_MAX_LEN 256

enum message_type {
	MSG_TYPE_INVALID = 0,
	/* Client send message types */
	JOIN = 1,
	LEAVE = 2,
	CHAT = 3,
	LIST_CHANNELS = 4,
	/* Server send message types */
	/* Do not put any new message types after MAX_MSG_TYPES */
	MAX_MSG_TYPES = 256
};

/* Make sure there is no padding in message structures */
#pragma pack(push, 1)


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
