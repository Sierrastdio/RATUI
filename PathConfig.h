#ifndef PATH_CONFIG_H
#define PATH_CONFIG_H

#define Config_FileName "config.rtuconf"

extern char INGEST_PATH[512]; //얘 폴더는 컴퓨터에 있음
extern char ROS_PATH[512];
extern char EDS_PATH[512];
extern char BKS_PATH[512]; 
extern char TRS_PATH[512]; 

void LOAD_CONFIG();
void ENSURE_DIRECTORIES();
void STRIP_NEWLINE(char *str);

#endif