if [ $# -eq 1 ] && [ $1 -eq 1 ]; then
  g++ base_code_mosaique.cpp -o base_code_mosaique $(pkg-config --cflags --libs opencv4) -std=c++17
fi

./base_code_mosaique
