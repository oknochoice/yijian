src = $(wildcard ./yijian/base/*.cpp) $(wildcard ./*.cpp)
#notDirSrc = $(notdir $(src))
objects = $(patsubst %.cpp, %.o, $(src))

cleanObjects = $(shell find ./ -type f | grep "\.o$$");

target = a.out
ALL:$(target)
#ALL:
#	@echo $(src)
#	@echo $(notDirSrc)
#	@echo $(objects)
#	@echo $(cleanObjects)
#	@echo "end"

CC = g++
CFLAGS = -Wl,--no-as-needed -std=c++11 -pthread -I./yijian/base -g

$(target):$(objects)
	$(CC) -o $@ $^ $(CFLAGS)
$(objects):%.o:%.cpp
	$(CC) -c $< $(CFLAGS) -o $@
	#$(CC) -c $< $(CFLAGS) -o $(addprefix $(objectPath),$@)

clean:
	-rm -f $(cleanObjects)

.PHONY: ALL 
