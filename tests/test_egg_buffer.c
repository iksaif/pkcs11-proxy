#include "config.h"
#include "egg-buffer.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

#define TEST(name) static void name(void)
#define RUN_TEST(name) do { printf("  " #name "..."); name(); printf(" OK\n"); } while(0)

/* --- EggBuffer init/uninit tests --- */

TEST(test_init_default)
{
	EggBuffer buf;
	assert(egg_buffer_init(&buf, 128));
	assert(buf.buf != NULL);
	assert(buf.len == 0);
	assert(buf.allocated_len == 128);
	assert(!egg_buffer_has_error(&buf));
	egg_buffer_uninit(&buf);
}

TEST(test_init_zero_reserve)
{
	EggBuffer buf;
	assert(egg_buffer_init(&buf, 0));
	assert(buf.buf != NULL);
	assert(buf.allocated_len == 64); /* default minimum */
	egg_buffer_uninit(&buf);
}

TEST(test_init_static)
{
	unsigned char data[32];
	EggBuffer buf;

	memset(data, 0xAB, sizeof(data));
	egg_buffer_init_static(&buf, data, sizeof(data));

	assert(buf.buf == data);
	assert(buf.len == sizeof(data));
	assert(buf.allocated_len == sizeof(data));
	assert(buf.allocator == NULL);
	/* uninit should not free static buffers */
	egg_buffer_uninit(&buf);
}

TEST(test_init_allocated)
{
	unsigned char *data = malloc(64);
	EggBuffer buf;

	assert(data != NULL);
	memset(data, 0, 64);

	egg_buffer_init_allocated(&buf, data, 64, NULL);
	assert(buf.buf == data);
	assert(buf.len == 64);
	assert(buf.allocator != NULL); /* should have default allocator */
	egg_buffer_uninit(&buf);
}

/* --- Byte operations --- */

TEST(test_add_get_byte)
{
	EggBuffer buf;
	unsigned char val;
	size_t offset;

	assert(egg_buffer_init(&buf, 16));

	assert(egg_buffer_add_byte(&buf, 0x42));
	assert(egg_buffer_add_byte(&buf, 0xFF));
	assert(egg_buffer_add_byte(&buf, 0x00));
	assert(buf.len == 3);

	assert(egg_buffer_get_byte(&buf, 0, &offset, &val));
	assert(val == 0x42);
	assert(offset == 1);

	assert(egg_buffer_get_byte(&buf, 1, &offset, &val));
	assert(val == 0xFF);

	assert(egg_buffer_get_byte(&buf, 2, &offset, &val));
	assert(val == 0x00);

	/* Out of bounds */
	assert(!egg_buffer_get_byte(&buf, 3, &offset, &val));

	egg_buffer_uninit(&buf);
}

/* --- uint16 operations --- */

TEST(test_add_get_uint16)
{
	EggBuffer buf;
	uint16_t val;
	size_t offset;

	assert(egg_buffer_init(&buf, 16));

	assert(egg_buffer_add_uint16(&buf, 0x1234));
	assert(egg_buffer_add_uint16(&buf, 0));
	assert(egg_buffer_add_uint16(&buf, 0xFFFF));
	assert(buf.len == 6);

	assert(egg_buffer_get_uint16(&buf, 0, &offset, &val));
	assert(val == 0x1234);
	assert(offset == 2);

	assert(egg_buffer_get_uint16(&buf, 2, &offset, &val));
	assert(val == 0);

	assert(egg_buffer_get_uint16(&buf, 4, &offset, &val));
	assert(val == 0xFFFF);

	egg_buffer_uninit(&buf);
}

TEST(test_set_uint16)
{
	EggBuffer buf;
	uint16_t val;

	assert(egg_buffer_init(&buf, 16));
	assert(egg_buffer_add_uint16(&buf, 0));

	assert(egg_buffer_set_uint16(&buf, 0, 0xABCD));
	assert(egg_buffer_get_uint16(&buf, 0, NULL, &val));
	assert(val == 0xABCD);

	egg_buffer_uninit(&buf);
}

