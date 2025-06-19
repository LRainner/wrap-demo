#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <unistd.h>
#include <string.h>

static char buf[1024 * 1024 * 100];
static const char prefix[] = "[WRAP] ";

int __real_fprintf(FILE *__stream, const char *__fmt, ...);

// 包装 fprintf
int __wrap_fprintf(FILE *stream, const char *format, ...) {
    int vsps = 0;
    va_list args;
    va_start(args, format);
    vsps = vsnprintf(buf + sizeof(prefix) - 1, sizeof(buf) - (sizeof(prefix) - 1), format, args);
    va_end(args);

    // 先把 prefix 拷贝到 buf 开头
    memcpy(buf, prefix, sizeof(prefix) - 1);

    // 输出时用 "%.*s" 限定长度，避免缓冲区末尾未清理
    return __real_fprintf(stream, "%.*s", vsps + (int)(sizeof(prefix) - 1), buf);
}