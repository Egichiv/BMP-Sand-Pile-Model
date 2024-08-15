#include <fstream>
#include <iostream>
#include <string>

using namespace std;

const int kHeaderSize = 26;
const int kBitsPerPixel = 3;

struct BMP_Header {
    char header_identifier[2] = {'B', 'M'}; //00 - start
    char file_size[4] = {(char) 0, (char) 0, (char) 0, (char) 0}; //02 - NTBC
    char insig1[2] = {(char) 0, (char) 0}; //depends on program where you create BMP no1, basically insignificant
    char insig2[2] = {(char) 0, (char) 0}; //same 2
    char pixel_array_address[4] = {(char) 26, (char) 0, (char) 0, (char) 0}; //0A
}; //14 bytes

struct DIB_header { //OE
    char SizeOfBitMapCoreHeader[4] = {(char) 12, (char) 0, (char) 0, (char) 0}; //always 12 though
    char WidthOfBitMap[2] = {(char) 0, (char) 0}; //unsigned 16 bit - NTBC - 1b
    char LengthOfBitMap[2] = {(char) 0, (char) 0}; //unsigned 16 bit - NTBC - 14
    char NumberOfColorPlanes[2] = {(char) 1, (char) 0}; //always 1 though - 1
    char BitsPerPixel[2] = {(char) 24, (char) 0}; //cannot be 16 or 32, MINE = 24 - offset 18
}; //12 bytes

string input_file_;
string output_directory_;

class CFlags {
public:
    CFlags(string* short_f, string* long_f, size_t short_number, size_t long_number) {
        number_of_long_flags_ = long_number;
        number_of_short_flags_ = short_number;

        short_flags_names_ = new string[number_of_short_flags_];
        long_flags_names_ = new string[number_of_long_flags_];

        flags_values_ = new long long[number_of_long_flags_]{};

        for (int i = 0; i < short_number; ++i) {
            short_flags_names_[i] = short_f[i];
        }
        for (int i = 0; i < long_number; ++i) {
            long_flags_names_[i] = long_f[i];
        }
    }

    ~CFlags() {
        delete[] long_flags_names_;
        delete[] short_flags_names_;
        delete[] flags_values_;
    }

    CFlags& OptionsFlags (char** options, int total_options) {
        for (int i = 1; i < total_options; i += 2) {
            if (options[i] == (string) "--input" || options[i] == (string) "-i") {
                input_file_ = options[i + 1];
                continue;
            }
            if (options[i] == (string) "--output" || options[i] == (string) "-o") {
                output_directory_ = options[i + 1];
                continue;
            }

            for (int j = 0; j < number_of_long_flags_; ++j) {
                if (!flags_values_[j]) {
                    if (long_flags_names_[j] == options[i]) {
                        flags_values_[j] = strtoll(options[i + 1], nullptr, 10);
                    }
                }
            }

            for (int j = 0; j < number_of_short_flags_; ++j) {
                if (!flags_values_[j]) {
                    if (short_flags_names_[j] == options[i]) {
                        flags_values_[j] = strtoll(options[i + 1], nullptr, 10);
                    }
                }
            }
        }

        return *this;
    }

    void PrintValues() const {
        for (int i = 0; i < number_of_short_flags_; ++i) {
            cout << flags_values_[i] << ' ';
        }
        cout << '\n';
    }

    [[nodiscard]] long long ValueUse (int argument_number) const {
        if (argument_number < 5 && argument_number > 0) {
            return flags_values_[argument_number - 1];
        } else {
            cout << "Big balls";
            exit(1);
        }
    }

private:
    size_t number_of_short_flags_;
    size_t number_of_long_flags_;

    string* short_flags_names_;
    string* long_flags_names_;

    long long* flags_values_;
};

string PrintInfoToBMP (BMP_Header FirstHeader, DIB_header SecondHeader, long long** pixels, long long width, long long length) {
    string result{};
    for (char i : FirstHeader.header_identifier) {
        result += i;
    }
    for (char i : FirstHeader.file_size) {
        result += i;
    }
    for (char i : FirstHeader.insig1) {
        result += i;
    }
    for (char i : FirstHeader.insig2) {
        result += i;
    }
    for (char i : FirstHeader.pixel_array_address) {
        result += i;
    }

    for (char i : SecondHeader.SizeOfBitMapCoreHeader) {
        result += i;
    }
    for (char i : SecondHeader.WidthOfBitMap) {
        result += i;
    }
    for (char i : SecondHeader.LengthOfBitMap) {
        result += i;
    }
    for (char i : SecondHeader.NumberOfColorPlanes) {
        result += i;
    }
    for (char i : SecondHeader.BitsPerPixel) {
        result += i;
    }

    for (long long x = 0; x < width; ++x) {
        for (long long y = length - 1; y >= 0; --y) {
            if (pixels[x][y] >= 4) { //black
                result += (char) 0;
                result += (char) 0;
                result += (char) 0;
            } else if (pixels[x][y] == 3) { //yellow
                result += (char) 0;
                result += (char) 255;
                result += (char) 255;
            } else if (pixels[x][y] == 2) { //violet
                result += (char) 255;
                result += (char) 0;
                result += (char) 139;
            } else if (pixels[x][y] == 1) { //green
                result += (char) 0;
                result += (char) 255;
                result += (char) 0;
            } else if (pixels[x][y] == 0) { //white
                result += (char) 255;
                result += (char) 255;
                result += (char) 255;
            }
        }
        if ((width * kBitsPerPixel) % 4 != 0) {
            for (int i = 0; i < 4 - ((width * kBitsPerPixel) % 4); ++i) {
                result += (char) 0;
            }
        }
    }

    return result;
}

