include (LibAddMacros)

add_plugin (
	hosts
	SOURCES hosts.h hosts-get.c hosts-set.c keymetaformatting.h keymetaformatting.c
	LINK_ELEKTRA elektra-ease elektra-proposal elektra-meta
	ADD_TEST TEST_README
	TEST_REQUIRED_PLUGINS network
	INSTALL_TEST_DATA)
