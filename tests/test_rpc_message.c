#include "config.h"
#include "gck-rpc-private.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("  " #name "..."); name(); printf(" OK\n"); } while(0)

/* Provide the log function that gck-rpc-util.c calls */
void gck_rpc_log(const char *msg, ...)
{
	(void)msg;
}

/* --- Message lifecycle --- */

TEST(test_message_new_free)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	assert(msg != NULL);
	assert(!egg_buffer_has_error(&msg->buffer));
	gck_rpc_message_free(msg);
}

TEST(test_message_reset)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	assert(msg != NULL);

	gck_rpc_message_prep(msg, GCK_RPC_CALL_C_Initialize, GCK_RPC_REQUEST);
	assert(msg->call_id == GCK_RPC_CALL_C_Initialize);

	gck_rpc_message_reset(msg);
	assert(msg->call_id == 0);
	assert(msg->signature == NULL);

	gck_rpc_message_free(msg);
}

/* --- Prep and parse roundtrip --- */

TEST(test_prep_request)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	assert(gck_rpc_message_prep(msg, GCK_RPC_CALL_C_GetInfo, GCK_RPC_REQUEST));
	assert(msg->call_id == GCK_RPC_CALL_C_GetInfo);
	assert(msg->call_type == GCK_RPC_REQUEST);
	assert(msg->signature != NULL);
	assert(strcmp(msg->signature, "") == 0); /* C_GetInfo request has empty sig */

	gck_rpc_message_free(msg);
}

TEST(test_prep_response)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	assert(gck_rpc_message_prep(msg, GCK_RPC_CALL_C_GetInfo, GCK_RPC_RESPONSE));
	assert(msg->call_id == GCK_RPC_CALL_C_GetInfo);
	assert(msg->call_type == GCK_RPC_RESPONSE);
	/* C_GetInfo response signature is "vsusv" */
	assert(strcmp(msg->signature, "vsusv") == 0);

	gck_rpc_message_free(msg);
}

TEST(test_prep_and_parse_roundtrip)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	/* Prep a request */
	assert(gck_rpc_message_prep(msg, GCK_RPC_CALL_C_Finalize, GCK_RPC_REQUEST));

	/* Parse the same buffer as a request */
	msg->parsed = 0;
	assert(gck_rpc_message_parse(msg, GCK_RPC_REQUEST));
	assert(msg->call_id == GCK_RPC_CALL_C_Finalize);

	gck_rpc_message_free(msg);
}

/* --- Byte read/write --- */

TEST(test_write_read_byte)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	CK_BYTE val;

	assert(gck_rpc_message_prep(msg, GCK_RPC_CALL_C_Initialize, GCK_RPC_REQUEST));

	/* C_Initialize request signature is "ay" - we need to write byte array first, then byte */
	/* Let's use a call with simpler sig. Use ERROR with no signature checking */
	gck_rpc_message_reset(msg);
	msg->signature = NULL;
	msg->sigverify = NULL;

	/* Without signature, write/read work freely */
	assert(gck_rpc_message_write_byte(msg, 0x42));
	msg->parsed = 0;
	assert(gck_rpc_message_read_byte(msg, &val));
	assert(val == 0x42);

	gck_rpc_message_free(msg);
}

/* --- Ulong read/write --- */

TEST(test_write_read_ulong)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	CK_ULONG val;

	gck_rpc_message_reset(msg);
	msg->signature = NULL;
	msg->sigverify = NULL;

	assert(gck_rpc_message_write_ulong(msg, 12345678));
	msg->parsed = 0;
	assert(gck_rpc_message_read_ulong(msg, &val));
	assert(val == 12345678);

	gck_rpc_message_free(msg);
}

TEST(test_write_read_ulong_max)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	CK_ULONG val;

	gck_rpc_message_reset(msg);
	msg->signature = NULL;
	msg->sigverify = NULL;

	assert(gck_rpc_message_write_ulong(msg, (CK_ULONG)-1));
	msg->parsed = 0;
	assert(gck_rpc_message_read_ulong(msg, &val));
	assert(val == (CK_ULONG)-1);

	gck_rpc_message_free(msg);
}

/* --- Version read/write --- */

TEST(test_write_read_version)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	CK_VERSION ver_in = { 2, 40 };
	CK_VERSION ver_out = { 0, 0 };

	gck_rpc_message_reset(msg);
	msg->signature = NULL;
	msg->sigverify = NULL;

	assert(gck_rpc_message_write_version(msg, &ver_in));
	msg->parsed = 0;
	assert(gck_rpc_message_read_version(msg, &ver_out));
	assert(ver_out.major == 2);
	assert(ver_out.minor == 40);

	gck_rpc_message_free(msg);
}

/* --- Space string read/write --- */

TEST(test_write_read_space_string)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);
	CK_UTF8CHAR label_in[32];
	CK_UTF8CHAR label_out[32];

	memset(label_in, ' ', sizeof(label_in));
	memcpy(label_in, "Test Label", 10);
	memset(label_out, 0, sizeof(label_out));

	gck_rpc_message_reset(msg);
	msg->signature = NULL;
	msg->sigverify = NULL;

	assert(gck_rpc_message_write_space_string(msg, label_in, sizeof(label_in)));
	msg->parsed = 0;
	assert(gck_rpc_message_read_space_string(msg, label_out, sizeof(label_out)));
	assert(memcmp(label_in, label_out, sizeof(label_in)) == 0);

	gck_rpc_message_free(msg);
}

/* --- Message equals --- */