/* --- uint32 operations --- */

TEST(test_add_get_uint32)
{
	EggBuffer buf;
	uint32_t val;
	size_t offset;

	assert(egg_buffer_init(&buf, 16));

	assert(egg_buffer_add_uint32(&buf, 0x12345678));
	assert(egg_buffer_add_uint32(&buf, 0));
	assert(egg_buffer_add_uint32(&buf, 0xFFFFFFFF));
	assert(buf.len == 12);

	assert(egg_buffer_get_uint32(&buf, 0, &offset, &val));
	assert(val == 0x12345678);
	assert(offset == 4);

	assert(egg_buffer_get_uint32(&buf, 4, &offset, &val));
	assert(val == 0);

	assert(egg_buffer_get_uint32(&buf, 8, &offset, &val));
	assert(val == 0xFFFFFFFF);

	egg_buffer_uninit(&buf);
}

TEST(test_set_uint32)
{
	EggBuffer buf;
	uint32_t val;

	assert(egg_buffer_init(&buf, 16));
	assert(egg_buffer_add_uint32(&buf, 0));

	assert(egg_buffer_set_uint32(&buf, 0, 0xDEADBEEF));
	assert(egg_buffer_get_uint32(&buf, 0, NULL, &val));
	assert(val == 0xDEADBEEF);

	egg_buffer_uninit(&buf);
}

TEST(test_encode_decode_uint32)
{
	unsigned char raw[4];

	egg_buffer_encode_uint32(raw, 0x01020304);
	assert(raw[0] == 0x01);
	assert(raw[1] == 0x02);
	assert(raw[2] == 0x03);
	assert(raw[3] == 0x04);

	assert(egg_buffer_decode_uint32(raw) == 0x01020304);
}

/* --- uint64 operations --- */

TEST(test_add_get_uint64)
{
	EggBuffer buf;
	uint64_t val;
	size_t offset;

	assert(egg_buffer_init(&buf, 16));

	assert(egg_buffer_add_uint64(&buf, 0x0102030405060708ULL));
	assert(buf.len == 8);

	assert(egg_buffer_get_uint64(&buf, 0, &offset, &val));
	assert(val == 0x0102030405060708ULL);
	assert(offset == 8);

	egg_buffer_uninit(&buf);
}

TEST(test_uint64_zero_and_max)
{
	EggBuffer buf;
	uint64_t val;

	assert(egg_buffer_init(&buf, 32));

	assert(egg_buffer_add_uint64(&buf, 0));
	assert(egg_buffer_add_uint64(&buf, UINT64_MAX));

	assert(egg_buffer_get_uint64(&buf, 0, NULL, &val));
	assert(val == 0);

	assert(egg_buffer_get_uint64(&buf, 8, NULL, &val));
	assert(val == UINT64_MAX);

	egg_buffer_uninit(&buf);
}

/* --- Byte array operations --- */

TEST(test_add_get_byte_array)
{
	EggBuffer buf;
	const unsigned char *val;
	size_t vlen;
	unsigned char data[] = { 0x01, 0x02, 0x03, 0x04, 0x05 };

	assert(egg_buffer_init(&buf, 64));

	assert(egg_buffer_add_byte_array(&buf, data, sizeof(data)));
	assert(egg_buffer_get_byte_array(&buf, 0, NULL, &val, &vlen));
	assert(vlen == sizeof(data));
	assert(memcmp(val, data, sizeof(data)) == 0);

	egg_buffer_uninit(&buf);
}

