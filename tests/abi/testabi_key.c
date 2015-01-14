/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/

#include <tests.h>

struct test {
	char	*testName;
	char	*keyName;
	
	char	*expectedKeyName;
	char	*expectedBaseName;
	char	*expectedFRootName;
	char	*expectedParentName;
};

struct test tstKeyName[] = 
{
	{ "Normal key", "system/foo/bar",
		"system/foo/bar",
		"bar",
		"system",
		"system/foo"
	},

	{ "Key containing redundant & trailing separator", "system//foo//bar//",
		"system/foo/bar", 	/* keyName 	*/
		"bar", 			/* keyBaseName	*/
		"system",		/* keyGetFullRootName	*/
		"system/foo"		/* keyGetParentName	*/
	},

	{ "Normal user key", "user/key",
		"user/key", 			/* keyName 	*/
		"key", 				/* keyBaseName 	*/
		"user",				/* keyGetFullRootName 	*/ 
		"user"				/* keyGetParentName	*/
	
	},

	{ "Normal user key with owner", "user:owner/key",
		"user/key", 			/* keyName 	*/
		"key", 				/* keyBaseName 	*/
		"user:owner",			/* keyGetFullRootName 	*/ 
		"user"				/* keyGetParentName	*/
	
	},

	{ "Depth user key with owner", "user:owner/folder/long/base/dir/key",
		"user/folder/long/base/dir/key", /* keyName 	*/
		"key", 				/* keyBaseName 	*/
		"user:owner",			/* keyGetFullRootName 	*/ 
		"user/folder/long/base/dir"	/* keyGetParentName	*/
	
	},

#ifdef COMPAT
	{ "Key containing escaped separator", "user:yl///foo\\///bar\\/foo_bar\\",
		"user/foo\\//bar\\/foo_bar\\", 	/* keyName 	*/
		"bar/foo_bar\\", 		/* keyBaseName 	*/
		"user:yl",			/* keyGetFullRootName 	*/ 
		"user/foo\\/"			/* keyGetParentName	*/
	
	},
#endif

	{ "Key containing escaped separator at the end", "user:yl///foo\\///bar\\/foo_bar\\/",
		"user/foo\\//bar\\/foo_bar\\/",	/* keyName 	*/
		"bar/foo_bar/", 		/* keyBaseName 	*/
		"user:yl",			/* keyGetFullRootName 	*/ 
		"user/foo\\/"			/* keyGetParentName	*/
	
	},

	{ "Key with empty part", "user///%",
		"user/%",	/* keyName 	*/
		"", 		/* keyBaseName 	*/
		"",			/* keyGetFullRootName 	*/ 
		""			/* keyGetParentName	*/
	
	},

	{ "Key with escaped %", "user///\\%",
		"user/\\%",	/* keyName 	*/
		"%", 		/* keyBaseName 	*/
		"",			/* keyGetFullRootName 	*/ 
		""			/* keyGetParentName	*/
	
	},

	{ "Key with multi escaped %", "user///\\\\%",
		"user/\\\\%",	/* keyName 	*/
		"\\%", 		/* keyBaseName 	*/
		"",			/* keyGetFullRootName 	*/ 
		""			/* keyGetParentName	*/
	
	},

	{ NULL, NULL,
		NULL, /* keyName 	*/
		NULL, /* keyBaseName 	*/
		NULL, /* keyGetFullRootName 	*/ 
		NULL  /* keyGetParentName	*/
	}
};

static void test_keyNewSpecial()
{
	printf ("Test special key creation\n");

	Key *k = keyNew (0);
	succeed_if_same_string (keyName(k), "");
	keyDel (k);

	k = keyNew (0, KEY_END); //  might break, useless arguments?
	succeed_if_same_string (keyName(k), "");
	keyDel (k);

	k = keyNew ("", KEY_END);
	succeed_if_same_string (keyName(k), "");
	keyDel (k);

	k = keyNew ("invalid", KEY_END);
	succeed_if_same_string (keyName(k), "");
	keyDel (k);


	k = keyNew ("other invalid", KEY_END);
	succeed_if_same_string (keyName(k), "");
	keyDel (k);

	k = keyNew ("system spaces", KEY_END);
	succeed_if_same_string (keyName(k), "");
	keyDel (k);

	k = keyNew ("system/bin",
			KEY_BINARY,
			KEY_VALUE, "a 2d\0b",
			KEY_END);
	succeed_if_same_string (keyValue(k), "a 2d");
	succeed_if(keyGetValueSize(k) == sizeof("a 2d"), "no KEY_SIZE given, so bin is truncated");
	succeed_if(keyIsBinary(k), "not a binary key");
	keyDel (k);
}

