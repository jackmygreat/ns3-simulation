
The Network Simulator, Version 3
================================

## Table of Contents:

1) [An overview](#an-open-source-project)
2) [Building ns-3](#building-ns-3)
3) [Running ns-3](#running-ns-3)
4) [Getting access to the ns-3 documentation](#getting-access-to-the-ns-3-documentation)
5) [Working with the development version of ns-3](#working-with-the-development-version-of-ns-3)
6) [Use Protobuf and DC topology](#use-protobuf-and-dc-topology)

Note:  Much more substantial information about ns-3 can be found at
https://www.nsnam.org

## An Open Source project

ns-3 is a free open source project aiming to build a discrete-event
network simulator targeted for simulation research and education.
This is a collaborative project; we hope that
the missing pieces of the models we have not yet implemented
will be contributed by the community in an open collaboration
process.

The process of contributing to the ns-3 project varies with
the people involved, the amount of time they can invest
and the type of model they want to work on, but the current
process that the project tries to follow is described here:
https://www.nsnam.org/developers/contributing-code/

This README excerpts some details from a more extensive
tutorial that is maintained at:
https://www.nsnam.org/documentation/latest/

## Building ns-3

The code for the framework and the default models provided
by ns-3 is built as a set of libraries. User simulations
are expected to be written as simple programs that make
use of these ns-3 libraries.

To build the set of default libraries and the example
programs included in this package, you need to use the
tool 'ns3'. Detailed information on how to use ns3 is
included in the file doc/build.txt

However, the real quick and dirty way to get started is to
type the command
```shell
./ns3 configure --enable-examples
```

followed by

```shell
./ns3
```

in the directory which contains this README file. The files
built will be copied in the build/ directory.

The current codebase is expected to build and run on the
set of platforms listed in the [release notes](RELEASE_NOTES.md)
file.

Other platforms may or may not work: we welcome patches to
improve the portability of the code to these other platforms.

## Running ns-3

On recent Linux systems, once you have built ns-3 (with examples
enabled), it should be easy to run the sample programs with the
following command, such as:

```shell
./ns3 run simple-global-routing
```

That program should generate a `simple-global-routing.tr` text
trace file and a set of `simple-global-routing-xx-xx.pcap` binary
pcap trace files, which can be read by `tcpdump -tt -r filename.pcap`
The program source can be found in the examples/routing directory.

## Getting access to the ns-3 documentation

Once you have verified that your build of ns-3 works by running
the simple-point-to-point example as outlined in 3) above, it is
quite likely that you will want to get started on reading
some ns-3 documentation.

All of that documentation should always be available from
the ns-3 website: https://www.nsnam.org/documentation/.

This documentation includes:

  - a tutorial

  - a reference manual

  - models in the ns-3 model library

  - a wiki for user-contributed tips: https://www.nsnam.org/wiki/

  - API documentation generated using doxygen: this is
    a reference manual, most likely not very well suited
    as introductory text:
    https://www.nsnam.org/doxygen/index.html

## Working with the development version of ns-3

If you want to download and use the development version of ns-3, you
need to use the tool `git`. A quick and dirty cheat sheet is included
in the manual, but reading through the git
tutorials found in the Internet is usually a good idea if you are not
familiar with it.

If you have successfully installed git, you can get
a copy of the development version with the following command:
```shell
git clone https://gitlab.com/nsnam/ns-3-dev.git
```

However, we recommend to follow the Gitlab guidelines for starters,
that includes creating a Gitlab account, forking the ns-3-dev project
under the new account's name, and then cloning the forked repository.
You can find more information in the [manual](https://www.nsnam.org/docs/manual/html/working-with-git.html).

## Use Protobuf and DC topology

**NOTICE**: this feature is still working in progress and does not have full function for now.

### Features

1. Use Python script to serve as the configuration (see `config/dumbell_topo.py` as an example). It uses Protobuf to transform the configuration to serialized binary files to be read by C++ .
2. Provide 3 modules for now to add support for datacenter. `src/dc-env/` carries the topology information in `DcTopology`. `src/protobuf-loader/` is used to load protobuf binary files to `DcTopology`. `src/dcb/` contains the datacenter protocol stacks.

### Usage

1. Install Protobuf

	Follow the [instructions](https://github.com/protocolbuffers/protobuf/blob/main/src/README.md) in the official page.
	
	To build from source, download the one of the [releases](https://github.com/protocolbuffers/protobuf/releases/latest). Since we only need support for C++ and Python, user can download the release of "protobuf-python".
	
2. Configure ns-3 to use protobuf
   
   Add the flag `--enable-protobuf` when configuring. For example:
   
   ```bash
   $ ./ns3 configure --build-profile=debug --enable-examples --enable-protobuf
   ```

3. Run the example:

	One program has two parts, the Python configuration script and the C++ program.
	First, run the Python configuration script:
	
	```bash
	$ python3 config/dumbell_topo.py
	```

	This will generate two binary files `config/topology.bin` and `config/flows.bin`.
	
	Then, run the C++ program which will automatically find the two files and run the ns-3 logic.

	```bash
	$ cp src/protobuf-loader/example/dumbell.cc scratch/
	$ ./ns3 run dumbell
	```

	If you do not want to run the Python script manually every time, you can use the `ProtobufTopologyLoader::RunConfigScript()` in the C++ file, as is shown in line 32 in `src/protobuf-loader/example/dumbell.cc`.


