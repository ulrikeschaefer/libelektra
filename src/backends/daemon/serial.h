/***************************************************************************
                serial.c  -  Abstraction of serialization
                             -----------------------------
    begin                : Sun Mar 12 2006
    copyright            : (C) 2006 by Yannick Lecaillez, Avi Alkalay
    email                : sizon5@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the BSD License (revised).                      *
 *                                                                         *
 ***************************************************************************/


#ifndef __SERIAL__
#define __SERIAL__

#include <stdlib.h>
#include "datatype.h"

typedef struct {
	ssize_t (*getSizeOfSerialized)(const void *data);
	ssize_t (*serialize)(const void *data, void *into);
	ssize_t (*unserialize)(const void *data, void *into);
} SerializerFunc;

ssize_t serializeGetSize(DataType type, void *val);
ssize_t serialize(DataType type, const void *pType, void *pBuffer);
ssize_t unserialize(DataType type, const void *pBuffer, void *pType); 

#endif
