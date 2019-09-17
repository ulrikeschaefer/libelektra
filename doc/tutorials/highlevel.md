# High-level API (with code-generation)

Most applications don't need the flexibility of the low-level API (with `kdbGet`, `kdbSet`, etc.). In most cases an easy way and safe way to
access the configuration values is preferred. This is why we created the high-level API.

There are two different ways of using the high-level API:

1. directly
2. via code-generation

The recommended way is via code-generation (which will be explained below). If you want to use the high-level API directly take a look
at [its documentation](/src/libs/highlevel/README.md). Please note, however, that certain features are only available through code-generation.

## Overview

The code-generation API builds on Elektra's specifications. Instead of just using specifications at runtime, the code-generator parses
them when invoked (ideally right before compiling) and utilises the specification when generating configuration accessor functions.

## Writing a specification

The process of writing a full specification for your application is beyond the scope of this guide. We will just focus on the parts that are
necessary for using the code-generation API.

We will use this specification in the format of the `ni` plugin:

```ini
[]
mountpoint = myapp.ini

[mydouble]
type = double
default = 0.0

[myfloatarray/#]
type = float
default = 1.1
```

In Elektra a specification is defined through the metadata of keys in the `spec` namespace. The specification above contains metadata for
three keys the parent key (`@`), `@/mydouble` and `@/myfloatarray/#`. The `#` at the end of `myfloatarray/#` indicates that it is an array.

The `mountpoint` metadata on the parent key sets the name of our application's config file (the location is defined by Elektra), it should
be unique.

The `type` metadata specifies the type of a key. The available types can be found in the high-level API
[Readme](/src/libs/highlevel/README.md) under "Data Types". It is important to set the `type`, because the code-generator will ignore all
keys that don't have a `type`.

Because we want our getters to be unable to fail (makes error handling trivial), we need to provide a `default` value as well. Note that
`default`s for array keys like `myfloatarray/#` only work via the `spec` plugin. If you didn't mount everything correctly, you will get an
error.

That's it. The code-generator just requires that each key (that you want to access) has a `type` and a `default` value.

_Note:_ You can also mark keys with the `require` metadata, if there is no reasonable default value. This is only recommended as a last
resort, but still preserves the guarantee that `elektraGet*` calls won't fail. If a `require`d key is missing, the initialization of the
`Elektra` handle will fail.

## Invoking the code-generator

The code-generator is a very powerful and flexible tool and has many options to tweak its output. If you want to know more about how to setup
everything just the way you want to, take a look at the man-pages [`kdb-gen(1)`](../help/kdb-gen.md) and
[`kdb-gen-highlevel(1)`](../help/kdb-gen-highlevel.md).

To get started the basic invocation of the code-generator should be enough:

```sh
kdb gen -F ni=spec.ini highlevel "/sw/example/myapp/#0/current" conf
```