TEST(test_null_byte_array)
{
	EggBuffer buf;
	const unsigned char *val;
	size_t vlen;

	assert(egg_buffer_init(&buf, 64));

	/* NULL array should be encoded specially */
	assert(egg_buffer_add_byte_array(&buf, NULL, 0));
	assert(egg_buffer_get_byte_array(&buf, 0, NULL, &val, &vlen));
	assert(val == NULL);
	assert(vlen == 0);

	egg_buffer_uninit(&buf);
}

/* --- String operations --- */

TEST(test_add_get_string)
{
	EggBuffer buf;
	char *str = NULL;
	size_t offset;

	assert(egg_buffer_init(&buf, 64));

	assert(egg_buffer_add_string(&buf, "hello"));
	assert(egg_buffer_get_string(&buf, 0, &offset, &str, NULL));
	assert(str != NULL);
	assert(strcmp(str, "hello") == 0);
	free(str);

	egg_buffer_uninit(&buf);
}

TEST(test_null_string)
{
	EggBuffer buf;
	char *str = (char *)1; /* non-NULL sentinel */
	size_t offset;

	assert(egg_buffer_init(&buf, 64));

	assert(egg_buffer_add_string(&buf, NULL));
	assert(egg_buffer_get_string(&buf, 0, &offset, &str, NULL));
	assert(str == NULL);

	egg_buffer_uninit(&buf);
}

TEST(test_empty_string)
{
	EggBuffer buf;
	char *str = NULL;
	size_t offset;

	assert(egg_buffer_init(&buf, 64));

	assert(egg_buffer_add_string(&buf, ""));
	assert(egg_buffer_get_string(&buf, 0, &offset, &str, NULL));
	assert(str != NULL);
	assert(strcmp(str, "") == 0);
	free(str);

	egg_buffer_uninit(&buf);
}

/* --- Buffer operations --- */

TEST(test_append)
{
	EggBuffer buf;
	unsigned char data[] = { 0xAA, 0xBB, 0xCC };

	assert(egg_buffer_init(&buf, 16));

	assert(egg_buffer_append(&buf, data, 3));
	assert(buf.len == 3);
	assert(buf.buf[0] == 0xAA);
	assert(buf.buf[1] == 0xBB);
	assert(buf.buf[2] == 0xCC);

	egg_buffer_uninit(&buf);
}

TEST(test_resize)
{
	EggBuffer buf;

	assert(egg_buffer_init(&buf, 16));
	assert(egg_buffer_resize(&buf, 100));
	assert(buf.len == 100);
	assert(buf.allocated_len >= 100);

	egg_buffer_uninit(&buf);
}

TEST(test_reserve_growth)
{
	EggBuffer buf;

	assert(egg_buffer_init(&buf, 16));
	assert(buf.allocated_len == 16);

	assert(egg_buffer_reserve(&buf, 256));
	assert(buf.allocated_len >= 256);
	assert(buf.len == 0); /* reserve shouldn't change len */

	egg_buffer_uninit(&buf);
}

TEST(test_reset)
{
	EggBuffer buf;

	assert(egg_buffer_init(&buf, 16));
	assert(egg_buffer_add_uint32(&buf, 42));
	assert(buf.len == 4);

	egg_buffer_reset(&buf);
	assert(buf.len == 0);
	assert(!egg_buffer_has_error(&buf));

	egg_buffer_uninit(&buf);
}

TEST(test_equal)
{
	EggBuffer b1, b2;

	assert(egg_buffer_init(&b1, 16));
	assert(egg_buffer_init(&b2, 16));

	assert(egg_buffer_add_uint32(&b1, 42));
	assert(egg_buffer_add_uint32(&b2, 42));
	assert(egg_buffer_equal(&b1, &b2));

	assert(egg_buffer_add_byte(&b1, 1));
	assert(!egg_buffer_equal(&b1, &b2));

	egg_buffer_uninit(&b1);
	egg_buffer_uninit(&b2);
}

