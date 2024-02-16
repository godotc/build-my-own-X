
#include "editor.h"

int main(int argc, char **argv)
{
    try {
        ced::Editor ed;

        if (argc >= 2) {
            ed.open(argv[1]);
        }

        ed.run();
    }
    catch (std::exception &e) {
        printf("%s\n", e.what());
        // print the erron string?
    }

    return 0;
}
