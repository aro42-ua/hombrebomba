# Makefile de Bomberman DCA (incidencia #20)
#
# Estrategia multidirectorio: MONOLITICA. Un unico Makefile en la raiz que
# descubre los .cpp de todas las carpetas y conoce el grafo de dependencias
# completo. Se descarta la recursiva (un Makefile por carpeta) porque cada make
# solo veria su parte del grafo ("Recursive Make Considered Harmful") y aqui el
# proyecto es pequeno: src/ y tests/ comparten includes y librerias.
#
# Uso:
#   make              compila el juego en bin/bomberman (= make all)
#   make test         compila y ejecuta las pruebas de tests/
#   make clean        borra build/ y los ejecutables generados
#   make -j4          compila en paralelo
#   make CXX="ccache g++"      usa cache de compilacion
#   make CXXFLAGS="-std=c++17 -O2"   compila optimizado
#
# Requiere una shell POSIX (Linux, Git Bash, w64devkit): usa mkdir y rm.

# --- Proyecto: lo unico que hay que tocar al anadir carpetas ---------------
NAME     := bomberman
# Carpetas con codigo del juego (puede haber varias)
SRC_DIRS := src
# Fichero con main(): se excluye al enlazar las pruebas, que tienen el suyo
MAIN     := src/main.cpp
# Carpeta de pruebas: cada .cpp es un ejecutable independiente
TEST_DIR := tests
INC_DIRS := src vendor/include
LIB_DIRS := vendor/lib
BUILD    := build
BIN_DIR  := bin

# --- Herramientas y opciones (nombres estandar, se pueden sobrescribir) -----
CXX      := g++
CXXFLAGS := -std=c++17 -Wall
CPPFLAGS := $(addprefix -I,$(INC_DIRS))
LDFLAGS  := $(addprefix -L,$(LIB_DIRS))
# Genera build/**.d con las cabeceras de cada .cpp: si cambia un .hpp se
# recompilan solo los que lo incluyen
DEPFLAGS := -MMD -MP

ifeq ($(OS),Windows_NT)
  EXE    := .exe
  LDLIBS := -lraylib -lopengl32 -lgdi32 -lwinmm
else
  EXE    :=
  LDLIBS := -lraylib -lGL -lm -lpthread -lrt -lX11
endif

# --- Descubrimiento de ficheros (sin find: en Windows chocaria con find.exe) -
# rwildcard <carpetas> <patron>: wildcard recursivo
rwildcard = $(foreach d,$(wildcard $(1:=/*)),$(call rwildcard,$d,$2) $(filter $(subst *,%,$2),$d))

SRCS      := $(call rwildcard,$(SRC_DIRS),*.cpp)
TEST_SRCS := $(call rwildcard,$(TEST_DIR),*.cpp)
OBJS      := $(SRCS:%.cpp=$(BUILD)/%.o)
TEST_OBJS := $(TEST_SRCS:%.cpp=$(BUILD)/%.o)
LIB_OBJS  := $(filter-out $(BUILD)/$(MAIN:.cpp=.o),$(OBJS))
DEPS      := $(OBJS:.o=.d) $(TEST_OBJS:.o=.d)
TARGET    := $(BIN_DIR)/$(NAME)$(EXE)
TESTS     := $(TEST_SRCS:$(TEST_DIR)/%.cpp=$(BIN_DIR)/%$(EXE))

.PHONY: all test clean

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Cada prueba = su .cpp + todo el codigo del juego salvo main()
$(TESTS): $(BIN_DIR)/%$(EXE): $(BUILD)/$(TEST_DIR)/%.o $(LIB_OBJS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LDLIBS)

# Los objetos van a build/<ruta del fuente>.o. No se usa dependencia de orden
# para el directorio porque cada objeto puede ir a una carpeta distinta.
$(BUILD)/%.o: %.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

test: $(TESTS)
	@for t in $(TESTS); do echo "== $$t"; ./$$t || exit 1; done

clean:
	rm -rf $(BUILD)
	rm -f $(TARGET) $(TESTS)

-include $(DEPS)
