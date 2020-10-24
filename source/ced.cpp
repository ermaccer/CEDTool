#include "ced.h"

const char * GetSectionType(int type)
{
	switch (type)
	{
	case 'MATI': return "Material Info"; break;
	case 'MAT_': return "Material Data"; break;
	case 'OBJI': return "Object Info"; break;
	case 'OBJ_': return "Object Data"; break;
	case 'BONI': return "Bone Information"; break;
	case 'PB__': return "3D Data"; break;
	default: return "Unknown"; break;
	}
}
