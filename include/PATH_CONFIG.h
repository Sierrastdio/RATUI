#ifndef PATH_CONFIG_H
#define PATH_CONFIG_H

#define Config_FileName "config.ratui"

extern char *ARCHIVE_HOME_PATH;

void LOAD_CONFIG(void);
void ENSURE_HOME_DIRECTORY(void);
void FREE_CONFIG(void);

#endif