TEST(test_add_empty)
{
	EggBuffer buf;
	unsigned char *ptr;

	assert(egg_buffer_init(&buf, 16));

	ptr = egg_buffer_add_empty(&buf, 10);
	assert(ptr != NULL);
	assert(buf.len == 10);

	/* Write into the empty space */
	memset(ptr, 0xFF, 10);
	assert(buf.buf[0] == 0xFF);
	assert(buf.buf[9] == 0xFF);

	egg_buffer_uninit(&buf);
}

/* --- Out of bounds / error handling --- */

TEST(test_get_uint32_out_of_bounds)
{
	EggBuffer buf;
	uint32_t val;

	assert(egg_buffer_init(&buf, 16));
	assert(egg_buffer_add_byte(&buf, 0x01)); /* only 1 byte */

	assert(!egg_buffer_get_uint32(&buf, 0, NULL, &val));
	assert(egg_buffer_has_error(&buf));

	egg_buffer_uninit(&buf);
}

TEST(test_static_buffer_no_grow)
{
	unsigned char data[4];
	EggBuffer buf;

	egg_buffer_init_static(&buf, data, sizeof(data));

	/* Can't grow a static buffer */
	assert(!egg_buffer_reserve(&buf, 100));
	assert(egg_buffer_has_error(&buf));

	egg_buffer_uninit(&buf);
}

/* --- Encode/decode uint16 --- */

TEST(test_encode_decode_uint16)
{
	unsigned char raw[2];

	egg_buffer_encode_uint16(raw, 0x0102);
	assert(raw[0] == 0x01);
	assert(raw[1] == 0x02);
	assert(egg_buffer_decode_uint16(raw) == 0x0102);
}

/* --- Sequential reads --- */

TEST(test_sequential_reads)
{
	EggBuffer buf;
	size_t offset = 0;
	uint32_t u32;
	unsigned char byte;
	uint16_t u16;

	assert(egg_buffer_init(&buf, 64));

	assert(egg_buffer_add_uint32(&buf, 100));
	assert(egg_buffer_add_byte(&buf, 0x42));
	assert(egg_buffer_add_uint16(&buf, 0x1234));

	assert(egg_buffer_get_uint32(&buf, offset, &offset, &u32));
	assert(u32 == 100);
	assert(offset == 4);

	assert(egg_buffer_get_byte(&buf, offset, &offset, &byte));
	assert(byte == 0x42);
	assert(offset == 5);

	assert(egg_buffer_get_uint16(&buf, offset, &offset, &u16));
	assert(u16 == 0x1234);
	assert(offset == 7);

	egg_buffer_uninit(&buf);
}

int main(void)
{
	printf("test_egg_buffer:\n");

	RUN_TEST(test_init_default);
	RUN_TEST(test_init_zero_reserve);
	RUN_TEST(test_init_static);
	RUN_TEST(test_init_allocated);
	RUN_TEST(test_add_get_byte);
	RUN_TEST(test_add_get_uint16);
	RUN_TEST(test_set_uint16);
	RUN_TEST(test_add_get_uint32);
	RUN_TEST(test_set_uint32);
	RUN_TEST(test_encode_decode_uint32);
	RUN_TEST(test_add_get_uint64);
	RUN_TEST(test_uint64_zero_and_max);
	RUN_TEST(test_add_get_byte_array);
	RUN_TEST(test_null_byte_array);
	RUN_TEST(test_add_get_string);
	RUN_TEST(test_null_string);
	RUN_TEST(test_empty_string);
	RUN_TEST(test_append);
	RUN_TEST(test_resize);
	RUN_TEST(test_reserve_growth);
	RUN_TEST(test_reset);
	RUN_TEST(test_equal);
	RUN_TEST(test_add_empty);
	RUN_TEST(test_get_uint32_out_of_bounds);
	RUN_TEST(test_static_buffer_no_grow);
	RUN_TEST(test_encode_decode_uint16);
	RUN_TEST(test_sequential_reads);

	printf("All tests passed!\n");
	return 0;
}
