/* :tabSize=4:indentSize=4:folding=indent:
 * $Id: Logging.c,v 1.6 2006/06/05 17:05:34 ken Exp $
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "bool.h"
#include "Logging.h"

#define LOGBUFFER_INITIAL_SIZE 133

#ifdef _WIN32
#define vsnprintf _vsnprintf
#endif

static unsigned long logFlags = LOG_LEVELINFO | LOG_TOSTDERR;
static char   *logAppName     = NULL;
static FILE   *logFile        = NULL;
static size_t logBufferLength = 0;
static char   *logBuffer      = NULL;
static bool   exitRegistered  = false;


void doLogTo(FILE *f, char *level, char *fileName, int lineNo, char *msg){
	time_t t = time(NULL);
	struct tm *now = localtime(&t);
	char   timeStamp[20];
	strftime(timeStamp, 20, "%Y-%m-%d %H:%M:%S", now);
	if(fileName != NULL){
		fprintf(f, "%s\t%s\t%s\t(%s: %d)\t%s\n", 
				timeStamp, 
				(logAppName == NULL ? "unknown" : logAppName), 
				(level == NULL ? "UNKNOWN" : level), 
				fileName,
				lineNo,
				(msg == NULL ? "No message" : msg));
	}
	else{
		fprintf(f, "%s\t%s\t%s\t%s\n", 
				timeStamp, 
				(logAppName == NULL ? "unknown" : logAppName), 
				(level == NULL ? "UNKNOWN" : level), 
				(msg == NULL ? "No message" : msg));
	}
	printf("%s\n", msg);
}


void doLog(char *level, char *fileName, int lineNo, char *msg){
	if(logFlags & LOG_TOSTDERR){
		doLogTo(stderr, level, fileName, lineNo, msg);
	}
	if((logFlags & LOG_TOFILE) && logFile != NULL){
		doLogTo(logFile, level, fileName, lineNo, msg);
	}
}


void initBuffer(void){
	if(logBuffer == NULL){ 
		if((logBuffer = (char *)malloc(LOGBUFFER_INITIAL_SIZE * sizeof(char))) != NULL){
			logBufferLength = LOGBUFFER_INITIAL_SIZE;
		}
		else{
			doLogTo(stderr, "FATAL", __FILE__, __LINE__, 
					"Unable to allocate buffer for logging.");
			exit(EXIT_FAILURE);
		}
	}
}


void doLogf(char *level, char *fileName, int lineNo, const char *fmt, va_list ap){
	char   *tmp    = NULL;
	size_t newSize = 0;
	
	if(logBuffer == NULL)initBuffer();
	do{
		if(vsnprintf(logBuffer, logBufferLength, fmt, ap) >= logBufferLength){
			newSize = logBufferLength * 2;
			if((tmp = (char *)realloc(logBuffer, newSize)) == NULL){
				doLog("FATAL", __FILE__, __LINE__, 
						"Unable to extend logging buffer!");
				exit(EXIT_FAILURE);
			}
			else{
				logBuffer = tmp;
				logBufferLength = newSize;
			}
		}
		else{
			doLog(level, fileName, lineNo, logBuffer);
			break;
		}
	} while(true);
}


void stopLogging(void){
	if(logFile != NULL){
		doLogTo(logFile, "INFO", NULL, 0, "*** Program complete ***");
		fclose(logFile);
	}
	if(logBuffer != NULL) free(logBuffer);
	if(logAppName != NULL) free(logAppName);
}


void Logging__trace(char *fileName, int lineNo, char *msg){
	if((logFlags >> 2) >= (LOG_LEVELTRACE >> 2))
		doLog("TRACE", fileName, lineNo, msg);
}


void Logging__debug(char *fileName, int lineNo, char *msg){
	if((logFlags >> 2) >= (LOG_LEVELDEBUG >> 2))
		doLog("DEBUG", fileName, lineNo, msg);
}


void Logging__info(char *fileName, int lineNo, char *msg){
	if((logFlags >> 2) >= (LOG_LEVELINFO >> 2))
		doLog("INFO", fileName, lineNo, msg);
}


void Logging__warn(char *fileName, int lineNo, char *msg){
	if((logFlags >> 2) & (LOG_LEVELWARN >> 2))
		doLog("WARNING", fileName, lineNo, msg);
}


void Logging__error(char *fileName, int lineNo, char *msg){
	doLog("ERROR", fileName, lineNo, msg);
}


void Logging__fatal(char *fileName, int lineNo, char *msg){
	doLog("FATAL", fileName, lineNo, msg);
	exit(EXIT_FAILURE);
}


void Logging__tracef(char *fileName, int lineNo, const char *fmt, ...){
	va_list ap;
	
	if((logFlags >> 2) >= (LOG_LEVELTRACE >> 2)){
		va_start(ap, fmt);
		doLogf("TRACE", fileName, lineNo, fmt, ap);
		va_end(ap);
	}
}


void Logging__debugf(char *fileName, int lineNo, const char *fmt, ...){
	va_list ap;
	
	if((logFlags >> 2) >= (LOG_LEVELDEBUG >> 2)){
		va_start(ap, fmt);
		doLogf("DEBUG", fileName, lineNo, fmt, ap);
		va_end(ap);
	}
}


void Logging__infof(char *fileName, int lineNo, const char *fmt, ...){
	va_list ap;
	
	if((logFlags >> 2) >= (LOG_LEVELINFO >> 2)){
		va_start(ap, fmt);
		doLogf("INFO", fileName, lineNo, fmt, ap);
		va_end(ap);
	}
}


void Logging__warnf(char *fileName, int lineNo, const char *fmt, ...){
	va_list ap;
	
	if((logFlags >> 2) & (LOG_LEVELWARN >> 2)){
		va_start(ap, fmt);
		doLogf("WARNING", fileName, lineNo, fmt, ap);
		va_end(ap);
	}
}


void Logging__errorf(char *fileName, int lineNo, const char *fmt, ...){
	va_list ap;
	
	va_start(ap, fmt);
	doLogf("ERROR", fileName, lineNo, fmt, ap);
	va_end(ap);
}


void Logging__fatalf(char *fileName, int lineNo, const char *fmt, ...){
	va_list ap;
	
	va_start(ap, fmt);
	doLogf("FATAL", fileName, lineNo, fmt, ap);
	va_end(ap);
	exit(EXIT_FAILURE);
}


void Logging_setup(char *appName, unsigned long flags, char *logFileName){
	if(logBuffer == NULL) initBuffer();
	if(appName != NULL){
		logAppName = (char *)malloc((strlen(appName) + 1) * sizeof(char));
		if(logAppName != NULL) strcpy(logAppName, appName);
	}
	logFlags = flags;
	if((logFlags & LOG_TOFILE) && logFileName != NULL){
		if((logFile = fopen(logFileName, "a")) == NULL){
			Logging__warnf(__FILE__, __LINE__, 
				"Unable to open log file \"%s\": %s",
				logFileName, strerror(errno));
		}
	}
	if(!exitRegistered){
		atexit(stopLogging);
		exitRegistered = true;
	}
	Logging__info(NULL, 0, "*** Program started ***");
}

