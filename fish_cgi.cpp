// These headers are in the same order as fish-shell until a proper dependency
// chain is figured out so we can avoid redundant includes.
//
#include "config.h"

#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <limits.h>
#include <locale.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <unistd.h>

#include <cstring>
#include <cwchar>
#include <memory>
#include <string>
#include <vector>

// This src/ prefix is a hack because signal.h on disk conflicts with signal.h
// in the global include path, so we must "prefix" the includes to avoid missing
// the signal header.
//
#include "src/builtin.h"
#include "src/env.h"
#include "src/parser.h"
#include "src/utf8.h"

#include <iostream>
#include <cstring>
#include <fcgio.h>

int
main()
{
    using std::cin;
    using std::cout;
    using std::cerr;

    FCGX_Request request;

    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);

    builtin_init();
    parser_t &parser = parser_t::principal_parser();

    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf fcgi_cin (request.in);
        fcgi_streambuf fcgi_cout (request.out);
        fcgi_streambuf fcgi_cerr (request.err);

        cin.rdbuf(&fcgi_cin);
        cout.rdbuf(&fcgi_cout);
        cerr.rdbuf(&fcgi_cerr);

        const char *script_path = FCGX_GetParam("SCRIPT_FILENAME", request.envp);
        size_t path_size = strlen(script_path);

        wcstring wscript_path;
        utf8_to_wchar(script_path, path_size, &wscript_path, 0);

        parser.vars().set(L"SCRIPT_FILENAME", ENV_UNIVERSAL, {wscript_path});

        io_chain_t chain {};
        parser.eval(L". $SCRIPT_FILENAME", chain);

        cout << "Content-Type: text/html\r\n"
             << "\r\n"
             << "Hello, world\n";
    }

    return 0;
}