int main(int argc, char** argv) {
    string short_array[4] = {"-l", "-w", "-m", "-f"};
    string long_array[4] = {"--length", "--width", "--max-iter", "--freq"};
    CFlags BMP_flags(short_array, long_array, 4, 4);

    BMP_flags.OptionsFlags(argv, argc).PrintValues();

    long long length = BMP_flags.ValueUse(1);
    long long width = BMP_flags.ValueUse(2);
    long long total_iterations = BMP_flags.ValueUse(3);
    long long save_frequency = BMP_flags.ValueUse(4);

    if (total_iterations == 0) {
        total_iterations = 1000000000000000;
        save_frequency = 0;
    }

    BMP_Header FirstHeader;
    DIB_header BitMapCoreHeader;

    long long** pixel_array;
    pixel_array = new long long*[width]{};
    for (int i = 0; i < width; ++i) {
        pixel_array[i] = new long long[length]{};
    }


    BitMapCoreHeader.WidthOfBitMap[1] = (char) (width / 256);
    BitMapCoreHeader.WidthOfBitMap[0] = (char) (width % 256);

    BitMapCoreHeader.LengthOfBitMap[1] = (char) (length / 256);
    BitMapCoreHeader.LengthOfBitMap[0] = (char) (length % 256);

    long long sizeWithPadding = (width * kBitsPerPixel + (4 - (width * kBitsPerPixel) % 4)) * length + kHeaderSize;
    long long sizeWithoutPadding = width * length * kBitsPerPixel + kHeaderSize;
    long long temporary_size;

    if (width % 4 == 0) {
        temporary_size = sizeWithoutPadding;
    } else {
        temporary_size = sizeWithPadding;
    }

    FirstHeader.file_size[3] = (char) (temporary_size / 16777216);
    temporary_size = temporary_size % 16777216;
    FirstHeader.file_size[2] = (char) (temporary_size / 65536);
    temporary_size = temporary_size % 65536;
    FirstHeader.file_size[1] = (char) (temporary_size / 256);
    temporary_size = temporary_size % 256;
    FirstHeader.file_size[0] = (char) temporary_size;

    ifstream Input(input_file_);
    while (!Input.eof()) {
        int x;
        int y;
        long long total_sand;
        Input >> x >> y >> total_sand;
        pixel_array[x - 1][y - 1] = total_sand;
    }
    Input.close();

    auto** pixel_map = new long long*[width]{};
    for (int i = 0; i < width; ++i) {
        pixel_map[i] = new long long[length]{};
    }

    for (int x = 0; x < width; ++x) {
        for (int y = 0; y < length; ++y) {
            pixel_map[x][y] = pixel_array[x][y];
        }
    }

    for (long long iteration = 0; iteration < total_iterations; ++iteration) {
        bool stability = true;

        for (int x = 0; x < width; ++x) {
            for (int y = 0; y < length; ++y) {
                if (pixel_array[x][y] >= 4) {
                    stability = false;
                    pixel_map[x][y] -= 4;

                    if (x > 0) {
                        pixel_map[x - 1][y] += 1;
                    }
                    if (x < width - 1) {
                        pixel_map[x + 1][y] += 1;
                    }
                    if (y > 0) {
                        pixel_map[x][y - 1] += 1;
                    }
                    if (y < length - 1) {
                        pixel_map[x][y + 1] += 1;
                    }
                }
            }
        }

        if (!stability) {
            for (int x = 0; x < width; ++x) {
                for (int y = 0; y < length; ++y) {
                    if (pixel_map[x][y] != pixel_array[x][y]) {
                        pixel_array[x][y] = pixel_map[x][y];
                    }
                }
            }

            if (save_frequency != 0) {
                if (iteration % save_frequency == 0) {
                    std::ofstream Picture(R"(C:\Users\egich\CLionProjects\2022-ITMO\C++_Programming\lab_3_bmp\cmake-build-debug\)" + output_directory_ + '\\' +"BMP_no_" + std::to_string(iteration + 1) + ".bmp"); //output_directory_ + '/' +
                    Picture << PrintInfoToBMP(FirstHeader, BitMapCoreHeader, pixel_map, width, length);
                    Picture.close();
                }
            }
        } else {
            std::ofstream Picture(R"(C:\Users\egich\CLionProjects\2022-ITMO\C++_Programming\lab_3_bmp\cmake-build-debug\)" + output_directory_ + '\\' + "Final_BMP.bmp"); //output_directory_ + '/' +
            Picture << PrintInfoToBMP(FirstHeader, BitMapCoreHeader, pixel_map, width, length);
            Picture.close();
            break;
        }
    }

    for (int i = 0; i < width; ++i) {
        delete[] pixel_array[i];
    }
    delete[] pixel_array;

    for (int i = 0; i < width; ++i) {
        delete[] pixel_map[i];
    }
    delete[] pixel_map;

    return 0;
}
