

class DynamicCommand
{

	public:
	unsigned short token;
	int extension;			// if there is a extension number.
	char *name;
	int len;
	int args;
	int comma;
	BOOL return_value;

	DynamicCommand(unsigned short token, char *name,int args,BOOL return_value);
	DynamicCommand();
	~DynamicCommand();
};

