#include "Toolbox.h"


QString GetNumString(QString gcodeString, int *pos)
{
	bool leadingZero = true;
	QString numString("");
	(*pos)++;		//change gcode plot to make this unnecessary
	while((gcodeString[*pos] >= '0' && gcodeString[*pos] <= '9') || gcodeString[*pos] == '.' || gcodeString[*pos] == '-')
	{
	if(gcodeString[*pos] != '0')
		leadingZero = false;
	if(leadingZero == false || gcodeString[(*pos)+1] == '.' || gcodeString[(*pos)+1] <= ' ')
		numString.append(gcodeString[*pos]);
	(*pos)++;
	}
	(*pos)--;		//change gcode plot to make this unnecessary
	return numString;
}

bool CompareFloats(float x, float y)
{
	if (fabs(x - y) < 0.0001)
	return true;
	else
	return false;
}