static void test_keyNewSystem()
{
	Key     *key;
	char array[] = "here is some data stored";
	Key *k1;
	Key *k2;
	Key *k3;
	char * getBack;

	printf("Test system key creation\n");

	// Empty key
	key = keyNew(0);
	succeed_if(key != NULL, "keyNew: Unable to create a new empty key");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete empty key");

	// Key with name
	key = keyNew("system/sw/test", KEY_END);
	succeed_if(key != NULL, "keyNew: Unable to create a key with name");
	succeed_if_same_string (keyName(key), "system/sw/test");
	keyCopy (key, 0);
	succeed_if_same_string (keyName(key), "");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name");
	
	// Key with name
	key = keyNew("system/sw/test", KEY_END);
	succeed_if(key != NULL, "keyNew: Unable to create a key with name");
	succeed_if_same_string (keyName(key), "system/sw/test");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name");
	
	// Key with name + value
	key = keyNew("system/sw/test",
			KEY_VALUE, "test",
			KEY_END);
	succeed_if(key != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if_same_string (keyValue(key), "test");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name + value");
	key = keyNew("system/valid/there",
			KEY_BINARY,
			KEY_SIZE, sizeof(array),
			KEY_VALUE, array,
			KEY_END);
	succeed_if(key != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsBinary (key), "Could not set type to binary");
	succeed_if(keyGetValueSize(key) == sizeof(array), "Value size not correct");
	succeed_if(memcmp ((char *) keyValue(key), array, sizeof(array)) == 0, "could not get correct binary value");
	getBack = malloc (keyGetValueSize(key));
	keyGetBinary(key, getBack, keyGetValueSize(key));
	succeed_if(memcmp(getBack, array, sizeof(array)) == 0, "could not get correct value with keyGetBinary");
	free (getBack);
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name + owner");

	key = keyNew("system", KEY_END);
	succeed_if_same_string (keyName(key), "system");
	succeed_if (keyGetNameSize(key) == 7, "empty name size" );
	succeed_if(keyValue(keyGetMeta(key, "owner")) == 0, "owner not null");
	keyDel (key);

	key = keyNew("user/abc", KEY_OWNER, "huhu", KEY_END);
	succeed_if_same_string (keyName(key), "user/abc");
	succeed_if (keyGetNameSize(key) == 9, "empty name size" );
	succeed_if_same_string (keyString(keyGetMeta(key, "owner")), "huhu");
	keyDel (key);

	key = keyNew("user:lost/abc", KEY_OWNER, "huhu", KEY_END);
	succeed_if_same_string (keyName(key), "user/abc");
	succeed_if (keyGetNameSize(key) == 9, "empty name size" );
	succeed_if_same_string (keyString(keyGetMeta(key, "owner")), "huhu");
	keyDel (key);

	key = keyNew("user:huhu/abc", KEY_END);
	succeed_if_same_string (keyName(key), "user/abc");
	succeed_if (keyGetNameSize(key) == 9, "empty name size" );
	succeed_if_same_string (keyString(keyGetMeta(key, "owner")), "huhu");
	keyDel (key);

	// testing multiple values at once
	k1=keyNew("system/1",  KEY_VALUE, "singlevalue", KEY_END);
	k2=keyNew("system/2",   KEY_VALUE, "myvalue", KEY_END);
	k3=keyNew("system/3", KEY_VALUE, "syskey",  KEY_END);
	succeed_if(k1 != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsString(k1), "keyNew: Default key value isn't set to string");
	succeed_if_same_string (keyValue(k1), "singlevalue");
	
	succeed_if(k2 != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsString(k2), "keyNew: Default key value isn't set to string");
	succeed_if_same_string (keyValue(k2), "myvalue");
	
	succeed_if(k3 != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsString(k3), "keyNew: Default key value isn't set to string");
	succeed_if_same_string (keyValue(k3), "syskey");

	succeed_if(keyDel(k1) == 0, "keyDel: Unable to delete key with name + value");
	succeed_if(keyDel(k2) == 0, "keyDel: Unable to delete key with name + value");
	succeed_if(keyDel(k3) == 0, "keyDel: Unable to delete key with name + value");
}

static void test_keyNewUser()
{
	Key *k1;
	Key *k2;
	Key *k3;

	printf("Test user key creation\n");
	// testing multiple values at once
	k1=keyNew("user/1",  KEY_VALUE, "singlevalue", KEY_END);
	k2=keyNew("user/2",   KEY_VALUE, "myvalue", KEY_END);
	k3=keyNew("user/3", KEY_VALUE, "syskey",  KEY_END);
	succeed_if(k1 != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsString(k1), "keyNew: Default key value isn't set to string");
	succeed_if_same_string (keyValue(k1), "singlevalue");
	
	succeed_if(k2 != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsString(k2), "keyNew: Default key value isn't set to string");
	succeed_if_same_string (keyValue(k2), "myvalue");
	
	succeed_if(k3 != NULL, "keyNew: Unable to create a key with name + value of default type");
	succeed_if(keyIsString(k3), "keyNew: Default key value isn't set to string");
	succeed_if_same_string (keyValue(k3), "syskey");

	succeed_if(keyDel(k1) == 0, "keyDel: Unable to delete key with name + value");
	succeed_if(keyDel(k2) == 0, "keyDel: Unable to delete key with name + value");
	succeed_if(keyDel(k3) == 0, "keyDel: Unable to delete key with name + value");

	k1 = keyNew ("invalid", KEY_END);
	succeed_if (k1 != 0, "should construct key even on invalid names");
	succeed_if_same_string (keyName(k1), "");
	succeed_if (keyGetNameSize(k1) == 1, "namesize");
	keyDel (k1);
}

static void test_keyReference()
{
	printf("Test key reference\n");

	Key *key = keyNew ("user/key", KEY_END);
	Key *c = keyNew ("user/c", KEY_END);
	Key *d;
	KeySet *ks1, *ks2;
	succeed_if (keyGetRef(key) == 0, "New created key reference");

	succeed_if (keyIncRef (key) == 1, "keyIncRef return value");
	succeed_if (keyGetRef(key) == 1, "After keyIncRef key reference");
	succeed_if (keyIncRef (key) == 2, "keyIncRef return value");
	succeed_if (keyGetRef(key) == 2, "After keyIncRef key reference");
	succeed_if (keyIncRef (key) == 3, "keyIncRef return value");
	succeed_if (keyGetRef(key) == 3, "After keyIncRef key reference");
	succeed_if (keyIncRef (key) == 4, "keyIncRef return value");
	succeed_if (keyGetRef(key) == 4, "After keyIncRef key reference");

	d = keyDup (key);
	succeed_if (keyGetRef(d) == 0, "After keyDup key reference");
	succeed_if (keyIncRef (d) == 1, "keyIncRef return value");
	succeed_if (keyGetRef(key) == 4, "Reference should not change");
	succeed_if (keyDecRef (d) == 0, "decrement key");
	succeed_if (keyDel(d) == 0, "last keyDel d, key exist");

	keyCopy (c, key);
	succeed_if (keyGetRef(c) == 0, "After keyCopy key reference");
	succeed_if (keyIncRef (c) == 1, "keyIncRef return value");
	succeed_if (keyGetRef(key) == 4, "Reference should not change");

	keyCopy (c, key);
	succeed_if (keyGetRef(c) == 1, "After keyCopy key reference");
	succeed_if (keyDecRef (c) == 0, "keyDecRef return value");
	succeed_if (keyGetRef(key) == 4, "Reference should not change");

	succeed_if (keyIncRef (c) == 1, "keyIncRef return value");
	succeed_if (keyIncRef (c) == 2, "keyIncRef return value");
	keyCopy (c, key);
	succeed_if (keyGetRef(c) == 2, "After keyCopy key reference");
	succeed_if (keyDecRef (c) == 1, "keyDecRef return value");
	succeed_if (keyDecRef (c) == 0, "keyDecRef return value");
	succeed_if (keyDel (c) == 0, "could not delete copy");

	succeed_if (keyGetRef(key) == 4, "After keyIncRef key reference");
	succeed_if (keyDecRef (key) == 3, "keyDel return value");
	succeed_if (keyDel (key) == 3, "should not do anything");
	succeed_if (keyGetRef(key) == 3, "After keyIncRef key reference");
	succeed_if (keyDecRef (key) == 2, "keyDel return value");
	succeed_if (keyDel (key) == 2, "should not do anything");
	succeed_if (keyGetRef(key) == 2, "After keyIncRef key reference");
	succeed_if (keyDecRef (key) == 1, "keyDel return value");
	succeed_if (keyDel (key) == 1, "should not do anything");
	succeed_if (keyGetRef(key) == 1, "Should have no more reference");
	succeed_if (keyDecRef (key) == 0, "last keyDel key, key exist");
	succeed_if (keyDel (key) == 0, "last keyDel key, key exist");

	/* From examples in ksNew () */
	key = keyNew(0); // ref counter 0
	succeed_if (keyGetRef(key) == 0, "reference counter");
	keyIncRef(key); // ref counter of key 1
	succeed_if (keyGetRef(key) == 1, "reference counter");
	keyDel(key);    // has no effect
	succeed_if (keyGetRef(key) == 1, "reference counter");
	keyDecRef(key); // ref counter back to 0
	succeed_if (keyGetRef(key) == 0, "reference counter");
	keyDel(key);    // key is now deleted

	ks1 = ksNew(0, KS_END);
	ks2 = ksNew(0, KS_END);
	key = keyNew("user/key", KEY_END); // ref counter 0
	succeed_if (keyGetRef(key) == 0, "reference counter");
	ksAppendKey(ks1, key); // ref counter of key 1
	succeed_if (keyGetRef(key) == 1, "reference counter");
	ksAppendKey(ks2, key); // ref counter of key 2
	succeed_if (keyGetRef(key) == 2, "reference counter");
	ksDel(ks1); // ref counter of key 1
	succeed_if (keyGetRef(key) == 1, "reference counter");
	ksDel(ks2); // key is now deleted

	key = keyNew(0); // ref counter 0
	succeed_if (keyGetRef(key) == 0, "reference counter");
	keyIncRef(key); // ref counter of key 1
	succeed_if (keyGetRef(key) == 1, "reference counter");
	keyDel (key);   // has no effect
	succeed_if (keyGetRef(key) == 1, "reference counter");
	keyIncRef(key); // ref counter of key 2
	succeed_if (keyGetRef(key) == 2, "reference counter");
	keyDel (key);   // has no effect
	succeed_if (keyGetRef(key) == 2, "reference counter");
	keyDecRef(key); // ref counter of key 1
	succeed_if (keyGetRef(key) == 1, "reference counter");
	keyDel (key);   // has no effect
	succeed_if (keyGetRef(key) == 1, "reference counter");
	keyDecRef(key); // ref counter is now 0
	succeed_if (keyGetRef(key) == 0, "reference counter");
	keyDel (key); // key is now deleted

	Key *k = keyNew("system/proper_name", KEY_END); // ref counter = 0
	succeed_if(keyGetRef(k) == 0, "ref should be zero");
	KeySet *ks = ksNew (1, k, KS_END);
	succeed_if(keyGetRef(k) == 1, "ref should be one");
	succeed_if(keyDel(k) == 1, "key will not be deleted, because its in the keyset");
	succeed_if(keyGetRef(k) == 1, "ref should be one");
	succeed_if(ksDel(ks) == 0, "could not del"); // now the key will be deleted

	return;

	/* This code needs very long to execute, especially on 64bit
	 * systems. */

	key = keyNew(0); // ref counter 0
	while (keyGetRef(key) < SSIZE_MAX) keyIncRef(key);
	succeed_if (keyGetRef(key) == SSIZE_MAX, "reference counter");
	succeed_if (keyIncRef(key) == SSIZE_MAX, "should stay at maximum");
	succeed_if (keyGetRef(key) == SSIZE_MAX, "reference counter");
	succeed_if (keyIncRef(key) == SSIZE_MAX, "should stay at maximum");
	keyDel (key);

}

static void test_keyName()
{
	Key	*key;
	char	ret [1000];
	size_t	i;
	char testName[] = "user/name";
	char testFullName[] = "user:max/name";
	char testBaseName[] = "name";

#ifdef HAVE_CLEARENV
	clearenv();
#else
	unsetenv("USER");
#endif

	printf ("Test Key Name\n");

	key = keyNew (testName, KEY_END);
	succeed_if (keyGetName (0,ret,100) == -1, "null pointer");
	succeed_if (keyGetName (key,0,100) == -1, "string null pointer");
	succeed_if (keyGetName (key,ret,0) == -1, "length checking");
	for (i=1; i< sizeof(testName);i++)
	{
		succeed_if (keyGetName (key,ret,i) == -1, "length checking too short");
	}
	for (i=sizeof(testName); i<sizeof(testName)*2; i++)
	{
		succeed_if (keyGetName (key,ret,i) == sizeof(testName), "length checking longer");
	}
	succeed_if (keyGetName (key,ret, (size_t)-1) == -1, "maxSize exceeded");
	keyDel (key);

	succeed_if (keyName(0) == 0, "null pointer");

	key = keyNew (0);
	succeed_if_same_string (keyName(key), "");
	succeed_if (keyGetName (key,ret, 1000) == 1, "get empty name");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetName (key,ret, 0) == -1, "get empty name");
	keyDel (key);

	succeed_if (keySetName(0,ret) == -1, "Null pointer");



	printf ("Test Key Full Name\n");

	key = keyNew (testFullName, KEY_END);
	succeed_if (keyGetFullName (0,ret,100) == -1, "null pointer");
	succeed_if (keyGetFullName (key,0,100) == -1, "string null pointer");
	succeed_if (keyGetFullName (key,ret,0) == -1, "length checking");
	for (i=1; i< sizeof(testFullName);i++)
	{
		succeed_if (keyGetFullName (key,ret,i) == -1, "length checking too short");
	}
	for (i=sizeof(testFullName); i<sizeof(testFullName)*2; i++)
	{
		succeed_if (keyGetFullName (key,ret,i) == sizeof(testFullName), "length checking longer");
	}
	succeed_if (keyGetFullName (key,ret, (size_t)-1) == -1, "maxSize exceeded");
	keyDel (key);

	key = keyNew (0);
	succeed_if (keyGetFullName (key,ret, 1000) == 1, "get empty name");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetFullName (key,ret, 0) == -1, "get empty name");
	keyDel (key);

	succeed_if (keySetName(0,ret) == -1, "Null pointer");




	printf ("Test Key Base Name\n");

	key = keyNew (testFullName, KEY_END);
	succeed_if (keyGetBaseName (0,ret,100) == -1, "null pointer");
	succeed_if (keyGetBaseName (key,0,100) == -1, "string null pointer");
	succeed_if (keyGetBaseName (key,ret,0) == -1, "length checking");
	for (i=1; i< sizeof(testBaseName);i++)
	{
		succeed_if (keyGetBaseName (key,ret,i) == -1, "length checking too short");
	}
	for (i=sizeof(testBaseName); i<sizeof(testBaseName)*2; i++)
	{
		succeed_if (keyGetBaseName (key,ret,i) == sizeof(testBaseName), "length checking longer");
	}
	succeed_if (keyGetBaseName (key,ret, (size_t)-1) == -1, "maxSize exceeded");
	keyDel (key);

	succeed_if (keyBaseName(0) == 0, "null pointer");

	key = keyNew (0);
	succeed_if_same_string (keyBaseName(key), "");
	succeed_if (keyGetBaseName (key,ret, 1000) == 1, "get empty name");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetBaseName (key,ret, 0) == -1, "get empty name");
	keyDel (key);

	succeed_if (keySetName(0,ret) == -1, "Null pointer");

	key = keyNew ("user", KEY_END);
	succeed_if (keyGetBaseNameSize (key) == 1, "length checking");
	succeed_if (keyGetBaseName (key,ret,1) == 1, "GetBaseName for root key");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetBaseName (key,ret,2) == 1, "GetBaseName for root key");
	succeed_if_same_string (ret, "");
	keyDel (key);

	key = keyNew ("system", KEY_END);
	succeed_if (keyGetBaseNameSize (key) == 1, "length checking");
	succeed_if (keyGetBaseName (key,ret,1) == 1, "GetBaseName for root key");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetBaseName (key,ret,2) == 1, "GetBaseName for root key");
	succeed_if_same_string (ret, "");
	keyDel (key);
}


