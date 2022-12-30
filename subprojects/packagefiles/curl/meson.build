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
		'lib/altsvc.c',
		'lib/amigaos.c',
		'lib/asyn-ares.c',
		'lib/asyn-thread.c',
		'lib/base64.c',
		'lib/bufref.c',
		'lib/c-hyper.c',
		'lib/conncache.c',
		'lib/connect.c',
		'lib/content_encoding.c',
		'lib/cookie.c',
		'lib/curl_addrinfo.c',
		'lib/curl_ctype.c',
		'lib/curl_des.c',
		'lib/curl_endian.c',
		'lib/curl_fnmatch.c',
		'lib/curl_get_line.c',
		'lib/curl_gethostname.c',
		'lib/curl_gssapi.c',
		'lib/curl_memrchr.c',
		'lib/curl_multibyte.c',
		'lib/curl_ntlm_core.c',
		'lib/curl_ntlm_wb.c',
		'lib/curl_path.c',
		'lib/curl_range.c',
		'lib/curl_rtmp.c',
		'lib/curl_sasl.c',
		'lib/curl_sspi.c',
		'lib/curl_threads.c',
		'lib/dict.c',
		'lib/doh.c',
		'lib/dotdot.c',
		'lib/dynbuf.c',
		'lib/easy.c',
		'lib/easygetopt.c',
		'lib/easyoptions.c',
		'lib/escape.c',
		'lib/file.c',
		'lib/fileinfo.c',
		'lib/fopen.c',
		'lib/formdata.c',
		'lib/ftp.c',
		'lib/ftplistparser.c',
		'lib/getenv.c',
		'lib/getinfo.c',
		'lib/gopher.c',
		'lib/h2h3.c',
		'lib/hash.c',
		'lib/headers.c',
		'lib/hmac.c',
		'lib/hostasyn.c',
		'lib/hostip.c',
		'lib/hostip4.c',
		'lib/hostip6.c',
		'lib/hostsyn.c',
		'lib/hsts.c',
		'lib/http.c',
		'lib/http_aws_sigv4.c',
		'lib/http_chunks.c',
		'lib/http_digest.c',
		'lib/http_negotiate.c',
		'lib/http_ntlm.c',
		'lib/http_proxy.c',
		'lib/http2.c',
		'lib/idn_win32.c',
		'lib/if2ip.c',
		'lib/imap.c',
		'lib/inet_ntop.c',
		'lib/inet_pton.c',
		'lib/krb5.c',
		'lib/ldap.c',
		'lib/llist.c',
		'lib/md4.c',
		'lib/md5.c',
		'lib/memdebug.c',
		'lib/mime.c',
		'lib/mprintf.c',
		'lib/mqtt.c',
		'lib/multi.c',
		'lib/netrc.c',
		'lib/nonblock.c',
		'lib/openldap.c',
		'lib/parsedate.c',
		'lib/pingpong.c',
		'lib/pop3.c',
		'lib/progress.c',
		'lib/psl.c',
		'lib/rand.c',
		'lib/rename.c',
		'lib/rtsp.c',
		'lib/select.c',
		'lib/sendf.c',
		'lib/setopt.c',
		'lib/sha256.c',
		'lib/share.c',
		'lib/slist.c',
		'lib/smb.c',
		'lib/smtp.c',
		'lib/socketpair.c',
		'lib/socks.c',
		'lib/socks_gssapi.c',
		'lib/socks_sspi.c',
		'lib/speedcheck.c',
		'lib/splay.c',
		'lib/strcase.c',
		'lib/strdup.c',
		'lib/strerror.c',
		'lib/strtok.c',
		'lib/strtoofft.c',
		'lib/system_win32.c',
		'lib/telnet.c',
		'lib/tftp.c',
		'lib/timediff.c',
		'lib/timeval.c',
		'lib/transfer.c',
		'lib/url.c',
		'lib/urlapi.c',
		'lib/version.c',
		'lib/version_win32.c',
		'lib/warnless.c',
		'lib/wildcard.c'
	], 
	include_directories : libcurl_inc,
	install: true,
	dependencies : curl_deps)
	
libcurl_dep = declare_dependency(include_directories : libcurl_inc, link_with: libcurl_lib)
meson.override_dependency('libcurl', libcurl_dep)