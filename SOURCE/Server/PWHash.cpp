#include "Util.h"
#include "AccountData.h"
#include "md5.hh"

int main(int argc, char *argv[])
{
	std::string username = argv[0];
	std::string pw = argv[1];
	std::string key = argv[2];

	MD5 hash;
	hash.update(argv[1], (unsigned int)pw.size());
	hash.finalize();

	std::string pwHashed = hash.hex_digest();

//	this.mOutBuf.putStringUTF(this.md5(::_username + ":" + this.md5(::_password) + ":" + auth_data));
	std::string password;
	AccountData::GenerateSaltedHash(authorizationHash.c_str(), password);
}