TEST(test_message_equals)
{
	GckRpcMessage *m1 = gck_rpc_message_new((EggBufferAllocator)realloc);
	GckRpcMessage *m2 = gck_rpc_message_new((EggBufferAllocator)realloc);

	gck_rpc_message_prep(m1, GCK_RPC_CALL_C_Finalize, GCK_RPC_REQUEST);
	gck_rpc_message_prep(m2, GCK_RPC_CALL_C_Finalize, GCK_RPC_REQUEST);
	assert(gck_rpc_message_equals(m1, m2));

	/* Different call IDs should not be equal */
	gck_rpc_message_prep(m2, GCK_RPC_CALL_C_GetInfo, GCK_RPC_REQUEST);
	assert(!gck_rpc_message_equals(m1, m2));

	gck_rpc_message_free(m1);
	gck_rpc_message_free(m2);
}

/* --- Verify part --- */

TEST(test_verify_part)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	gck_rpc_message_prep(msg, GCK_RPC_CALL_C_GetInfo, GCK_RPC_RESPONSE);
	/* Response signature is "vsusv" */

	assert(gck_rpc_message_verify_part(msg, "v"));
	assert(gck_rpc_message_verify_part(msg, "s"));
	assert(gck_rpc_message_verify_part(msg, "u"));
	assert(gck_rpc_message_verify_part(msg, "s"));
	assert(gck_rpc_message_verify_part(msg, "v"));
	assert(gck_rpc_message_is_verified(msg));

	gck_rpc_message_free(msg);
}

/* --- Call table consistency --- */

TEST(test_call_table_consistency)
{
	int i;
	for (i = 0; i < GCK_RPC_CALL_MAX; ++i) {
		assert(gck_rpc_calls[i].call_id == i);
		if (i > 0) {
			assert(gck_rpc_calls[i].name != NULL);
			assert(gck_rpc_calls[i].request != NULL);
			assert(gck_rpc_calls[i].response != NULL);
		}
	}
}

/* --- Mechanism utility functions --- */

TEST(test_mechanism_no_parameters)
{
	assert(gck_rpc_mechanism_has_no_parameters(CKM_RSA_PKCS));
	assert(gck_rpc_mechanism_has_no_parameters(CKM_SHA_1));
	assert(gck_rpc_mechanism_has_no_parameters(CKM_AES_KEY_GEN));
	assert(!gck_rpc_mechanism_has_no_parameters(CKM_RSA_PKCS_OAEP));
}

TEST(test_mechanism_sane_parameters)
{
	assert(gck_rpc_mechanism_has_sane_parameters(CKM_RSA_PKCS_OAEP));
	assert(gck_rpc_mechanism_has_sane_parameters(CKM_RSA_PKCS_PSS));
	assert(!gck_rpc_mechanism_has_sane_parameters(CKM_RSA_PKCS));
}

TEST(test_mechanism_is_supported)
{
	assert(gck_rpc_mechanism_is_supported(CKM_RSA_PKCS));
	assert(gck_rpc_mechanism_is_supported(CKM_RSA_PKCS_OAEP));
	assert(gck_rpc_mechanism_is_supported(CKM_SHA256));
}

/* --- Ulong parameter detection --- */

TEST(test_has_ulong_parameter)
{
	assert(gck_rpc_has_ulong_parameter(CKA_CLASS));
	assert(gck_rpc_has_ulong_parameter(CKA_KEY_TYPE));
	assert(!gck_rpc_has_ulong_parameter(CKA_VALUE));
	assert(!gck_rpc_has_ulong_parameter(CKA_LABEL));
}

/* --- Parse invalid data --- */

TEST(test_parse_invalid_call_id)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	egg_buffer_add_uint32(&msg->buffer, 9999); /* invalid call id */
	msg->parsed = 0;
	assert(!gck_rpc_message_parse(msg, GCK_RPC_REQUEST));

	gck_rpc_message_free(msg);
}

TEST(test_parse_error_response)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	egg_buffer_add_uint32(&msg->buffer, GCK_RPC_CALL_ERROR);
	msg->parsed = 0;
	/* Error in response is valid */
	assert(gck_rpc_message_parse(msg, GCK_RPC_RESPONSE));

	gck_rpc_message_free(msg);
}

TEST(test_parse_error_request_invalid)
{
	GckRpcMessage *msg = gck_rpc_message_new((EggBufferAllocator)realloc);

	egg_buffer_add_uint32(&msg->buffer, GCK_RPC_CALL_ERROR);
	msg->parsed = 0;
	/* Error in request is invalid */
	assert(!gck_rpc_message_parse(msg, GCK_RPC_REQUEST));

	gck_rpc_message_free(msg);
}

int main(void)
{
	printf("test_rpc_message:\n");

	RUN_TEST(test_message_new_free);
	RUN_TEST(test_message_reset);
	RUN_TEST(test_prep_request);
	RUN_TEST(test_prep_response);
	RUN_TEST(test_prep_and_parse_roundtrip);
	RUN_TEST(test_write_read_byte);
	RUN_TEST(test_write_read_ulong);
	RUN_TEST(test_write_read_ulong_max);
	RUN_TEST(test_write_read_version);
	RUN_TEST(test_write_read_space_string);
	RUN_TEST(test_message_equals);
	RUN_TEST(test_verify_part);
	RUN_TEST(test_call_table_consistency);
	RUN_TEST(test_mechanism_no_parameters);
	RUN_TEST(test_mechanism_sane_parameters);
	RUN_TEST(test_mechanism_is_supported);
	RUN_TEST(test_has_ulong_parameter);
	RUN_TEST(test_parse_invalid_call_id);
	RUN_TEST(test_parse_error_response);
	RUN_TEST(test_parse_error_request_invalid);

	printf("All tests passed!\n");
	return 0;
}
