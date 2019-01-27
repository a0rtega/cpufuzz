# cpufuzz

cpufuzz is a dumb, simple and portable CPU fuzzer.

It's written in less than 100 lines of C code and supports any architecture in which you can install Linux or other Unix-like operating systems.

Its purpose is to find bugs or at least do an initial testing in CPU emulators, but it could also be used to find bugs or undocumented instructions in hardware CPUs.

You can see some of the bugs it found in the [results](#results) section.

## Architecture

The program simply tries to execute random data as if they were instructions. This is done by child processes that will likely crash, or be killed by the parent process after a timeout.

The data that is going to be executed is sent out of the system using a network socket, so in the case of a system failure the executed buffer can be recovered for analysis.

### How to compile

```
make
```

You can easily cross compile replacing CC:

```
make CC=arm-linux-gnueabihf-gcc
```

### How to run

Run a listening socket in the host:

```
while [ 1 ]; do nc -l -p 9999; done
```

Run the fuzzer in the guest: `./cpufuzz <output_ip_addr> <port>`

Although the fuzzer tries its best to not crash, we are executing random data with unexpected results, so it's recommended to restart the fuzzer often while running:

```
while [ 1 ]; do timeout -s 9 300 ./cpufuzz 10.0.2.2 9999; pkill -9 cpufuzz; done
```

If everything goes as expected you should see something like this in the guest:

![cpufuzz fuzzing ARM in a Nexus 4 smartphone](https://raw.githubusercontent.com/a0rtega/cpufuzz/master/screens/arm_n4.png)

## Results

- [1803160](https://bugs.launchpad.net/qemu/+bug/1803160): qemu-3.1.0-rc0: tcg.c crash in temp\_load
- [1807675](https://bugs.launchpad.net/qemu/+bug/1807675): qemu commit 80422b0: tcg.c crash in temp\_load
- [1804678](https://bugs.launchpad.net/qemu/+bug/1804678): qemu-3.1.0-rc0: mips emulation hangs when executing invalid instructions
- [1813201](https://bugs.launchpad.net/qemu/+bug/1813201): QEMU TCG i386 / x86\_64 system emulation crash when executing int instruction

