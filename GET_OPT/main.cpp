#include <getopt.h>
#include <iostream>

using namespace std;

#define NO_ARG		0
#define HAS_ARG		1

static struct option long_options[] = {

        {"model_path", HAS_ARG, nullptr, 'm'},
        {"input", HAS_ARG, nullptr, 'i'},
        {"output", HAS_ARG, nullptr, 'o'},
        {nullptr, 0, nullptr, 0}

};

static const char* short_options = "mio:";


int main(int argc, char **argv)
{

    int ch, option_index;

    while ((ch = getopt_long(argc, argv, short_options, long_options, &option_index)) != -1)
    {

        switch (ch) {
            case 'm':
                cout << optarg << endl;
                break;

            case 'i':
                cout << optarg << endl;
                break;

            case 'o':
                cout << optarg << endl;
                break;

            default:
                return -1;
            
        }
        
    }

    return 0;

}