static void test_keyNameSlashes()
{
	printf("Test Slashes in Key Name\n");
	size_t	size;
	char	*buf;
	char * getBack;
	char	ret [1000];
	Key *key = 0;
	int i;


	key = keyNew(0);
	succeed_if (keyGetNameSize(key) == 1, "empty name size" );
	keyDel (key);

	key = keyNew("", KEY_END);
	succeed_if (key != 0, "key should not be null!");
	succeed_if_same_string (keyName(key), "");
	succeed_if (keyGetName(key,ret, 999) == 1, "keyGetName should return 1");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetNameSize(key) == 1, "empty name size" );
	keyDel (key);

	key = keyNew (0);
	keySetName(key,"user");
	succeed_if_same_string (keyName(key), "user");
	succeed_if (keyGetNameSize(key) == 5, "empty name size" );

	keySetName(key,"system");
	succeed_if_same_string (keyName(key), "system");
	succeed_if (keyGetNameSize(key) == 7, "empty name size" );
	keyDel (key);

	key = keyNew (0);
	keySetName(key,"system");
	succeed_if_same_string (keyName(key), "system");
	succeed_if (keyGetNameSize(key) == 7, "empty name size" );

	keySetName(key,"user");
	succeed_if_same_string (keyName(key), "user");
	succeed_if (keyGetNameSize(key) == 5, "empty name size" );
	keyDel (key);

	key = keyNew(0);
	succeed_if (keySetName(key,"user:") == 5, "setting user: generates error");
	succeed_if_same_string (keyName(key), "user");
	succeed_if (keyGetNameSize(key) == 5, "empty name size" );
	keyDel (key);

	key = keyNew(0);
	succeed_if (keySetName(key,"user:y") == 5, "setting user: generates error");
	succeed_if_same_string (keyName(key), "user");
	succeed_if (keyGetNameSize(key) == 5, "empty name size" );
	keyDel (key);

	key = keyNew(0);
	succeed_if (keySetName(key,"no") == -1, "no error code setting invalid name");
	succeed_if_same_string (keyName(key), "");
	succeed_if (keyGetNameSize(key) == 1, "empty name size" );
	keyDel (key);

	key = keyNew("user/noname", KEY_END);
	succeed_if(keyGetNameSize(key) == 12, "size not correct after keyNew");
	getBack = malloc (12);
	succeed_if(keyGetName(key, getBack, 12), "could not get name");
	succeed_if_same_string (getBack, "user/noname");
	free (getBack);

	keySetName (key, "user/noname");
	succeed_if(keyGetNameSize(key) == 12, "size not correct after keySetName");
	getBack = malloc (12);
	succeed_if(keyGetName(key, getBack, 12), "could not get name");
	succeed_if_same_string (getBack, "user/noname");
	free (getBack);

	keySetName (key, "no");
	succeed_if(keyGetNameSize(key) == 1, "size not correct after keySetName");
	getBack = malloc (1);
	succeed_if(keyGetName(key, getBack, 1), "could not get name");
	succeed_if_same_string (getBack, "");
	free (getBack);
	keyDel (key);

	key = keyNew("user/noname", KEY_END);
	keySetName(key,"");
	succeed_if_same_string (keyName(key), "");
	succeed_if (keyGetName(key,ret, 999) == 1, "keyGetName should return 1");
	succeed_if_same_string (ret, "");
	succeed_if (keyGetNameSize(key) == 1, "empty name size" );

	keySetName(key,"user//hidden");
	succeed_if_same_string (keyName(key), "user/hidden");
	succeed_if (keyGetNameSize(key) == 12, "name size minus slashes" );

	keySetName(key,"user///hidden");
	succeed_if_same_string (keyName(key), "user/hidden");
	succeed_if (keyGetNameSize(key) == 12, "name size minus slashes" );

	keySetName(key,"user////////////////////////////////////hidden");
	succeed_if_same_string (keyName(key), "user/hidden");
	succeed_if (keyGetNameSize(key) == 12, "name size minus slashes" );

	printf("Test trailing Slashes in Key Name\n");
	keySetName(key,"user//hidden/");
	succeed_if_same_string (keyName(key), "user/hidden");
	succeed_if (keyGetNameSize(key) == 12, "name size minus slashes" );

	keySetName(key,"user//hidden//");
	succeed_if_same_string (keyName(key), "user/hidden");
	succeed_if (keyGetNameSize(key) == 12, "name size minus slashes" );

	keySetName(key,"user//hidden///////");
	succeed_if_same_string (keyName(key), "user/hidden");
	succeed_if (keyGetNameSize(key) == 12, "name size minus slashes" );

	keySetName(key,"user/");
	// printf ("Name: %s\n", keyName(key));
	succeed_if_same_string (keyName(key), "user");

	keySetName(key,"user/a");
	succeed_if_same_string (keyName(key), "user/a");

	keySetName(key,"user//");
	succeed_if_same_string (keyName(key), "user");

	keySetName(key,"user/////////");
	succeed_if_same_string (keyName(key), "user");

	printf("Test Dots in Key Name\n");
	keySetName(key,"user/hidden/.");
	succeed_if_same_string (keyName(key), "user/hidden");

	keySetName(key,"user/.");
	succeed_if_same_string (keyName(key), "user");

	keySetName(key,"user/./hidden");
	succeed_if_same_string (keyName(key), "user/hidden");

	keySetName(key,"user/.valid/.");
	succeed_if_same_string (keyName(key), "user/.valid");

	keySetName(key,"user/./.valid");
	succeed_if_same_string (keyName(key), "user/.valid");

	keySetName(key,"user/./.valid/.");
	succeed_if_same_string (keyName(key), "user/.valid");

	keySetName(key,"user/././././.valid/././././.");
	succeed_if_same_string (keyName(key), "user/.valid");

	printf("Test Double Dots in Key Name\n");
	keySetName(key,"user/hidden/parent/..");
	succeed_if_same_string (keyName(key), "user/hidden");

	keySetName(key,"user/hidden/..");
	succeed_if_same_string (keyName(key), "user");

	keySetName(key,"user/..");
	succeed_if_same_string (keyName(key), "user");

	keySetName(key,"user/hidden/../..");
	// printf ("Name: %s\n", keyName(key));
	succeed_if_same_string (keyName(key), "user");

	succeed_if (keySetName(key, "user///sw/../sw//././MyApp")==sizeof("user/sw/MyApp"), "could not set keySet example");
	// printf("%s %d\n", keyName(key), keyGetNameSize(key));
	succeed_if_same_string (keyName(key), "user/sw/MyApp");
	succeed_if (keyGetNameSize(key) == sizeof("user/sw/MyApp"), "incorrect length for keySet example");

	printf("Test Mixed Dots and Slashes in Key Name\n");
	keySetName(key,"user/hidden/../.");

	keyDel (key);


	printf("Test failure key creation\n");

	key = keyNew ("invalid", KEY_END);
	succeed_if (key!=0, "null pointer for invalid name");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name");

	key = keyNew("nonhere/valid/there", KEY_END);
	succeed_if (key != NULL, "keyNew did not accept wrong name");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name");

	key = keyNew("nonhere:y/valid/there", KEY_END);
	succeed_if (key != NULL, "keyNew did not accept wrong name");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name");

	key = keyNew ("user/validname", KEY_END);
	succeed_if(key != NULL, "keyNew: Unable to create a key with name");
	succeed_if_same_string (keyName(key), "user/validname");

	keySetName(key, "invalid");
	succeed_if (keyGetNameSize(key) == 1, "name size for invalid name");
	succeed_if_same_string (keyName(key), "");

	keySetName (key, "user/validname");
	succeed_if_same_string (keyName(key), "user/validname");

	keySetName(key, "");
	succeed_if (keyGetNameSize(key) == 1, "name size for invalid name");
	succeed_if_same_string (keyName(key), "");

	keySetName (key, "user/validname\\/t");
	succeed_if_same_string (keyName(key), "user/validname\\/t");

	keySetName(key, 0);
	succeed_if (keyGetNameSize(key) == 1, "name size for invalid name");
	succeed_if_same_string (keyName(key), "");

