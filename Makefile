CC = gcc 

rwildcard=$(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

src = $(call rwildcard,.,*.c)
obj = $(src:.c=.o)

archive: $(obj)
	$(CC) -o $@ $^

.PHONY: clean
clean:
	rm -f $(obj) archive
