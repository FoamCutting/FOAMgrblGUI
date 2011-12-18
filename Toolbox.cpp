#include "Toolbox.h"


QString GetNumString(QString gcodeString, int *pos)
{
    QString numString("");
    (*pos)++;
    while((gcodeString[*pos] >= '0' && gcodeString[*pos] <= '9') || gcodeString[*pos] == '.' || gcodeString[*pos] == '-')
       {
	   numString.append(gcodeString[*pos]);
	   (*pos)++;
       }
    (*pos)--;	    //change gcode plot to make this unnecessary
    return numString;
}
