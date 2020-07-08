
CXXFLAGS = -std=c++11 -W -Wall -O1
CXX = g++

testbench: MyTest.cc reed_solomon.hh bose_chaudhuri_hocquenghem.hh berlekamp_massey.hh chien.hh forney.hh find_locations.hh correction.hh galois_field.hh galois_field_tables.hh
	$(CXX) $(CXXFLAGS) -g $< -o $@

benchmark: MyTest.cc reed_solomon.hh bose_chaudhuri_hocquenghem.hh berlekamp_massey.hh chien.hh forney.hh find_locations.hh correction.hh galois_field.hh galois_field_tables.hh
	$(CXX) $(CXXFLAGS) -DNDEBUG $< -o $@

tables_generator: tables_generator.cc
	$(CXX) $(CXXFLAGS) $< -o $@

galois_field_tables.hh: tables_generator
	./tables_generator > $@

test: testbench
	uname -p
	./testbench

speed: benchmark
	uname -p | tee RESULTS
	./benchmark | tee -a RESULTS

.PHONY: clean test

clean:
	rm -f benchmark testbench tables_generator galois_field_tables.hh

