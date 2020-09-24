struct MBX_PKT{
	T_MSG pk_msg;
	unsigned int fromID;
	unsigned int toID;
	char data[50];
};

typedef struct MBX_PKT MBX_PKT;