#ifdef COMPAT
	keySetName (key, "user/validname\\");
	succeed_if_same_string (keyName(key), "user/validname\\");
#endif

	keySetName (key, "user/validname\\/");
	succeed_if_same_string (keyName(key), "user/validname\\/");
	succeed_if(keyDel(key) == 0, "keyDel: Unable to delete key with name");

	printf("Test key's name manipulation\n");

	Key * copy = keyNew (0);

	for(i = 0 ; tstKeyName[i].testName != NULL ; i++)
	{
		key = keyNew(tstKeyName[i].keyName, KEY_END);

		succeed_if (keyGetRef (copy) == 0, "reference of copy not correct");
		keyCopy (copy, key);
		succeed_if (keyGetRef (copy) == 0, "reference of copy not correct");

		Key *dup = keyDup(key);
		succeed_if (keyGetRef (dup) == 0, "reference of dup not correct");

		compare_key (copy, key);
		compare_key (copy, dup);
		compare_key (dup, key);

		/* keyName */
		succeed_if_same_string(keyName(key), tstKeyName[i].expectedKeyName);
		succeed_if_same_string(keyName(copy), tstKeyName[i].expectedKeyName);
		succeed_if_same_string(keyName(dup), tstKeyName[i].expectedKeyName);

		/* keyBaseName */
		succeed_if_same_string(keyBaseName(key), tstKeyName[i].expectedBaseName);
		succeed_if_same_string(keyBaseName(copy), tstKeyName[i].expectedBaseName);
		succeed_if_same_string(keyBaseName(dup), tstKeyName[i].expectedBaseName);

		/* keyGetBaseNameSize */
		size = keyGetBaseNameSize(key);
		succeed_if( (size == strlen(tstKeyName[i].expectedBaseName)+1), "keyGetBaseNameSize" );

		/* keyGetBaseName */
		size = keyGetBaseNameSize(key)+1;
		buf = malloc(size*sizeof(char));
		keyGetBaseName(key, buf, size);
		succeed_if_same_string(buf, tstKeyName[i].expectedBaseName);
		free(buf);

		/* keyGetNameSize */
		size = keyGetNameSize(key);
		succeed_if( (size == strlen(tstKeyName[i].expectedKeyName)+1), "keyGetKeyNameSize" );
		
		/* keyGetName */
		size = keyGetNameSize(key);
		buf = malloc(size*sizeof(char));
		keyGetName(key, buf, size);
		succeed_if_same_string (buf, tstKeyName[i].expectedKeyName);
		free(buf);

		keyDel(key);
		keyDel(dup);
	}
	keyDel (copy);
}


static void test_keyValue()
{
	Key * key;
	char	ret [1000];
	size_t i;
	char testString[] = "teststring";
	char testBinary[] = "\0tes\1tbinary";
	testBinary[sizeof(testBinary)-1] = 'T';

	printf("Test value of keys\n");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if (keyGetValueSize (key) == 1, "empty value size");
	succeed_if (keySetString (key,"perfectvalue") == 13, "could not set string");
	succeed_if (keyGetValueSize (key) == 13, "value size not correct");
	succeed_if_same_string (keyValue(key), "perfectvalue");
	succeed_if (keySetString (key,"perfectvalue") == 13, "could not re-set same string");
	succeed_if_same_string (keyValue(key), "perfectvalue");
	succeed_if (keySetString (key,"nearperfectvalue") == 17, "could not re-set other string");
	succeed_if (keyGetValueSize (key) == 17, "value size not correct");
	succeed_if_same_string (keyValue(key), "nearperfectvalue");
	succeed_if (keyGetString (key, ret, keyGetValueSize (key)>=999 ? 999 : keyGetValueSize (key))
			== 17, "could not get string");
	succeed_if_same_string (ret, "nearperfectvalue");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if_same_string (keyValue(key), "");
	succeed_if (keyGetValueSize(key) == 1, "Empty value size problem");
	succeed_if (keySetString (key,"") == 1, "could not set empty string");
	succeed_if_same_string (keyValue(key), "");
	succeed_if (keyGetValueSize(key) == 1, "Empty value size problem");
	succeed_if (keyGetString(key, ret, 0) == -1, "Could not get empty value");
	succeed_if (keyGetString(key, ret, 1) == 1, "Could not get empty value");
	succeed_if (ret[0] == 0, "keyGetValue did not return empty value");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if (keySetString (key, "a long long string") == 19, "could not set string");
	succeed_if (keyGetString (key, ret, 6) == -1, "string not truncated");
	succeed_if (keyGetBinary (key, ret, 999) == -1, "binary not mismatch");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if (keySetBinary (key, "a", 1) == 1, "could not set binary");
	succeed_if (keyIsString (key) == 0, "is not a string");
	succeed_if (keyIsBinary (key) == 1, "is not a string");
	succeed_if (keyGetBinary (key, ret, 1) == 1, "binary not truncated");
	succeed_if (!strncmp(ret, "a", 1), "binary value wrong");
	succeed_if (keyGetString (key, ret, 999) == -1, "string not mismatch");
	succeed_if (keySetString (key, 0) == 1, "wrong error code for SetString");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if (keySetBinary (key, NULL, 0) == 0, "could not set null binary");
	succeed_if (keyIsString (key) == 0, "is not a string");
	succeed_if (keyIsBinary (key) == 1, "is not a string");
	succeed_if (keyGetValueSize(key) == 0, "Empty value size problem");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if (keySetString (key, "") == 1, "could not set empty string");
	succeed_if (keyIsString (key) == 1, "is not a string");
	succeed_if (keyIsBinary (key) == 0, "is a binary");
	succeed_if (keyGetValueSize(key) == 1, "Empty value size problem");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	succeed_if (keySetBinary (key, "a long long binary", 19) == 19, "could not set string");
	succeed_if (keyIsString (key) == 0, "is not a string");
	succeed_if (keyIsBinary (key) == 1, "is not a string");
	succeed_if (keyGetBinary (key, ret, 6) == -1, "binary not truncated");
	succeed_if (keyGetBinary (key, ret, 19) == 19, "could not get binary");
	succeed_if (!strncmp(ret, "a long long binary", 19), "binary value wrong");
	succeed_if (keyGetString (key, ret, 999) == -1, "string not mismatch");
	succeed_if (keyDel (key) == 0, "could not delete key");

	succeed_if (key = keyNew(0), "could not create new key");
	for (i=1; i<255; i++)
	{
		ret[0] =  i; ret[1] = i; ret[2] = 0;
		//output_key (key);
		succeed_if (keySetString (key,ret) == 3, "could not set string");
		succeed_if_same_string (keyValue(key), ret);
	}
	succeed_if (keyDel (key) == 0, "could not delete key");


	succeed_if (key = keyNew(0), "could not create new key");
	for (i=0; i<255; i++)
	{
		ret[0] =  i; ret[1] = 255-i; ret[2] = i;
		//output_key (key);
		succeed_if (keySetBinary(key,ret,3) == 3, "could not set string");
		succeed_if (memcmp (keyValue(key), ret, 3) == 0, "String not same as set");
	}
	succeed_if (keyDel (key) == 0, "could not delete key");



	printf ("Test string of key\n");

	succeed_if (keyValue(0) == 0, "null pointer");
	succeed_if (keyGetValueSize(0) == -1, "null pointer");
	succeed_if (keySetString(0,"") == -1, "null pointer");

	key = keyNew(0);
	succeed_if (keyGetValueSize(key) == 1, "empty value size");

	keySetString (key, testString);
	succeed_if (keyGetString (0,ret,100) == -1, "null pointer");
	succeed_if (keyGetString (key,0,100) == -1, "string null pointer");
	succeed_if (keyGetString (key,ret,0) == -1, "length checking");

	for (i=1; i< sizeof(testString);i++)
	{
		succeed_if (keyGetString (key,ret,i) == -1, "length checking too short");
	}
	for (i=sizeof(testString); i<sizeof(testString)*2; i++)
	{
		succeed_if (keyGetString (key,ret,i) == sizeof(testString), "length checking longer");
	}
	succeed_if (keyGetString (key,ret, (size_t)-1) == -1, "maxSize exceeded");

	succeed_if (keySetString(key,0) == 1, "delete string");
	succeed_if (keyGetString (key,ret,i) == 1, "length checking deleting");
	succeed_if_same_string (ret, "");

	succeed_if (keySetString(key,testString) == sizeof(testString), "set string");
	succeed_if (keyGetString (key,ret,i) == sizeof(testString), "length checking working");
	succeed_if_same_string (ret, testString);

	succeed_if (keySetString(key,"") == 1, "delete string");
	succeed_if (keyGetString (key,ret,i) == 1, "length checking deleting");
	succeed_if_same_string (ret, "");

	succeed_if (keySetString(key,testString) == sizeof(testString), "set string");
	succeed_if (keyGetString (key,ret,i) == sizeof(testString), "length checking working");
	succeed_if_same_string (ret, testString);

	succeed_if (keyGetValueSize(key) == sizeof(testString), "testString value size");
	succeed_if (strncmp(keyValue(key), testString, sizeof(testString)) == 0, "testString not same");
	keyDel (key);



	printf ("Test binary of key\n");

	succeed_if (keyValue(0) == 0, "null pointer");
	succeed_if (keyGetValueSize(0) == -1, "null pointer");
	succeed_if (keySetBinary(0,"",1) == -1, "null pointer");

	key = keyNew(0);
	succeed_if (keySetBinary(key,"",0) == -1, "null size");
	succeed_if (keySetBinary(key,"b",0) == -1, "null size");
	succeed_if (keySetBinary(key,"",SIZE_MAX) == -1, "max size");
	succeed_if (keySetBinary(key,"b",SIZE_MAX) == -1, "max size");
	succeed_if (keyGetValueSize(key) == 1, "empty value size");

	keySetBinary (key, testBinary, sizeof(testBinary));
	succeed_if (keyGetBinary (0,ret,100) == -1, "null pointer");
	succeed_if (keyGetBinary (key,0,100) == -1, "binary null pointer");
	succeed_if (keyGetBinary (key,ret,0) == -1, "length checking");

	for (i=1; i< sizeof(testBinary);i++)
	{
		succeed_if (keyGetBinary (key,ret,i) == -1, "length checking too short");
	}
	for (i=sizeof(testBinary); i<sizeof(testBinary)*2; i++)
	{
		succeed_if (keyGetBinary (key,ret,i) == sizeof(testBinary), "length checking longer");
	}
	succeed_if (keyGetBinary (key,ret, (size_t)-1) == -1, "maxSize exceeded");

	succeed_if (keySetBinary(key,0,0) == 0, "delete binary");
	succeed_if (keyGetBinary (key,ret,i) == 0, "length checking deleting");

	succeed_if (keySetBinary(key,testBinary, sizeof(testBinary)) == sizeof(testBinary), "set binary");
	succeed_if (keyGetBinary (key,ret,i) == sizeof(testBinary), "length checking working");
	succeed_if_same_string (ret, testBinary);

	succeed_if (keySetBinary(key,0,1) == 0, "delete binary");
	succeed_if (keyGetBinary (key,ret,i) == 0, "length checking deleting");

	succeed_if (keySetBinary(key,testBinary, sizeof(testBinary)) == sizeof(testBinary), "set binary");
	succeed_if (keyGetBinary (key,ret,i) == sizeof(testBinary), "length checking working");
	succeed_if_same_string (ret, testBinary);

	succeed_if (keySetBinary(key,"",1) == 1, "delete binary the string way");
	succeed_if (keyGetBinary (key,ret,i) == 1, "length checking deleting string way");
	succeed_if_same_string (ret, "");

	succeed_if (keySetBinary(key,testBinary, sizeof(testBinary)) == sizeof(testBinary), "set binary");
	succeed_if (keyGetBinary (key,ret,i) == sizeof(testBinary), "length checking working");
	succeed_if_same_string (ret, testBinary);

	succeed_if (keyGetValueSize(key) == sizeof(testBinary), "testBinary value size");
	succeed_if (strncmp(keyValue(key), testBinary, sizeof(testBinary)) == 0, "testBinary not same");
	keyDel (key);

}

