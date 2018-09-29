










extern int errno, signgam;

extern double atof(), frexp(), ldexp(), modf();
extern double j0(), j1(), jn(), y0(), y1(), yn();
extern double erf(), erfc();
extern double exp(), log(), log10(), pow(), sqrt();
extern double floor(), ceil(), fmod(), fabs();
extern double gamma();
extern double hypot();
extern int matherr();
extern double sinh(), cosh(), tanh();
extern double sin(), cos(), tan(), asin(), acos(), atan(), atan2();





































struct exception {
	int type;
	char *name;
	double arg1;
	double arg2;
	double retval;
};









double
fmod(x, y)
register double x, y;
{
	double d; 

	



	d = ((x) < 0 ? -(x) : (x));
	if (d - ((y) < 0 ? -(y) : (y)) == d)
		return (0.0);

	(void)

	modf(x/y, &d); 
	return (x - d * y);
}
