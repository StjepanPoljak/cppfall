proj := cppfall

src_dir := source
inc_dir := include
obj_dir := build

src := $(wildcard $(src_dir)/*.cpp)
obj_no_ext := $(addprefix $(obj_dir)/, $(notdir $(basename $(src))))
obj := $(addsuffix .o,$(obj_no_ext))

cppflags := -I$(inc_dir)
libflags := -pthread

$(proj): $(obj_dir) $(obj)
	g++ $(cppflags) $(libflags) $(filter-out $(obj_dir),$^) -o $@

build/main.o: source/main.cpp
	g++ $(cppflags) -c $< -o $@

$(obj_dir)/%.o: $(src_dir)/%.cpp
	g++ $(cppflags) -c $< -o $@

.PHONY = all run clean force
run: $(proj)
	@./$(proj)

clean:
	@-rm -rf cppfall $(obj_dir)

$(obj_dir):
	@-mkdir -p $(obj_dir)
