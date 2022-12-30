project('curl', ['c'])

libcurl_inc = include_directories('include')

zlib_dep = dependency('zlib', fallback: [ 'zlib','zlib_dep'])
openssl_dep = dependency('openssl', fallback: [ 'openssl','openssl_dep'])
curl_deps = [zlib_dep, openssl_dep]
host_system = host_machine.system()
compiler = meson.get_compiler('c')

message('OpenSSL: ' + openssl_dep.name() + ' : ' + openssl_dep.version() + ' : ' + host_system)

if host_system == 'windows'

    curl_deps += [compiler.find_library('ws2_32')]
    
	add_project_arguments([
		'-DUSE_WINSOCK',
		'-DWIN32=1'
	], language: ['cpp','c'])
	
	if host_machine.cpu_family() == 'x86_64'
		add_project_arguments([
		'-D_WIN64=1',
		], language: ['cpp','c'])
	endif
else
    curl_deps += compiler.find_library('dl')
endif

#
if get_option('unicode') and host_machine.system() == 'windows'
	add_project_arguments('-DUNICODE', language: ['cpp','c'])
endif
if get_option('ssl')
	add_project_arguments('-DUSE_OPENSSL=1', language: ['cpp','c'])
	add_project_arguments('-DUSE_SSLEAY=1', language: ['cpp','c'])
endif
add_project_arguments('-DCURL_DISABLE_FTP', language: ['cpp','c'])
add_project_arguments('-DCURL_DISABLE_TELNET', language: ['cpp','c'])
add_project_arguments('-DCURL_DISABLE_FTP', language: ['cpp','c'])
add_project_arguments('-DCURL_DISABLE_LDAP', language: ['cpp','c'])

if get_option('default_library') == 'static'
	add_project_arguments('-DCURL_STATICLIB', language: ['cpp','c'])
else
	add_project_arguments('-D_USRDLL', language: ['cpp','c'])
	add_project_arguments('-DBUILDING_LIBCURL', language: ['cpp','c'])
endif 


libcurl_lib = library('curl',[
		'lib/amigaos.c',
		'lib/base64.c',
		'lib/connect.c',
		'lib/content_encoding.c',
		'lib/cookie.c',
		'lib/curl_addrinfo.c',
		'lib/curl_sspi.c',
		'lib/dict.c',
		'lib/easy.c',
		'lib/escape.c',
		'lib/file.c',
		'lib/formdata.c',
		'lib/getenv.c',
		'lib/getinfo.c',
		'lib/gtls.c',
		'lib/hash.c',
		'lib/hostares.c',
		'lib/hostasyn.c',
		'lib/hostip.c',
		'lib/hostip4.c',
		'lib/hostip6.c',
		'lib/llist.c',
		'lib/hostsyn.c',
		'lib/hostthre.c',
		'lib/http_chunks.c',
		'lib/http_digest.c',
		'lib/http_negotiate.c',
		'lib/http_ntlm.c',
		'lib/http.c',
		'lib/if2ip.c',
		'lib/inet_ntop.c',
		'lib/inet_pton.c',
		'lib/krb4.c',
		'lib/krb5.c',
		'lib/md5.c',
		'lib/memdebug.c',
		'lib/mprintf.c',
		'lib/multi.c',
		'lib/netrc.c',
		'lib/nss.c',
		'lib/nwlib.c',
		'lib/nwos.c',
		'lib/parsedate.c',
		'lib/progress.c',
		'lib/qssl.c',
		'lib/rawstr.c',
		'lib/security.c',
		'lib/select.c',
		'lib/sendf.c',
		'lib/share.c',
		'lib/slist.c',
		'lib/socks_gssapi.c',
		'lib/socks_sspi.c',
		'lib/socks.c',
		'lib/speedcheck.c',
		'lib/splay.c',
		'lib/sslgen.c',
		'lib/ssluse.c',
		'lib/strdup.c',
		'lib/strequal.c',
		'lib/strerror.c',
		'lib/strtok.c',
		'lib/strtoofft.c',
		'lib/tftp.c',
		'lib/timeval.c',
		'lib/transfer.c',
		'lib/url.c',
		'lib/version.c',
		'lib/ssh.c',
		'lib/ftp.c',
		'lib/ldap.c',
		'lib/telnet.c'
	], 
	include_directories : libcurl_inc,
	install: true,
	dependencies : curl_deps)
	
libcurl_dep = declare_dependency(include_directories : libcurl_inc, link_with: libcurl_lib)
	