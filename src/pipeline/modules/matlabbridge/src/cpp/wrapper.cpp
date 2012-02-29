
#include "mclmcrrt.h"
#include "mclcppclass.h"

#include "libmatlabbridge.h"

int init()
{
	const char *args[] = { "-nodisplay" };
	if (mclInitializeApplication(args, 1)) {
		std::cout << "matlab runtime initialized" << std::endl;
	} else {
		std::cerr << "An error occurred while initializing matlab runtime";
		std::cerr << mclGetLastErrorMessage() << std::endl;
		return false;  
	};

	if (libmatlabbridgeInitialize()) {
		std::cout << "matlab bridge initialized" << std::endl;
		return true;
	} else {
		std::cout << "can't init matlab bridge!" << std::endl;
		libmatlabbridgePrintStackTrace();
		return false;
	};
};

void do_matlab_stuff()
{
	int nargout;
	mwArray skymap;
	mwArray vispad;
	mwArray acc;
	mwArray u;
	mwArray v;
	mwArray duv;
	mwArray Nuv;
	mwArray uvsize;

	double data[] = {0,2,3,4,5,6,7,8,9};
	mwArray a(3, 3, mxDOUBLE_CLASS, mxREAL);
	mwArray b(3, 3, mxDOUBLE_CLASS, mxREAL);
	a.SetData(data, 9);
	b.SetData(data, 9);
	mwArray r;

	fft_imager(nargout, skymap, vispad, acc, u, v, duv, Nuv, uvsize);
};


int run_main(int argc, char **argv)
{
	if (!init()) {
		return -1;
	};

	try {
		do_matlab_stuff();
	}
	catch (const mwException& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "LastErrorMessage:" << std::endl;
		std::cerr << mclGetLastErrorMessage() << std::endl;
		std::cerr << "stacktrace:" << std::endl;
		libmatlabbridgePrintStackTrace();
		return -2;
	}
	catch (...) {
		std::cerr << "Unexpected error thrown" << std::endl;
		return -3;
	};

	std::cout << "i'm so happy I could die, bye bye" << std::endl;
	libmatlabbridgeTerminate();
	mclTerminateApplication(); 
	return 0;
};

int main()
{
	mclmcrInitialize();
	return mclRunMain((mclMainFcnType)run_main,0,NULL);
};

