#include "flags.h"

void set_flag(int flag, int value)  {
  if (flag == 0) permissionToRunFlag = value;
  if (flag == 1) readerPermissionToReadFlag = value;
}

short unsigned int get_flag(int flag)  {
  if (flag ==0) return permissionToRunFlag;
  if (flag ==1) return readerPermissionToReadFlag;
}
