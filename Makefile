
CC := em++
LD := ld

OBJ_NAMES := src/main.o src/resource/shadermanager.o src/resource/texturemanager.o

OUT := ./bin/website.html

C_FLAGS :=  -std=c++20
lEM_FLAGS := -sUSE_GLFW=3 -sSTB_IMAGE=1 -sEXPORTED_RUNTIME_METHODS=UTF8ToString
LEM_FLAGS := -sFULL_ES3 -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2 -sASSERTIONS=2 -sSAFE_HEAP=1 -sALLOW_MEMORY_GROWTH -sNO_DISABLE_EXCEPTION_CATCHING
EM_FLAGS := -g

.PHONY: clean

$(OUT): $(OBJ_NAMES)
	$(CC) $^ $(C_FLAGS) $(lEM_FLAGS) $(LEM_FLAGS) $(EM_FLAGS) -o $@ --embed-file resources/
	#rm -f $(OUT)

run: $(OUT)
	emrun ./bin/index.html
	#emrun $(OUT)


%.o: %.cpp
	$(CC) $(C_FLAGS) $(EM_FLAGS) -c $^ -o $@ -I./include/ -I./libs/emInclude/

clean:
	rm -rf *.o ./bin/*.js ./bin/*.wasm ./bin/*.mem $(OBJ_NAMES)