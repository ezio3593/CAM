#include <iostream>

#include "CAM.h"

#define VSWORKAROUND

#ifdef VSWORKAROUND
/******* workaround for gmp for VS2015 *******/
FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
	return _iob;
}
/*********************************************/
#endif

void usage(const char *pr_name) {
	std::cerr << "Usage: " << pr_name << " [-v] [-h] [-r] code" << std::endl << std::endl;
	std::cerr << "optional parameters:" << std::endl;
	std::cerr << "\t-v: verbose" << std::endl;
	std::cerr << "\t-h: show this message" << std::endl;
	std::cerr << "\t-r: print result (value in term after execution) if -v is not specified" << std::endl;
}

int main(int argc, char **argv)
{
	if (argc < 2 || argc > 5) {
		usage(*argv);
		return -1;
	}

	std::string code;
	bool find_code = false;
	CAM::CAM cam;

	char **args = &argv[1];
	while (*args)
	{
		if (!strcmp(*args, "-h")) {
			usage(*argv);
			return 0;
		}
		else if (!strcmp(*args, "-v")) {
			cam.set_verbose(true);
		}
		else if (!strcmp(*args, "-r")) {
			cam.set_is_print_result(true);
		}
		else {
			if (find_code) {
				usage(*argv);
				return -1;
			}

			code = *args;
			find_code = true;
		}
		args++;
	}

	if (!find_code) {
		usage(*argv);
		return -1;
	}

	cam.run(code);

	return 0;
}