static void test_keyBinary(void)
{
	Key *key =0;
	char ret [1000];
	int i;
	char binaryData[] = "\0binary \1\34data";
	binaryData[sizeof(binaryData)-1] = 'T';

	printf ("Test binary special cases\n");

	key = keyNew ("user/binary",
		KEY_BINARY,
		KEY_SIZE, sizeof(binaryData),
		KEY_VALUE, binaryData,
		KEY_END);

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == sizeof(binaryData), "size not correct");
	succeed_if (memcmp(binaryData, keyValue(key), sizeof(binaryData)) == 0, "memcmp");
	succeed_if (keyGetBinary(key, ret, 1000) == sizeof(binaryData), "could not get binary data");
	succeed_if (memcmp(binaryData, ret, sizeof(binaryData)) == 0, "memcmp");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	keyDel (key);

	key = keyNew(0);
	keySetBinary (key, binaryData, sizeof(binaryData));

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == sizeof(binaryData), "size not correct");
	succeed_if (memcmp(binaryData, keyValue(key), sizeof(binaryData)) == 0, "memcmp");
	succeed_if (keyGetBinary(key, ret, 1000) == sizeof(binaryData), "could not get binary data");
	succeed_if (memcmp(binaryData, ret, sizeof(binaryData)) == 0, "memcmp");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	keyDel (key);

	key = keyNew(0);
	keySetBinary(key, 0, 0);

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == 0, "size not correct");
	succeed_if (keyValue(key) == 0, "should be null pointer");
	succeed_if (keyGetBinary(key, ret, 1000) == 0, "should write nothing because of no data");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	keyDel (key);

	key = keyNew(0);
	keySetBinary(key, 0, 1);

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == 0, "size not correct");
	succeed_if (keyValue(key) == 0, "should be null pointer");
	succeed_if (keyGetBinary(key, ret, 1000) == 0, "should write nothing because of no data");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	keyDel (key);

	key = keyNew(0);
	keySetBinary(key, "", 1);
	succeed_if (keySetBinary(key, 0, SIZE_MAX) == -1, "should do nothing and fail");
	succeed_if (keySetBinary(key, 0, SSIZE_MAX) == 0, "should free data");

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == 0, "size not correct");
	succeed_if (keyValue(key) == 0, "should be null pointer");
	succeed_if (keyGetBinary(key, ret, 1000) == 0, "should write nothing because of no data");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	keyDel (key);

	key = keyNew(0);
	keySetBinary(key, "", 1);

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == 1, "size not correct");
	succeed_if (memcmp(binaryData, keyValue(key), 1) == 0, "memcmp");
	succeed_if (keyGetBinary(key, ret, 1000) == 1, "could not get binary data");
	succeed_if (memcmp(binaryData, ret, 1) == 0, "memcmp");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	keyDel (key);

	key = keyNew(0);
	i = 23;
	keySetBinary (key, (void*)&i, sizeof(i));

	succeed_if (keyIsBinary(key) == 1, "should be binary");
	succeed_if (keyIsString(key) == 0, "should not be string");
	succeed_if (keyGetValueSize(key) == sizeof(i), "size not correct");
	succeed_if (memcmp((void*)&i, keyValue(key), sizeof(i)) == 0, "memcmp");
	succeed_if (keyGetBinary(key, ret, 1000) == sizeof(i), "could not get binary data");
	succeed_if (memcmp((void*)&i, ret, sizeof(i)) == 0, "memcmp");
	succeed_if (keyGetString(key, ret, 1000) == -1, "should be type mismatch");

	i = *(int*)keyValue(key);
	succeed_if (i==23, "incorrect int");

	keyDel (key);

}

static void test_keyInactive ()
{
	Key * key = keyNew(0);

	succeed_if (keyIsInactive(0) == -1, "NULL pointer");
	succeed_if (keyIsInactive(key) == -1, "Key has no name");

	printf("Test of active and inactive keys\n");
	keySetName(key,"user/valid");
	succeed_if (!keyIsInactive(key), "Key should not be inactive");

	keySetName(key,"user/.hidden/valid");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.hidden/€valid");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/..hidden/valid");
	succeed_if (keyIsInactive(key), "Key should be inactive");

	keySetName(key,"user/hidden");
	succeed_if (!keyIsInactive(key), "Key should not be inactive");
	
	keySetName(key,"user/.hidden");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.valid/.hidden");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.valid/.:hidden");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.valid/.€hidden");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.HiddenStringKey");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.HiddenDirectoryKey");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.HiddenDirectoryKey/StringKey");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/tests/file8xdLVS/filesys/.HiddenStringKey");
	succeed_if (keyIsInactive(key), "Key should be inactive");
	
	keySetName(key,"user/.hidden/nothidden/hierarchy");
	succeed_if (keyIsInactive(key) == 1, "Key should be inactive");
	
	keySetName(key,"user/.hidden/nothidden/hierarchy");
	succeed_if (keyIsInactive(key) == 1, "Key should be inactive");
	keyDel (key);
}

