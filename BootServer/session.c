#include "global.h"
#include "session.h"

Hashtable_t g_SessionTable;
static HashtableNode_t* s_SessionBulk[1024];
static Atom32_t CHANNEL_SESSION_ID = 0;
static int __keycmp(const void* node_key, const void* key) { return node_key != key; }
static unsigned int __keyhash(const void* key) { return (ptrlen_t)key; }

static Session_t* defaultNewSession(int usertype) {
	Session_t* session = (Session_t*)malloc(sizeof(Session_t));
	if (session) {
		initSession(session);
		session->usertype = usertype;
	}
	return session;
}
static void unregSession(Session_t* session) {
	if (session->has_reg) {
		session->has_reg = 0;
		hashtableRemoveNode(&g_SessionTable, &session->m_htnode);
	}
}
static void defaultFreeSession(Session_t* s) { free(s); }
SessionActon_t g_SessionAction = { defaultNewSession, unregSession, defaultFreeSession };

#ifdef __cplusplus
extern "C" {
#endif

int initSessionTable(void) {
	hashtableInit(&g_SessionTable, s_SessionBulk, sizeof(s_SessionBulk) / sizeof(s_SessionBulk[0]), __keycmp, __keyhash);
	return 1;
}

int allocSessionId(void) {
	int session_id;
	do {
		session_id = _xadd32(&CHANNEL_SESSION_ID, 1) + 1;
	} while (0 == session_id);
	return session_id;
}

Session_t* initSession(Session_t* session) {
	session->has_reg = 0;
	session->persist = 0;
	session->id = 0;
	session->usertype = 0;
	session->userdata = NULL;
	listInit(&session->rpc_itemlist);
	session->expire_timeout_msec = 0;
	session->expire_timeout_ev = NULL;
	return session;
}

void freeSessionTable(void) {
	HashtableNode_t* htcur, *htnext;
	for (htcur = hashtableFirstNode(&g_SessionTable); htcur; htcur = htnext) {
		Session_t* session = pod_container_of(htcur, Session_t, m_htnode);
		htnext = hashtableNextNode(htcur);
		g_SessionAction.destroy(session);
	}
	hashtableInit(&g_SessionTable, s_SessionBulk, sizeof(s_SessionBulk) / sizeof(s_SessionBulk[0]), __keycmp, __keyhash);
}

void sessionBindChannel(Session_t* session, Channel_t* channel) {
	session->channel = channel;
	channelSession(channel) = session;
	channelSessionId(channel) = session->id;
}

Channel_t* sessionUnbindChannel(Session_t* session) {
	if (session) {
		Channel_t* channel = session->channel;
		if (channel) {
			channelSession(channel) = NULL;
			channelSessionId(channel) = 0;
		}
		session->channel = NULL;
		return channel;
	}
	return NULL;
}

#ifdef __cplusplus
}
#endif