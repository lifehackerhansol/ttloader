mkdir data
make -C bootloader LOADBIN=$(pwd)/data/load.bin
make -C libslim
make
