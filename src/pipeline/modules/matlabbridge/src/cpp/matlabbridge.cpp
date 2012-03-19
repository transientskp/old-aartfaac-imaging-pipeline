
#include "mclmcrrt.h"
#include "mclcppclass.h"

#include "libmatlabbridge.h"
#include "matlabbridge.h"


MatlabBridge::MatlabBridge
{
	initMatlab();
};


MatlabBridge::~MatlabBridge
{
	killMatlab();
};


bool MatlabBridge::callMatlab(const mwArray & vispad, const mwArray & acc,
							const mwArray & u, const mwArray & v,
							const mwArray & duv, const mwArray & Nuv, 
							const mwArray & vvsize)
{
	int nargout = 1;
	mwArray skymap;

	// example:
	double data[] = {0,2,3,4,5,6,7,8,9};
	mwArray a(3, 3, mxDOUBLE_CLASS, mxREAL);
	mwArray b(3, 3, mxDOUBLE_CLASS, mxREAL);
	a.SetData(data, 9);
	b.SetData(data, 9);

	fft_imager(nargout, skymap, vispad, acc, u, v, duv, Nuv, uvsize);
	return true;
};


bool MatlabBridge::initMatlab()
{
	// initialise matlab compiler runtime
	mclmcrInitialize();

	// probably we don't need this
	return mclRunMain((mclMainFcnType)run_main,0,NULL);

	const char *args[] = { "-nodisplay" };
	if (mclInitializeApplication(args, 1)) {
		std::cout << "matlab runtime initialized" << std::endl;
	} else {
		std::cerr << "An error occurred while initializing matlab application";
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


void MatlabBridge::killMatlab()
{
	libmatlabbridgeTerminate();
	mclTerminateApplication(); 
};


bool MatlabBridge::process()
{
	try {
		return callMatlab();
	}
	catch (const mwException& e) {
		std::cerr << e.what() << std::endl;
		std::cerr << "LastErrorMessage:" << std::endl;
		std::cerr << mclGetLastErrorMessage() << std::endl;
		std::cerr << "stacktrace:" << std::endl;
		libmatlabbridgePrintStackTrace();
		return false;
	}
	catch (...) {
		std::cerr << "Unexpected error thrown" << std::endl;
		return false;
	};
};

