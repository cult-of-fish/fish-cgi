// BSD 2-Clause License
//
// Copyright © 2021 Keegan Saunders
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
//    list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
//    this list of conditions and the following disclaimer in the documentation
//    and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// These headers are in the same order as fish-shell until a proper dependency
// chain is figured out so we can avoid redundant includes.
// clang-format off
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
#include "fish-shell-src/src/builtin.h"
#include "fish-shell-src/src/builtin_eval.h"
#include "fish-shell-src/src/env.h"
#include "fish-shell-src/src/parser.h"
#include "fish-shell-src/src/utf8.h"

#include <iostream>
#include <cstring>
#include <fcgio.h>
// clang-format on

int main() {
	using std::cerr;
	using std::cin;
	using std::cout;

	FCGX_Request request;

	FCGX_Init();
	FCGX_InitRequest(&request, 0, 0);

	builtin_init();
	parser_t &parser = parser_t::principal_parser();

	while (FCGX_Accept_r(&request) == 0) {
		fcgi_streambuf fcgi_cin(request.in);
		fcgi_streambuf fcgi_cout(request.out);
		fcgi_streambuf fcgi_cerr(request.err);

		cin.rdbuf(&fcgi_cin);
		cout.rdbuf(&fcgi_cout);
		cerr.rdbuf(&fcgi_cerr);

		const char *script_path = FCGX_GetParam(
				"SCRIPT_FILENAME", request.envp);
		size_t path_size = strlen(script_path);

		wcstring wscript_path;
		utf8_to_wchar(script_path, path_size, &wscript_path, 0);

		parser.vars().set(L"SCRIPT_FILENAME", ENV_DEFAULT,
				{wscript_path});

		auto filler = io_bufferfill_t::create();
		parser.eval(L". $SCRIPT_FILENAME", io_chain_t{filler});
		separated_buffer_t buffer = io_bufferfill_t::finish(std::move(filler));
		cout << buffer.newline_serialized();
	}

	return 0;
}
