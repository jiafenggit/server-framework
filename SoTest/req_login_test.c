#include "../BootServer/global.h"
#include "cmd.h"
#include "test_handler.h"
#include <stdio.h>

void reqLoginTest(TaskThread_t* thrd, UserMsg_t* ctrl) {
	cJSON *cjson_ret_root;
	InnerMsg_t ret_msg;

	logInfo(ptr_g_Log(), "%s recv: %s", __FUNCTION__, (char*)ctrl->data);

	channelSessionId(ctrl->channel) = allocSessionId();

	cjson_ret_root = cJSON_NewObject(NULL);
	cJSON_AddNewNumber(cjson_ret_root, "session_id", channelSessionId(ctrl->channel));
	cJSON_Print(cjson_ret_root);

	makeInnerMsg(&ret_msg, CMD_RET_LOGIN_TEST, cjson_ret_root->txt, cjson_ret_root->txtlen);
	channelSendv(ctrl->channel, ret_msg.iov, sizeof(ret_msg.iov) / sizeof(ret_msg.iov[0]), NETPACKET_FRAGMENT);
	cJSON_Delete(cjson_ret_root);
}

void retLoginTest(TaskThread_t* thrd, UserMsg_t* ctrl) {
	cJSON* cjson_ret_root;

	logInfo(ptr_g_Log(), "recv: %s", (char*)ctrl->data);

	cjson_ret_root = cJSON_Parse(NULL, (char*)ctrl->data);
	if (!cjson_ret_root) {
		logErr(ptr_g_Log(), "cJSON_Parse error");
		return;
	}

	do {
		cJSON* cjson_sessoin_id;

		cjson_sessoin_id = cJSON_Field(cjson_ret_root, "session_id");
		if (!cjson_sessoin_id) {
			logErr(ptr_g_Log(), "miss session id field");
			break;
		}
		channelSessionId(ctrl->channel) = cjson_sessoin_id->valueint;
	} while (0);
	cJSON_Delete(cjson_ret_root);

	// test code
	if (thrd->f_rpc) {
		newFiberSleepMillsecond(thrd, 5000);
		frpc_test_code(thrd, ctrl->channel);
	}
	else if (thrd->a_rpc)
		arpc_test_code(thrd, ctrl->channel);
}
