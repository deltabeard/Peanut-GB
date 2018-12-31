# Benchmarks

The following benchmarks were performed using a x86_64 static build of
`peanut-benchmark-rom` at commit 738cfdad2cc1c3d4e8094ba171892bcb33c30f3a,
compiled using the following commands on Archlinux:

```
make OPT="-s -Ofast -march=native -mtune=native -static" CC=musl-gcc peanut-benchmark-rom
make OPT="-s -Ofast -march=native -mtune=native -static" CC="diet -Os gcc" peanut-benchmark-rom
make OPT="-s -Ofast -march=native -mtune=native -static" CC=gcc peanut-benchmark-rom

# Profile glibc build
make OPT="-s -Ofast -march=native -mtune=native -static -fprofile-generate" CC=gcc peanut-benchmark-rom
make OPT="-s -Ofast -march=native -mtune=native -static -fprofile-use" CC=gcc peanut-benchmark-rom

# Uncommenting line 160 to enable Interlaced mode before build
make OPT="-s -Ofast -march=native -mtune=native -static" CC=gcc peanut-benchmark-rom

# Changing define ENABLE_LCD to 0 before build
make OPT="-s -Ofast -march=native -mtune=native -static" CC=gcc peanut-benchmark-rom
```

The executables were executed with a ROM of Pokemon Blue (UE) on an Alpine
Linux live operating system with no other running tasks, on an Intel 4770K CPU.

## Results

The average of five benchmarks are presented below:

|    Configuration   |  FPS  | Executable Size (Bytes) | Percentage FPS Change |
|:------------------:|:-----:|:-----------------------:|:---------------------:|
|      musl libc     |  6998 |          59120          |         -0.23         |
|      diet libc     |  4293 |          46096          |         -38.79        |
|        glibc       |  7014 |          711328         |           0           |
|  glibc (Profiled)  |  8881 |          764576         |         26.61         |
| glibc (Interlaced) |  8855 |          711328         |         26.24         |
|   glibc (No LCD)   | 19585 |          760480         |         179.22        |
