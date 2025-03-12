## Simple ARM64 OS made from scratch



Dependencies:
```
sudo apt-get updatee
sudo apt-get install gcc-aarch64-linux-gnu cmake mtools gdisk qemu-system-aarch64 direnv
```

Setup direnv:
```
echo "eval \"$(direnv hook bash)\"" >> ~/.bashrc && source ~/.bashrc
```


Clone repository:
```
git clone https://github.com/ProfessorLongBeard/HelixOS.git

cd HelixOS
git submodule init
git submodule update
```

Build project:
```
mkdir build && cd build
cmake .. && make
```

Run project:
```
make setup && make run
```