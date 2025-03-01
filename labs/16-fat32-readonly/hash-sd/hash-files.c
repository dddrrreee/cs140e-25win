// engler, cs140e: given a list of files, compute their hashes.
#include <ctype.h>
#include <string.h>
#include "libunix.h"


int main(int argc, char *argv[]) { 
    char *path = getenv("CS140E_2024_PATH");
    assert(path);
    unsigned n = strlen(path);

    output("about to hash %d files\n", argc);
    for(int i = 1; i < argc; i++) {
	    unsigned nbytes;
        uint8_t *code = read_file(&nbytes, argv[i]);
        uint32_t hash = fast_hash(code, nbytes);
        // get rid of the absolute path so easier to read.
        char *name = argv[i];
        if(strncmp(path, name, n) == 0)
            name += n + 1;
        output("\tHASH: crc of file=<%s> (nbytes=%d) = 0x%x\n", name, nbytes, hash);
        free(code);
    }
    return 0;
}