static void test_keyBelow()
{
	Key * key1 = keyNew(0);
	Key * key2 = keyNew(0);

	printf("Test of relative positions of keys\n");

	succeed_if (keyIsBelow(key1,0) == -1, "NULL pointer");
	succeed_if (keyIsBelow(0,0) == -1, "NULL pointer");
	succeed_if (keyIsBelow(0,key1) == -1, "NULL pointer");

	succeed_if (keyIsDirectBelow(key1,0) == -1, "NULL pointer");
	succeed_if (keyIsDirectBelow(0,0) == -1, "NULL pointer");
	succeed_if (keyIsDirectBelow(0,key1) == -1, "NULL pointer");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid");
	succeed_if (!keyIsBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/");
	succeed_if (!keyIsBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/below");
	succeed_if (keyIsBelow(key1,key2), "Key should be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/b");
	succeed_if (keyIsBelow(key1,key2), "Key should be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/b");
	succeed_if (keyIsBelow(key1,key2), "Key should be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/b/e");
	succeed_if (keyIsBelow(key1,key2), "Key should be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valide");
	keySetName(key2,"user/valid/e");
	succeed_if (!keyIsBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid/b");
	keySetName(key2,"user/valid/e");
	succeed_if (!keyIsBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valide");
	keySetName(key2,"user/valid/valide");
	succeed_if (!keyIsBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/valide");
	succeed_if (keyIsDirectBelow(key1,key2), "Key should be below");
	succeed_if (!keyIsDirectBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/non/valid");
	succeed_if (!keyIsDirectBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsDirectBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid/a");
	keySetName(key2,"user/valid/b");
	succeed_if (!keyIsDirectBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsDirectBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid");
	succeed_if (!keyIsDirectBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsDirectBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid/a/b");
	keySetName(key2,"user/valid");
	succeed_if (!keyIsDirectBelow(key1,key2), "Key should not be below");
	succeed_if (!keyIsDirectBelow(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid/a");
	keySetName(key2,"user/valid");
	succeed_if (!keyIsDirectBelow(key1,key2), "Key should not be below");
	succeed_if (keyIsDirectBelow(key2,key1), "Key should be below");


	keyDel (key1);
	keyDel (key2);
}

static void test_keyDup()
{
	Key     *orig, *copy;

	printf("Test key duplication\n");

	// Create test key
	orig = keyNew("user:yl/foo/bar",
			KEY_BINARY,
			KEY_SIZE, 6,
			KEY_VALUE, "foobar",
			KEY_COMMENT, "mycomment", 
			KEY_UID, 123,
			KEY_GID, 456,
			KEY_MODE, 0644,
			KEY_END);



	// Dup the key
	succeed_if( (copy = keyDup(orig)) != 0, "keyDup failed");
	compare_key(orig, copy);
	keyDel(orig); // everything independent from original!

	// Check the duplication
	succeed_if_same_string (keyName(copy), "user/foo/bar");
	succeed_if( strncmp(keyValue(copy), "foobar", 6) == 0, "keyDup: key value copy error");
	succeed_if (keyIsBinary(copy), "keyDup: key type copy error");

	keyDel(copy);

	orig = keyNew (0);
	keySetName (orig, "invalid");

	succeed_if ( (copy = keyDup(orig)) != 0, "keyDup failed");
	succeed_if_same_string (keyName(orig), "");
	succeed_if_same_string (keyName(copy), "");
	succeed_if (keyGetNameSize(orig) == 1, "orig name size");
	succeed_if (keyGetNameSize(copy) == 1, "orig name size");
	succeed_if (keyGetRef(orig) == 0, "orig ref counter should be 0");
	succeed_if (keyGetRef(copy) == 0, "copy ref counter should be 0");

	keyDel (orig);
	keyDel (copy);
}

static void test_keyCopy()
{
	Key     *orig, *copy;

	printf("Test key copy\n");

	// Create test key
	orig = keyNew("user:yl/foo/bar",
			KEY_BINARY,
			KEY_SIZE, 6,
			KEY_VALUE, "foobar",
			KEY_COMMENT, "mycomment", 
			KEY_UID, 123,
			KEY_GID, 456,
			KEY_MODE, 0644,
			KEY_END);



	// Copy the key
	copy = keyNew(0);
	succeed_if( keyCopy(copy, orig) == 1, "keyCopy failed");
	succeed_if (keyGetRef(orig) == 0, "orig ref counter should be 0");
	succeed_if (keyGetRef(copy) == 0, "copy ref counter should be 0");
	compare_key (orig, copy);
	keyDel(orig); // everything independent from original!

	// Check the duplication
	succeed_if_same_string (keyName(copy), "user/foo/bar");
	succeed_if( strncmp(keyValue(copy), "foobar", 6) == 0, "keyCopy: key value copy error");

	orig = keyNew(0);
	succeed_if (keyCopy(copy, 0) == 0, "make the key copy fresh");

	compare_key (orig, copy);
	keyDel (orig);

	keyDel(copy);

	orig = keyNew (0);
	keySetName (orig, "invalid");

	copy = keyNew(0);
	succeed_if( keyCopy(copy, orig) == 1, "keyCopy failed");
	succeed_if_same_string (keyName(orig), "");
	succeed_if_same_string (keyName(copy), "");
	succeed_if (keyGetNameSize(orig) == 1, "orig name size");
	succeed_if (keyGetNameSize(copy) == 1, "orig name size");
	succeed_if (keyGetRef(orig) == 0, "orig ref counter should be 0");
	succeed_if (keyGetRef(copy) == 0, "copy ref counter should be 0");

	keyDel (orig);
	keyDel (copy);

	orig = keyNew("user/orig", KEY_END);
	succeed_if(keyNeedSync(orig), "fresh key does not need sync?");
	KeySet *ks = ksNew(20, KS_END);
	ksAppendKey(ks, orig);
	copy = keyNew("user/othername", KEY_END);
	succeed_if(keyNeedSync(copy), "fresh key does not need sync?");
	succeed_if (keyGetRef(orig) == 1, "orig ref counter should be 1");
	succeed_if (keyGetRef(copy) == 0, "copy ref counter should be 0");
	succeed_if (keyCopy(orig, copy) == -1, "copy should not be allowed when key is already referred to");
	succeed_if(keyNeedSync(orig), "copied key does not need sync?");
	succeed_if(keyNeedSync(copy), "copied key does not need sync?");

	succeed_if (keyGetRef(orig) == 1, "orig ref counter should be 1");
	succeed_if (keyGetRef(copy) == 0, "copy ref counter should be 1");

	succeed_if_same_string(keyName(orig), "user/orig");
	succeed_if_same_string(keyName(copy), "user/othername");

	keyDel(copy);
	ksRewind(ks);
	succeed_if_same_string(keyName(ksNext(ks)), "user/orig");
	ksDel(ks);
}


typedef void (*fun_t) ();
static void fun()
{}

static void test_binary()
{
	printf ("Test binary values\n");

	Key *k = 0;

	int i = 20;
	int *p = &i;

	k = keyNew(0);
	succeed_if (keySetBinary(k, &p, sizeof(p)) == sizeof(p),
			"could not set binary");

	int *q;
	succeed_if (keyGetBinary(k, &q, sizeof(q)) == sizeof(q),
			"could not get binary");
	succeed_if (p == q, "pointers to int are not equal");
	succeed_if (*p == *q, "values are not equal");
	succeed_if (*q == 20, "values are not equal");
	succeed_if (*p == 20, "values are not equal");

	keyDel (k);



	union {void (*f)(); void* v;} conversation;

	k = keyNew(0);
	conversation.f = fun;
	succeed_if (keySetBinary(k, &conversation.v, sizeof(conversation)) == sizeof(conversation),
			"could not set binary");

	conversation.v = 0;
	conversation.f = 0;
	void (*g) () = 0;
	succeed_if (keyGetBinary(k, &conversation.v, sizeof(conversation)) == sizeof(conversation),
			"could not get binary");
	g = conversation.f;
	succeed_if (g == fun, "pointers to functions are not equal");

	keyDel (k);



	conversation.f = fun;
	k = keyNew("system/symbols/fun",
			KEY_BINARY,
			KEY_SIZE, sizeof (conversation),
			KEY_VALUE, &conversation.v,
			KEY_END);

	conversation.v = 0;
	conversation.f = 0;
	succeed_if (keyGetBinary(k, &conversation.v, sizeof(conversation)) == sizeof(conversation),
			"could not get binary");
	g = conversation.f;
	succeed_if (g == fun, "pointers to functions are not equal");

	keyDel (k);




	fun_t tmp = fun;

	k = keyNew ("system/symbol/fun",
			KEY_BINARY,
			KEY_SIZE, sizeof (fun_t),
			KEY_VALUE, &tmp,
			KEY_END);

	fun_t myfun = 0;
	succeed_if (keyGetBinary(k, &myfun, sizeof(fun_t)) == sizeof(fun_t),
			"could not get binary");

	succeed_if (fun == myfun, "pointers not equal");

	keyDel (k);


	k = keyNew ("system/symbol/cool",
			KEY_FUNC, fun,
			KEY_END);

	succeed_if (keyGetBinary(k, &myfun, sizeof(fun_t)) == sizeof(fun_t),
			"could not get binary");

	succeed_if (fun == myfun, "pointers not equal");

	keyDel (k);

	char data[10];
	k = keyNew ("system/empty_binary", KEY_END);
	succeed_if (keySetBinary(k, 0, 0) == 0, "could not set binary will null pointer");
	succeed_if (keyIsBinary(k), "key is not binary");
	succeed_if (keyGetBinary(k, data, 1) == 0, "could not get empty binary");
	succeed_if (keyValue(k) == 0, "did not get back null pointer");

	succeed_if (keySetBinary(k, 0, 1) == 0, "could not set binary will null pointer");
	succeed_if (keyIsBinary(k), "key is not binary");
	succeed_if (keyGetBinary(k, data, 1) == 0, "could not get empty binary");
	succeed_if (keyValue(k) == 0, "did not get back null pointer");

	succeed_if (keySetBinary(k, 0, 5) == 0, "could not set binary will null pointer");
	succeed_if (keyIsBinary(k), "key is not binary");
	succeed_if (keyGetBinary(k, data, 1) == 0, "could not get empty binary");
	succeed_if (keyValue(k) == 0, "did not get back null pointer");

	succeed_if (keySetBinary(k, 0, -1) == -1, "misusage: this will fail");
	succeed_if (keyIsBinary(k), "key is not binary (should be from previous calls)");
	succeed_if (keyGetBinary(k, data, 1) == 0, "could not get empty binary");
	succeed_if (keyValue(k) == 0, "did not get back null pointer");
	keyDel (k);
}

static void test_keyBelowOrSame()
{
	Key * key1 = keyNew(0);
	Key * key2 = keyNew(0);

	printf("Test of keyBelowOrSame\n");

	succeed_if (keyIsBelowOrSame(key1,0) == -1, "NULL pointer");
	succeed_if (keyIsBelowOrSame(0,0) == -1, "NULL pointer");
	succeed_if (keyIsBelowOrSame(0,key1) == -1, "NULL pointer");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid");
	succeed_if (keyIsBelowOrSame(key1,key2), "Key should be below");
	succeed_if (keyIsBelowOrSame(key2,key1), "Key should be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/");
	succeed_if (keyIsBelowOrSame(key1,key2), "Key should be below");
	succeed_if (keyIsBelowOrSame(key2,key1), "Key should be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/below");
	succeed_if (keyIsBelowOrSame(key1,key2), "Key should be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/b");
	succeed_if (keyIsBelowOrSame(key1,key2), "Key should be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/b");
	succeed_if (keyIsBelowOrSame(key1,key2), "Key should be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid");
	keySetName(key2,"user/valid/b/e");
	succeed_if (keyIsBelowOrSame(key1,key2), "Key should be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/valide");
	keySetName(key2,"user/valid/e");
	succeed_if (!keyIsBelowOrSame(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/valid/b");
	keySetName(key2,"user/valid/e");
	succeed_if (!keyIsBelowOrSame(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/export/a");
	keySetName(key2,"user/export-backup/b");
	succeed_if (!keyIsBelowOrSame(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keySetName(key1,"user/export");
	keySetName(key2,"user/export-backup-2/x");
	succeed_if (!keyIsBelowOrSame(key1,key2), "Key should not be below");
	succeed_if (!keyIsBelowOrSame(key2,key1), "Key should not be below");

	keyDel (key1);
	keyDel (key2);
}

static void test_keyNameSpecial()
{
	printf ("Test special keynames");
	Key *k = keyNew (0);
	succeed_if_same_string (keyName(k), "");

	succeed_if (keySetName (k, "system"), "could not set key name with system");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, 0) == 0, "could not set key name with 0");
	succeed_if_same_string (keyName(k), "");

	succeed_if (keySetName (k, "system"), "could not set key name with system");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "") == 0, "could not set key name with empty string");
	succeed_if_same_string (keyName(k), "");

	succeed_if (keySetName (k, "system"), "could not set key name with system");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "invalid") == -1, "could not set key name invalid");
	succeed_if_same_string (keyName(k), "");



	succeed_if (keySetName (k, "system/something/.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/something/.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/something/../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/something/../../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/something/../../../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/something/../../../../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");



	succeed_if (keySetName (k, "system/../something"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/something");

	succeed_if (keySetName (k, "system/../../something"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/something");

	succeed_if (keySetName (k, "system/../../../something"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/something");

	succeed_if (keySetName (k, "system/../../../../something"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/something");

	succeed_if (keySetName (k, "system/../../../../../something"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/something");



	succeed_if (keySetName (k, "system/a/b/c/.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a/b");

	succeed_if (keySetName (k, "system/a/b/c/../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a");

	succeed_if (keySetName (k, "system/a/b/c/../../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/a/b/c/../../../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");

	succeed_if (keySetName (k, "system/a/b/c/../../../../.."), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system");



	succeed_if (keySetName (k, "system/../a/b/c"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a/b/c");

	succeed_if (keySetName (k, "system/../../a/b/c"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a/b/c");

	succeed_if (keySetName (k, "system/../../../a/b/c"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a/b/c");

	succeed_if (keySetName (k, "system/../../../../a/b/c"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a/b/c");

	succeed_if (keySetName (k, "system/../../../../../a/b/c"), "could not set key name with ..");
	succeed_if_same_string (keyName(k), "system/a/b/c");


	keyDel (k);
}

static void test_keyClear()
{
	printf ("Test clear of key\n");

	Key *k1 = keyNew("system/abc", KEY_END);
	succeed_if_same_string (keyName(k1), "system/abc");

	succeed_if (keyGetRef(k1) == 0, "New key reference");
	keyIncRef(k1);
	succeed_if (keyGetRef(k1) == 1, "Incremented key reference");
	Key *k2 = k1; // create an alias for k1
	succeed_if_same_string (keyName(k2), "system/abc");
	succeed_if (keyGetRef(k1) == 1, "Incremented key reference");
	succeed_if (keyGetRef(k2) == 1, "Incremented key reference");

	keyIncRef(k1);
	Key *k3 = k1; // create an alias for k1
	succeed_if_same_string (keyName(k3), "system/abc");
	succeed_if (keyGetRef(k1) == 2, "Incremented key reference");
	succeed_if (keyGetRef(k2) == 2, "Incremented key reference");
	succeed_if (keyGetRef(k3) == 2, "Incremented key reference");

	keyClear(k1);
	succeed_if_same_string (keyName(k1), "");
	succeed_if_same_string (keyName(k2), "");
	succeed_if_same_string (keyName(k3), "");

	keySetMeta(k1, "test_meta", "test_value");
	succeed_if_same_string (keyValue(keyGetMeta(k1, "test_meta")), "test_value");
	succeed_if_same_string (keyValue(keyGetMeta(k2, "test_meta")), "test_value");
	succeed_if_same_string (keyValue(keyGetMeta(k3, "test_meta")), "test_value");

	keyClear(k2);
	succeed_if (keyGetMeta(k1, "test_meta") == 0, "there should be no meta after keyClear");
	succeed_if (keyGetMeta(k2, "test_meta") == 0, "there should be no meta after keyClear");
	succeed_if (keyGetMeta(k3, "test_meta") == 0, "there should be no meta after keyClear");

	keySetString(k1, "mystring");
	succeed_if_same_string (keyValue(k1), "mystring");
	succeed_if_same_string (keyValue(k2), "mystring");
	succeed_if_same_string (keyValue(k3), "mystring");

	keyClear(k3);
	succeed_if_same_string (keyValue(k1), "");
	succeed_if_same_string (keyValue(k2), "");
	succeed_if_same_string (keyValue(k3), "");

	succeed_if (keyGetRef(k1) == 2, "Incremented key reference");
	succeed_if (keyGetRef(k2) == 2, "Incremented key reference");
	succeed_if (keyGetRef(k3) == 2, "Incremented key reference");

	keyDel(k3); // does nothing
	keyDecRef(k3);
	k3 = 0; // remove alias
	succeed_if (keyGetRef(k1) == 1, "Incremented key reference");
	succeed_if (keyGetRef(k2) == 1, "Incremented key reference");

	keyDel(k2); // does nothing
	keyDecRef(k2);
	k2 = 0; // remove alias
	succeed_if (keyGetRef(k1) == 0, "Incremented key reference");

	keyDel(k1);
}

static void test_keyBaseName()
{
	printf ("Test basename\n");
	Key *k = keyNew("user:yl///foo\\///bar\\/foo_bar\\/", KEY_END);
	succeed_if_same_string(keyName(k), "user/foo\\//bar\\/foo_bar\\/");
	succeed_if_same_string(keyBaseName(k), "bar/foo_bar/");

	keySetName (k, "system");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "system/valid//////");
	succeed_if_same_string(keyName(k), "system/valid");
	succeed_if_same_string(keyBaseName(k), "valid");

	keySetName (k, "system//////valid//////");
	succeed_if_same_string(keyName(k), "system/valid");
	succeed_if_same_string(keyBaseName(k), "valid");

	keySetName (k, "system///.///valid//.////");
	succeed_if_same_string(keyName(k), "system/valid");
	succeed_if_same_string(keyBaseName(k), "valid");

	keySetName (k, "user");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "user/valid//////");
	succeed_if_same_string(keyName(k), "user/valid");
	succeed_if_same_string(keyBaseName(k), "valid");

	keySetName (k, "user//////valid//////");
	succeed_if_same_string(keyName(k), "user/valid");
	succeed_if_same_string(keyBaseName(k), "valid");

	keySetName (k, "user///.///valid//.////");
	succeed_if_same_string(keyName(k), "user/valid");
	succeed_if_same_string(keyBaseName(k), "valid");


	keySetName(k, "user:yl///foo\\///bar\\/foo_bar\\/");
	succeed_if_same_string(keyName(k), "user/foo\\//bar\\/foo_bar\\/");
	succeed_if_same_string(keyBaseName(k), "bar/foo_bar/");

	keySetName(k, "user//////foo_bar\\/");
	succeed_if_same_string(keyName(k), "user/foo_bar\\/");
	succeed_if_same_string(keyBaseName(k), "foo_bar/");

	keySetName(k, "user//////%");
	succeed_if_same_string(keyName(k), "user/%");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName(k, "user//////\\%");
	succeed_if_same_string(keyName(k), "user/\\%");
	succeed_if_same_string(keyBaseName(k), "%");

	keySetName(k, "user//////\\\\%");
	succeed_if_same_string(keyName(k), "user/\\\\%");
	succeed_if_same_string(keyBaseName(k), "\\%");

	keySetName(k, "user//////\\\\\\%");
	succeed_if_same_string(keyName(k), "user/\\\\\\%");
	succeed_if_same_string(keyBaseName(k), "\\\\%");
	keyDel (k);
}

static void test_keySetBaseName()
{
	printf ("Test set basename\n");

	Key *k = keyNew (0);

	succeed_if (keySetBaseName (k, "abc") == -1, "invalid key");
	succeed_if_same_string(keyName(k), "");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "system");
	succeed_if (keySetBaseName (k, 0) == -1, "could remove root name");
	succeed_if_same_string(keyName(k), "system");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "user");
	succeed_if (keySetBaseName (k, 0) == -1, "could remove root name");
	succeed_if_same_string(keyName(k), "user");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, 0) == sizeof("system"), "could not remove base name");
	succeed_if_same_string(keyName(k), "system");
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "") > 0, "could not set empty name");
	succeed_if_same_string(keyName(k), "system/%"); // is an empty name
	succeed_if_same_string(keyBaseName(k), "");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "/") >= 0, "escaped slash ok");
	succeed_if_same_string(keyName(k), "system/\\/");
	succeed_if_same_string(keyBaseName(k), "/");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\/") >= 0, "escaped slash ok");
	succeed_if_same_string(keyName(k), "system/\\\\/");
	succeed_if_same_string(keyBaseName(k), "\\/");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\\\/") >= 0, "backslash escaped, but slash unescaped");
	succeed_if_same_string(keyName(k), "system/\\\\\\/");
	succeed_if_same_string(keyBaseName(k), "\\\\/");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\\\\\/") >= 0, "backslash escaped, slash escaped");
	succeed_if_same_string(keyName(k), "system/\\\\\\\\/");
	succeed_if_same_string(keyBaseName(k), "\\\\\\/");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "%") == sizeof("system/\\%"), "could not set basename");
	succeed_if_same_string(keyBaseName(k), "%");
	succeed_if_same_string(keyName(k), "system/\\%");
	succeed_if_same_string(keyBaseName(k), "%");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, ".") == sizeof("system/\\%"), "could not set basename");
	succeed_if_same_string(keyBaseName(k), ".");
	succeed_if_same_string(keyName(k), "system/\\.");
	succeed_if_same_string(keyBaseName(k), ".");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "..") == sizeof("system/\\.."), "could not set basename");
	succeed_if_same_string(keyBaseName(k), "..");
	succeed_if_same_string(keyName(k), "system/\\..");
	succeed_if_same_string(keyBaseName(k), "..");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\\\\\\\") >= 0, "backslash escaped, backslash escaped");
	succeed_if_same_string(keyName(k), "system/\\\\\\\\");
	succeed_if_same_string(keyBaseName(k), "\\\\\\\\");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\\\") >= 0, "escaped backslash ok");
	succeed_if_same_string (keyName (k), "system/\\\\");
	succeed_if_same_string(keyBaseName(k), "\\\\");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\.") >= 0, "escaped dot");
	succeed_if_same_string (keyName (k), "system/\\\\.");
	succeed_if_same_string(keyBaseName(k), "\\.");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "\\..") >= 0, "escaped dot-dot");
	succeed_if_same_string(keyName (k), "system/\\\\..");
	succeed_if_same_string(keyBaseName(k), "\\..");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "%") == sizeof("system/\\%"), "add some char");
	succeed_if_same_string(keyName(k), "system/\\%");
	succeed_if_same_string(keyBaseName(k), "%");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "#1") == sizeof("system/#1"), "valid array entry");
	succeed_if_same_string (keyName (k), "system/#1");
	succeed_if_same_string(keyBaseName(k), "#1");

	keySetName (k, "system/valid");
	succeed_if (keySetBaseName (k, "#_10") >= 0, "valid array entry");
	succeed_if_same_string(keyName (k), "system/#_10");
	succeed_if_same_string(keyBaseName(k), "#_10");

	keySetName(k, "user/tests/yajl/___empty_map");
	succeed_if_same_string(keyBaseName(k), "___empty_map");
	keySetBaseName(k, "#0");
	succeed_if_same_string(keyBaseName(k), "#0");

	keySetBaseName(k, "nullkey");
	succeed_if_same_string(keyBaseName(k), "nullkey");
	succeed_if_same_string(keyName(k), "user/tests/yajl/nullkey");

	//! [base1]
	keySetName (k, "system/valid");
	keySetBaseName(k, ".hiddenkey");
	succeed_if_same_string(keyName(k), "system/.hiddenkey");
	succeed_if_same_string(keyBaseName(k), ".hiddenkey");
	//! [base1]

	//! [base2]
	keySetName (k, "system/valid");
	keySetBaseName(k, "");
	succeed_if_same_string(keyName(k), "system/%");
	succeed_if_same_string(keyBaseName(k), "");
	//! [base2]

	//! [base3]
	keySetName (k, "system/valid");
	keySetBaseName(k, "%");
	succeed_if_same_string(keyName(k), "system/\\%");
	succeed_if_same_string(keyBaseName(k), "%");
	//! [base3]



	keyDel (k);
}

static void test_keyAddBaseName()
{
	printf ("Test add basename\n");

	Key *k = keyNew (0);

//![base0 empty]
keySetName(k,"");
succeed_if_same_string(keyBaseName(k), "");
keySetName(k,"user");
succeed_if_same_string(keyBaseName(k), "");
//![base0 empty]

//![base1 empty]
keySetName (k, "system/valid");
succeed_if (keyAddBaseName (k, "") >= 0, "could not add a base name");
succeed_if_same_string(keyName(k), "system/valid/%");
succeed_if_same_string(keyBaseName(k), "");
//![base1 empty]

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "%") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/\\%");
	succeed_if_same_string(keyBaseName(k), "%");

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "#") == sizeof("system/valid/#"), "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/#");
	succeed_if_same_string(keyBaseName(k), "#");

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "#_2") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/#_2");
	succeed_if_same_string(keyBaseName(k), "#_2");

//![base1 add]
keySetName (k, "system/valid");
succeed_if (keyAddBaseName (k, ".") >= 0, "could not add a base name");
succeed_if_same_string(keyName(k), "system/valid/\\.");
succeed_if_same_string(keyBaseName(k), ".");
//![base1 add]

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "..") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/\\..");
	succeed_if_same_string(keyBaseName(k), "..");


	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "hello%#") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/hello%#");
	succeed_if_same_string(keyBaseName(k), "hello%#");

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "hello..") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/hello..");
	succeed_if_same_string(keyBaseName(k), "hello..");

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "..hello..") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/..hello..");
	succeed_if_same_string(keyBaseName(k), "..hello..");

	keySetName (k, "system/valid");
	succeed_if (keyAddBaseName (k, "has/slash") >= 0, "could not add a base name");
	succeed_if_same_string(keyName(k), "system/valid/has\\/slash");
	succeed_if_same_string(keyBaseName(k), "has/slash");

	keySetName (k, "system/valid");
	keyAddBaseName(k, "#0");
	succeed_if_same_string(keyName(k), "system/valid/#0");
	succeed_if_same_string(keyBaseName(k), "#0");

	keyDel (k);
}

static void test_keyDirectBelow()
{
	printf ("Test direct below check\n");

	Key *k1 = keyNew("/dir", KEY_CASCADING_NAME, KEY_END);
	Key *k2 = keyNew("/dir/directbelow", KEY_CASCADING_NAME, KEY_END);
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/directbelow");
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\/below");
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\/");
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\");
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\\\\\/below");
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\\\below");
	succeed_if(keyIsBelow(k1, k2) == 1, "below");
	succeed_if(keyIsDirectBelow(k1, k2) == 1, "not direct below");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\\\/b");
	succeed_if_same_string(keyName(k2), "user/dir/direct\\\\/b");
	succeed_if(keyIsBelow(k1, k2) == 1, "below");
	succeed_if(keyIsDirectBelow(k1, k2) == 0, "direct below, but shouldnt be");

	keySetName(k1, "user/dir");
	keySetName(k2, "user/dir/direct\\\\/below");
	succeed_if(keyIsBelow(k1, k2) == 1, "below");
	succeed_if(keyIsDirectBelow(k1, k2) == 0, "direct below, but shouldnt be");

	keyDel(k1);
	keyDel(k2);
}

#define TEST_ESCAPE_PART(A, S) \
	do { \
	succeed_if(keySetBaseName(k, A)!=-1, "keySetBaseName returned an error"); \
	succeed_if_same_string(keyBaseName(k), A); \
	succeed_if(keyGetBaseName(k, buffer, 499)!=-1, "keyGetBaseName returned an error"); \
	succeed_if_same_string(buffer, A); \
	} while(0)

static void test_keyEscape()
{
	printf ("test escape in basename\n");

	Key *k = keyNew("/", KEY_END);
	char buffer [500];

#include <data_escape.c>

	keySetName(k, "spec");

#include <data_escape.c>

	keySetName(k, "proc");

#include <data_escape.c>

	keySetName(k, "dir");

#include <data_escape.c>

	keySetName(k, "user");

#include <data_escape.c>

	keySetName(k, "system");

#include <data_escape.c>


	keyDel(k);
}

static void test_keyAdd()
{
	printf ("test keyAdd\n");

	/*
	Key *k = keyNew("", KEY_END);
	succeed_if (keyAddName(0, "valid") == -1, "cannot add to null name");
	succeed_if (keyAddName(k, "valid") == -1, "cannot add to empty name");

	keySetName("/");
	succeed_if (keyAddName(k, "invalid\\") == -1, "added invalid name"); // TODO?
	keyDel(k);
	*/
}

int main(int argc, char** argv)
{
	printf("KEY ABI  TESTS\n");
	printf("==================\n\n");

	init (argc, argv);

	test_keyNewSpecial();
	test_keyNewSystem();
	test_keyNewUser();
	test_keyReference();
	test_keyName();
	test_keyNameSlashes();
	test_keyValue();
	test_keyBinary();
	test_keyInactive();
	test_keyBelow();
	test_keyDup();
	test_keyCopy();
	test_binary();
	test_keyBelowOrSame();
	test_keyNameSpecial();
	test_keyClear();
	test_keyBaseName();
	test_keySetBaseName();
	test_keyAddBaseName();
	test_keyDirectBelow();
	test_keyEscape();
	test_keyAdd();

	printf("\ntestabi_key RESULTS: %d test(s) done. %d error(s).\n", nbTest, nbError);

	return nbError;
}
