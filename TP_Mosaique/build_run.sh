if [ $1 -eq 1 ]; then
  g++ mosaique.cpp -o mosaique $(pkg-config --cflags --libs opencv4) -std=c++17
fi

./mosaique
