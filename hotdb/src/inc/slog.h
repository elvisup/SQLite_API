#ifndef __SLOG_H__
#define __SLOG_H__

enum {
	LOG_ERR,
	LOG_INFO,
	LOG_DBG
};

void slog(int priority, const char *format, ...);

#endif /*__SLOG_H__*/
