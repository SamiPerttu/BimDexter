// BimDexter.cpp
// Main program.

#include <string>
#include <iostream>
#include <locale>
#include <exception>
#include <chrono>

#include "Pixmap.h"
#include "PixelBlock.h"

using namespace std;
using namespace std::chrono;


void usage()
{
    cerr << "Usage: BimDexter [-b | -d] [-q] [-u] {input file} {output file}\n";
    cerr << "Converts between .BMP (24-bit uncompressed) and .DDS (DXT1) files.\n";
    cerr << "If not specified, the mode is chosen based on the extension of the input file.\n";
    cerr << "Options:\n";
    cerr << "  -b  Set mode: input BMP and output DDS.\n";
    cerr << "  -d  Set mode: input DDS and output BMP.\n";
    cerr << "  -q  Suppress diagnostic output to stderr.\n";
    cerr << "  -u  Choose uniform color component weighting. Default is (3, 4, 2) (R, G, B).\n";
}


// Checks whether the string has the (proper) suffix.
// The suffix must be non-empty. Ignores case differences.
bool has_suffix(string const& s, string const& suffix)
{
    bool found = false;

    for(auto i = s.rbegin(), j = suffix.rbegin(); i != s.rend(); ++i) {
        if (tolower(*i) != tolower(*j)) break;
        if (++j == suffix.rend()) { found = true; break; }
    }

    return found;
}


int main(int argc, char** argv)
{
    string filename[2];
    int filenames = 0;
    bool verbose = true;
    bool bmp_to_dds;
    bool mode_specified = false;

    // Parse command line arguments.
    for (int i = 1; i < argc; ++i) {

        std::string arg = argv[i];

        if (arg == "-b") {
            bmp_to_dds = true;
            mode_specified = true;
        } else if (arg == "-d") {
            bmp_to_dds = false;
            mode_specified = true;
        } else if (arg == "-q") {
            verbose = false;
        } else if (arg == "-u") {
            DxtPalette::setColorImportance(Vec3(1.0f));
        } else if (filenames < 2) {
            filename[filenames++] = arg;
        } else {
            usage();
            return 0;
        }
    }

    if (filenames < 2) {
        usage();
        return 0;
    }

    if (!mode_specified) {
        if (has_suffix(filename[0], ".dds")) {
            bmp_to_dds = false;
        } else if (has_suffix(filename[0], ".bmp")) {
            bmp_to_dds = true;
        } else {
            cerr << "Error: Cannot deduce mode from input file extension.\n";
            return 1;
        }
    }

    Pixmap pixmap;

    ifstream infile;
    ofstream outfile;

    if (bmp_to_dds) {
        try {
            infile.open(filename[0], ios::binary);
            if (!infile.is_open()) throw runtime_error("Cannot open input file.");
            pixmap.read_bmp(infile, verbose);
            infile.close();
            outfile.open(filename[1], ios::binary);
            if (!outfile.is_open()) throw runtime_error("Cannot open output file.");
            double time0 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            pixmap.export_dxt1(outfile, verbose);
            double time1 = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
            if (verbose) cerr << "Time taken: " << (time1 - time0) * 0.001 << " seconds.\n";
            outfile.close();
        } catch(runtime_error e) {
            cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    } else {
        try {
            infile.open(filename[0], ios::binary);
            if (!infile.is_open()) throw runtime_error("Cannot open input file.");
            pixmap.read_dxt1(infile, verbose);
            infile.close();
            outfile.open(filename[1], ios::binary);
            if (!outfile.is_open()) throw runtime_error("Cannot open output file.");
            pixmap.export_bmp(outfile, verbose);
            outfile.close();
        } catch(runtime_error e) {
            cerr << "Error: " << e.what() << "\n";
            return 1;
        }
    }

    return 0;
}
