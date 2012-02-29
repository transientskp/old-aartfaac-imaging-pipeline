
class MatlabBridge {
public:
	MatlabBridge();
	~MatlabBridge();
	bool process();

private:
	bool initMatlab();
	bool callMatlab(const mwArray &, const mwArray &, const mwArray &, const mwArray &, const mwArray &, const mwArray &, const mwArray &);
};