This tells the code-generator that your application uses the parent key `/sw/example/myapp/#0/current` and that the output files should be
called `conf.*`. The argument `highlevel` just specifies which template to use and the option `-F ni=spec.ini` indicates that the file
`spec.ini` (in the `ni` plugin's format) contains the specification. While the code-generator can read a specification from the KDB, we
recommend you use the `-F` option. It keeps the KDB clean and can avoid troubles later on, when installing your application.

## Using the generated code

You can now take a look at `conf.h` and `conf.c` (the files generated by the compiler). Depending on the specification you used, these files
may be very long. They might also be formatted strangely, because of limitations in the code-generator. Feel free to reformat them with your
tool of choice, before inspecting them.

To explain how to use the generated code, lets take a look at some of `conf.h`. For brevity's sake some parts of the file have been replaced
by placeholder comments.

```c
/* file header ... */

#ifndef CONF_H
#define CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* includes ... */

/* helper macros ... */
```

First there is some boilerplate, including the copyright header, include statements and some helper macros.

```c
/**
* Tag name for 'mydouble'
*/
#define ELEKTRA_TAG_MYDOUBLE Mydouble

/**
* Tag name for 'myfloatarray/#'
*
* Required arguments:
* - kdb_long_long_t index1: Replaces occurence no. 1 of # in the keyname.
*/
#define ELEKTRA_TAG_MYFLOATARRAY Myfloatarray
```

Next we see all of the 'tag macros' used to refer to config values. These are essentially just aliases, but they allow for some flexibility
in how we generate the names of the `static inline` functions further down. You should always refer to your config values via these macros,
even if they are just aliases. This is because we might have to change the naming scheme for the functions, but we will try to keep the tag
macros unchanged.

Additionally, the comments for these macros contain the documentation on what arguments are needed for accessing the tag in question. For
example to access the elements of the array `myfloatarray/#`, we obviously need to provide an index.

```c
#define elektra_len19(x) ((x) < 10000000000000000000ULL ? 19 : 20)
/* local macros ... */
#define elektra_len(x) elektra_len00 (x)
```

Then we see some local helper macros only used in this file.

```c
static inline kdb_double_t ELEKTRA_GET (Mydouble) (Elektra * elektra) { /* ... */ }
static inline void ELEKTRA_SET (Mydouble) (Elektra * elektra, kdb_double_t value, ElektraError ** error) { /* ... */ }

static inline kdb_float_t ELEKTRA_GET (Myfloatarray) (Elektra * elektra, kdb_long_long_t index1) {  /* ... */ }
static inline void ELEKTRA_SET (Myfloatarray) (Elektra * elektra, kdb_float_t value, kdb_long_long_t index1, ElektraError ** error) { /* ... */ }
```

This is the most important part of the header. It is what makes the API work.

For each config value we generate an `ELEKTRA_GET(*)` and an `ELEKTRA_SET(*)` accessor function. All these functions are `static inline`,
because they just call other getter/setter functions with partially fixed arguments. In fact many of these functions will only be a single
line.

```c
#undef elektra_len19
/* local macros ... */
#undef elektra_len

int loadConfiguration (Elektra ** elektra, ElektraError ** error);
void printHelpMessage (Elektra * elektra, const char * usage, const char * prefix);
void exitForSpecload (int argc, const char ** argv);
```

Then we undefine the local macros we defined before and declare the three initialization functions `loadConfiguration`, `printHelpMessage`
and `exitForSpecload`.

```c
/* elektra* macros ... */

#ifdef __cplusplus
}
#endif

#endif // CONF_H
```

At the end of the file you will find the `elektra*` convenience macros. These macros can be used to make accessing config values look more
like normal function calls and avoid the ugly double parentheses in e.g. `ELEKTRA_GET (...) (...)`.

### Obtaining an `Elektra` handle

We start at the bottom of our `conf.h` excerpt. `exitForSpecload` is used to initiate specload mode, if needed. This mode makes your application
provide its specification to Elektra. How this works exactly is not so important (see [specload plugin](/src/plugins/specload/README.md)).
You only need to know, that `exitForSpecload` should be called immediately at the start of your `main` function and that it only returns, when
your application is not in specload mode.

```c
int main (int argc, const char ** argv) {
    exitForSpecload (argc, argv);
    // ...
}
```

To access your configuration, you first need to call `loadConfiguration` to get an `Elektra` handle for your application. This is done via
a snippet that is more or less the same for all applications:

```c
ElektraError * error = NULL;
Elektra * elektra = NULL;
int rc = loadConfiguration (&elektra, &error);

if (rc == -1)
{
    fprintf (stderr, "An error occurred while opening Elektra: %s", elektraErrorDescription (error));
    elektraErrorReset (&error);
    exit (EXIT_FAILURE);
}

if (rc == 1)
{
    // help mode - application was called with '-h' or '--help'
    // for more information see "Command line options" below
    printHelpMessage (elektra, NULL, NULL);
    elektraClose (elektra);
    exit (EXIT_SUCCESS);
}
```

Next it is recommended, you change the default handler for fatal errors. By default we just call `exit (EXIT_FAILURE)`, since we don't know
how you log your errors and what cleanup may be needed.

```c
elektraFatalErrorHandler (elektra, onFatalError);
```

`onFatalError` will receive the fatal `ElektraError *`. It must at least call `elektraErrorReset` on the error and then call `exit()`.

If you want to try out the application immediately, skip down to the [section about compiling](#compiling-your-application). You may also
have to follow the section on [running your application](#running-your-application) to get everything up and running.

### Reading config values

Once you have your `Elektra` instance, reading config values is easy. You just call one of the getter functions.

```c
kdb_double_t mydouble = elektraGet (elektra, ELEKTRA_TAG_MYDOUBLE);
```

Here we used the convenience macro `elektraGet`. You could also invoke the `static inline` accessor function directly:

```c
kdb_double_t mydouble = ELEKTRA_GET (ELEKTRA_TAG_MYDOUBLE) (elektra);
```

No error handling is required, because getter functions are designed to not fail. In a correct setup, either the initialization fails and
getters are never called, or getter calls always succeed.

To access config values that don't have a static key name, like arrays, you have to supply additional arguments (and use `elektraGetV`):

```c
kdb_float_t myfloat0 = elektraGetV (elektra, ELEKTRA_TAG_MYFLOATARRAY, 0);
kdb_float_t myfloat1 = elektraGetV (elektra, ELEKTRA_TAG_MYFLOATARRAY, 1);
// or
kdb_float_t myfloat0 = ELEKTRA_GET (ELEKTRA_TAG_MYFLOATARRAY) (elektra, 0);
float myfloat1 = ELEKTRA_GET (ELEKTRA_TAG_MYFLOATARRAY) (elektra, 1);
```

Of course we also need to know, how big the `myfloatarray/#` array actually is. To that end we can use `ELEKTRA_SIZE` or `elektraSize`:

```c
kdb_long_long_t myfloat_size = elektraSize (elektra, ELEKTRA_TAG_MYFLOATARRAY);
// or
kdb_long_long_t myfloat_size = ELEKTRA_SIZE (ELEKTRA_TAG_MYFLOATARRAY) (elektra);
```

`ELEKTRA_SIZE` functions like their `ELEKTRA_GET` counterparts are designed to not fail.

Please note that, while it shouldn't happen, if you setup everything correctly, calling a getter on a non-existent, wrongly typed or otherwise
inconvertible key is a fatal error. All fatal errors result in a call to the fatal error handler and therefore exit the application.

### Writing config values

Writing config values is not quite as easy as reading, but it is still quite simple:

```c
ElektraError * error = NULL;
ELEKTRA_SET (ELEKTRA_TAG_MYDOUBLE) (elektra, 3.141593, &error);
if (error == NULL) {
    // handle error
    elektraErrorReset (&error);
}
```

As you can see the complexity stems from the necessary error handling. Because setting values involves IO and other uncontrollable factors,
setter calls cannot be designed to not fail. This is why they accept an additional `ElektraError **` argument. It is important to call
`elektraErrorReset`, if an error was set. Calling a setter with a non-null `ElektraError **` parameter is a fatal error.

Of course you can also use `elektraSet` (error handling omitted):

```c
elektraSet (elektra, ELEKTRA_TAG_MYDOUBLE, 3.141593, &error);
elektraSetV (elektra, ELEKTRA_TAG_MYFLOATARRAY, 2.718282f, &error, 2);
```

Note that `elektraSetV` takes the `ElektraError` argument before the variable arguments, while in `ELEKTA_SET` the error is always the last
argument. This is because of limitations in the C macro system.

There is not setter for array sizes. Since Elektra's low-level part supports discontinuous arrays, we simply change the array size whenever
necessary, if an array element setter is called. However, the high-level API has no support for discontinuous arrays, so take care not to
create holes in your arrays, if you want to iterate over them. Remember, accessing non-existent keys (and this includes array elements) is a
fatal error.

### Command-line options

The generated `loadConfiguration` function automatically mounts the `gopts` plugin. This means that command-line options (as described
[here](command-line-options.md)) are parsed and their values are set on the corresponding keys. You don't have to do anything, apart from
setting the `opt` metadata. The only exception to that is the _help mode_.

When your application is called with `-h` or `--help`, we enter help mode. This is indicated by the return value `1` of `loadConfiguration`.
As you can see in the example above, you should call `printHelpMessage` to print an appropriate help message to `stdout` and then close the
allocated `Elektra` instance and `exit`. **Beware** an `Elektra` instance created in help mode may not be fully functional, which is why you
should immediately close it once you called `printHelpMessage`.

### Advanced concepts

The code-generator has some more advanced features that are supported out of the box. For example, you can read multiple config values at
once by utilising structs. The use of structs also allows for recursive configurations like menus (a menu can have submenus).

For more information take a look at the man-page [`kdb-gen-highlevel(1)`](../help/kdb-gen-highlevel.md).

## Compiling your application

Once you've written your application, you will want to compile it. This requires linking some libraries and adding to your include path.
The easiest way is to use CMake or pkg-config to find the needed compiler options. Examples on how set this up can be found in
[here](/examples/codegen/econf) and [here](/examples/codegen/tree).

The compiler invocation should look something like this:

```sh
cc myapp.c conf.c `pkg-config --cflags --libs elektra-codegen` -I. -o myapp -Wl,-rpath `pkg-config --variable=libdir elektra-codegen`
```

Note: At least C99 is required, so if your compiler defaults to an older version you'll need to add `-std=c99`.

## Running your application

Running your application is easy, just run the executable (e.g. `myapp`). While this might work out of the box, you will just get the default
configuration. To change the configuration you need to use `kdb`, which doesn't know about your specification yet. This means you would need
to set the `type` metadata and all the other stuff that your application expects by hand. For every single key. Obviously this is not the
right solution.

### Mounting the specification

A better solution is to inform Elektra (and `kdb`) about our specification. Then Elektra automatically copies metadata to where it should be.

First you need to mount your specification itself into the KDB. Mounting is basically the process of informing Elektra about a new part of
the KDB, similar to how mounting an external hard drive informs the OS about a new part of the file system.

```sh
sudo kdb mount -R noresolver /etc/myapp_spec.eqd "spec/sw/example/myapp/#0/current" specload app="$PWD/myapp"
```

The command above assumes that you also used the `kdb gen` command from [above](#invoking-the-code-generator) and that the `myapp` executable
is located in `$PWD`.

> **Note:** Because of a limitation in `specload`, we have to use the `noresolver` resolver. This also means that the the path to the config
> file (here `/etc/myapp_spec.eqd`) has to be absolute. Otherwise it will always be relative to the current working directory in which `kdb`
> or your application was executed. The file _should not_ exist when calling `kdb mount`. `specload` works different to other plugins. The
> given config file is only used, if the user makes changes to the specification via `kdb set`.

Now that Elektra knows about your specification, calling your application might work better, since metadata should now be copied, when you
set a config value via `kdb set`. However, there won't be any type checking. For that we need to enable the `type` plugin. While this could
be done manually, we can just let Elektra figure out which plugins we need and activate all of them.

This can be done with the `spec-mount` command:

```sh
sudo kdb spec-mount "/sw/example/myapp/#0/current"
```

Now finally your application is all setup.

### Configuring your application

To configure your application you can use `kdb`:

```sh
kdb set "/sw/example/myapp/#0/current/mydouble" 15.4
```

If you want to set a value system-wide (not just for your user) you can use `-N system`:

```sh
kdb set -N system "/sw/example/myapp/#0/current/mydouble" 15.4
```

Always use the cascading version of `kdb set` (i.e. the keyname begins with a slash `/`), otherwise type checking and other plugins might not
be called correctly.