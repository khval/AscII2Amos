

class DynamicCommand
{

	public:
	unsigned short token;
	unsigned short extension;			// if there is a extension number.
	char *name;
	int len;
	int args;
	int comma;
	BOOL return_value;

	DynamicCommand(unsigned short token,unsigned short  extension, char *name,int args,BOOL return_value);
	DynamicCommand();
	~DynamicCommand();
};

