/* :tabSize=4:indentSize=4:folding=indent:
** $Id: Logging.h,v 1.4 2010/08/25 09:35:55 ken Exp $
*/
/**
*** \file Logging.h
*** \brief Implements the logging system
*** 
*** Informational, warning or error messages may be logged during program 
*** execution. These logging functions can log to the standard error stream or 
*** to a log file. Six logging levels are supported with increasing severity: 
*** Trace, Debug, Info, Warn, Error and Fatal. A Fatal log message will 
*** terminate the application.
**/
#ifndef LOGGING_H
#define LOGGING_H
#include <stdarg.h>

/** Writes log messages to standard error */
#define LOG_TOSTDERR 0x01
/** Writes log messages to file */
#define LOG_TOFILE 0x02
/** Reports trace-level log messages and higher (most low-level) */
#define LOG_LEVELTRACE 0x26
/** Reports debug-level log messages and higher */
#define LOG_LEVELDEBUG 0x1c
/** Reports informational log messages and higher */
#define LOG_LEVELINFO 0x06
/** Reports warning log messages only */
#define LOG_LEVELWARN 0x04

void Logging_setup(char *appName, unsigned long flags, char *logFileName);

void Logging__trace(char *fileName, int lineNo, char *msg);
void Logging__debug(char *fileName, int lineNo, char *msg);
void Logging__info(char *fileName, int lineNo, char *msg);
void Logging__warn(char *fileName, int lineNo, char *msg);
void Logging__error(char *fileName, int lineNo, char *msg);
void Logging__fatal(char *fileName, int lineNo, char *msg) __attribute__ ((noreturn));

void Logging__tracef(char *fileName, int lineNo, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void Logging__debugf(char *fileName, int lineNo, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void Logging__infof(char *fileName, int lineNo, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void Logging__warnf(char *fileName, int lineNo, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void Logging__errorf(char *fileName, int lineNo, const char *fmt, ...) __attribute__ ((format (printf, 3, 4)));
void Logging__fatalf(char *fileName, int lineNo, const char *fmt, ...) __attribute__ ((format (printf, 3, 4))) __attribute__ ((noreturn));

#define Logging_trace(msg) Logging__trace(__FILE__, __LINE__, msg)
#define Logging_debug(msg) Logging__debug(__FILE__, __LINE__, msg)
#define Logging_info(msg) Logging__info(__FILE__, __LINE__, msg)
#define Logging_warn(msg) Logging__warn(__FILE__, __LINE__, msg)
#define Logging_error(msg) Logging__error(__FILE__, __LINE__, msg)
#define Logging_fatal(msg) Logging__fatal(__FILE__, __LINE__, msg)

/** \note GNU Extension */
#define Logging_tracef(fmt, args...) Logging__tracef(__FILE__, __LINE__, fmt, ## args)
#define Logging_debugf(fmt, args...) Logging__debugf(__FILE__, __LINE__, fmt, ## args)
#define Logging_infof(fmt, args...) Logging__infof(__FILE__, __LINE__, fmt, ## args)
#define Logging_warnf(fmt, args...) Logging__warnf(__FILE__, __LINE__, fmt, ## args)
#define Logging_errorf(fmt, args...) Logging__errorf(__FILE__, __LINE__, fmt, ## args)
#define Logging_fatalf(fmt, args...) Logging__fatalf(__FILE__, __LINE__, fmt, ## args)

#define Logging_warnNullArg(fn) Logging_warnf("%s(): NULL argument.", fn)

#endif /* LOGGING_H */
