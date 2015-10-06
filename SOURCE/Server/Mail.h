
class MailManager
{
public:
	MailManager();
	~MailManager();

	bool Mail(const char *subject, const char *recipient, const char *body);
};

extern MailManager g_MailManager;
