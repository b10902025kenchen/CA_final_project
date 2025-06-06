# ===============================
#  Makefile (incremental rebuild)
# ===============================

# --- Toolchain ---
CXX       := g++
CXXFLAGS  := -std=c++17 -fopenmp -O3 -Wall -MMD -MP

# --- Directories ---
SRC_DIR        := .
ANALYTICAL_DIR := ./analytical

# --- Targets ---
MAIN        := main
ANALYTICAL  := analytical_solver

MAIN_SRC        := $(SRC_DIR)/main.cpp
ANALYTICAL_SRCS := $(wildcard $(ANALYTICAL_DIR)/*.cpp)
ANALYTICAL_OBJS := $(ANALYTICAL_SRCS:.cpp=.o)

PLOT_DIR := ./plot

.PHONY: all clean testcase_gen 000 001 010 011 analytical


all: $(MAIN) $(ANALYTICAL)


$(MAIN): $(MAIN_SRC)
	$(CXX) $(CXXFLAGS) $< -o $@

$(ANALYTICAL): $(ANALYTICAL_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@


$(ANALYTICAL_DIR)/%.o: $(ANALYTICAL_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ========== Utility Targets ==========

testcase_gen:
	python3 testcase_gen.py 32 10 1000 1000

# --- Quick run helpers ---
000:
	./$(MAIN) ./data/data.csv

001:
	./$(MAIN) -m 5 ./data/data.csv

010:
	./$(MAIN) -s ./data/data.csv

011:
	./$(MAIN) -s -m 5 ./data/data.csv

100:
	./$(MAIN) ./data/data.csv -t ./baseline_observation.txt

101:
	./$(MAIN) -m 5 ./data/data.csv -t ./baseline_observation.txt

110:
	./$(MAIN) -s ./data/data.csv -t ./baseline_observation.txt

111:
	./$(MAIN) -s -m 5 ./data/data.csv -t ./baseline_observation.txt

do_analytical: 
	./$(ANALYTICAL) ./testcase_star.csv -p ./testcase_para.txt -s ./testcase_score.txt -o ./testcase_observation.txt

# ========== Clean ==========
clean:
	rm -f $(MAIN) $(ANALYTICAL) $(ANALYTICAL_DIR)/*.o $(ANALYTICAL_DIR)/*.d

# ========== Auto‑generated dependencies ==========
-include $(ANALYTICAL_OBJS:.o=.d)
