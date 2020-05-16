#ifndef SESSION_STRUCT_H
#define	SESSION_STRUCT_H

#include "util/inc/component/channel.h"

typedef struct Session_t {
	short has_reg;
	short persist;
	Channel_t* channel_client;
	Channel_t* channel_server;
	int id;
	void* userdata;
	void(*destroy)(struct Session_t*);
	unsigned int expire_timeout_msec;
	RBTimerEvent_t* expire_timeout_ev;
} Session_t;

#define	channelSession(channel)		((channel)->userdata)
#define	channelSessionId(channel)	((channel)->userid32)

#ifdef __cplusplus
extern "C" {
#endif

__declspec_dll int allocSessionId(void);
__declspec_dll Session_t* initSession(Session_t* session);

__declspec_dll void sessionChannelReplaceClient(Session_t* session, Channel_t* channel);
__declspec_dll void sessionChannelReplaceServer(Session_t* session, Channel_t* channel);
__declspec_dll void sessionUnbindChannel(Session_t* session, Channel_t* channel);
__declspec_dll Channel_t* sessionChannel(Session_t* session);

#ifdef __cplusplus
}
#endif

#endif // !SESSION_STRUCT_H
