aeswriter: aeswriter.h aeswriter.cpp
	g++ -o aeswriter aeswriter.cpp -I ../aes -L ../aes `wx-config --cflags --libs` -lwx_gtk3u_richtext-3.2 -laes
