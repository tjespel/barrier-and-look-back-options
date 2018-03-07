using namespace std;
#include <random>
#include <vector>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>

random_device generator;
int M = 100000; //number of paths to average over
double h = 1.0 / 1000.0, h_ = sqrt(h); //step size for the barrier
double k = 0.0938477;
const double PI = 3.141592653589793238463;
const double SPI = sqrt(PI);
const double S2 = sqrt(2);

class call //basic call class, stores initial price, strike, interest rate, volatility, time to expiry
{
protected:
	double S_0, K, r, sigma, T;
public:
	call(double price, double exercise, double interest, double vol, double expiry) : S_0(price), K(exercise), r(interest), sigma(vol), T(expiry) {}
	//~call();
	/*all of the following generate the result for a single path, then use average to work out the value*/
	virtual double value();
	virtual double delta_LR();
	double delta_PW();
	double gamma_PWLR();
	double gamma_LR();
	double vega_LR();
	virtual double valueA();
	double delta_LRA();
	double delta_PWA();
	double gamma_PWLRA();
	double gamma_LRA();
	double vega_LRA();
	//double average(double (call::*f)());
	virtual double delta() { return 0; } //DO NOT CALL THIS!! ONLY EXISTS AS A VIRTUAL
};

class barrier : public call //subclass of call for barrier options
{
protected:
	double B;
	bool up, out; //records whether barrier is up and out, down and in, etc
public:
	barrier(double price, double exercise, double interest, double vol, double expiry, double exit, bool above = true, bool outside = true) : call(price, exercise, interest, vol, expiry) { B = exit; up = above; out = outside; } //constructor for barrier. default is up and out
	double value();
	double valueA();
	double delta();
};

class lookback : public call
{
protected:
	double M_;
	bool c;
public:
	lookback(double price, double exercise, double interest, double vol, double expiry, double m, bool cl = true) : call(price, exercise, interest, vol, expiry) { M_ = m; c = cl; }
	double value();
	double delta_LR();
	double delta_PW();
	double gamma_LR();
	double vega_LR();
};

typedef double (call::*path)();

double stdnormal_pdf(double b)
{
	return (1 / sqrt(2.0*PI))*exp(-b * b / 2.0);
}

double stdnormal_cdf(double b)
{
	return 0.5 * erfc(-b / S2);
}

double P_M(const double &m, const double &drift, const double &stime)
{
	double k = drift * stime;
	return (stdnormal_cdf(m / stime - k) - exp(2 * drift*m)*stdnormal_cdf(-m / stime - k));
}

double rho_M(const double &m, const double &drift, const double &stime)
{
	double k = drift * stime;
	double p = (2 / stime)*stdnormal_pdf(m / stime - k);
	return p - 2 * drift * exp(2 * drift*m)*stdnormal_cdf(-m / stime - k);
}

double rho_pdrift(const double &m, const double &drift, const double &stime)
{
	double k = drift * stime;
	double x = -m / stime - k;
	double answer = -(1 + 2 * drift * m)* stdnormal_cdf(x);
	answer += stdnormal_pdf(x)*stime;
	answer *= 2 * drift *exp(2 * drift*m);
	answer += (m / stime - k)*stdnormal_pdf(m / stime - k);
	return answer * 2;
}

double rho_prime(const double &m, const double &drift, const double &stime)
{
	double k = drift * stime;
	double x = -m / stime - k;
	double answer = 2 * drift*exp(2 * drift*m)*(stdnormal_pdf(x) / stime - 2 * drift*stdnormal_cdf(x));
	answer -= (2 / (stime*stime))*(m/stime - k)*stdnormal_pdf(m / stime - k);
	return answer;
}

double newtonmax(double r, double drift, double stime)
{
	double x = 0.00001, c = P_M(x, drift, stime), k;
	do 
	{
		k = x;
		x = x - (c - r) / rho_M(x, drift, stime);
		c = P_M(x, drift, stime);
	} while (abs(c - r) > 0.0001);
	//if (isinf(x)) cout << k <<"  "<< rho_M(k, drift, stime) << endl;
	return x;
}

double bisectionmax(double r, double drift, double stime)
{
	double x = 0.0, y = 10.0, cx = 1, cy = P_M(y, drift, stime);
	int count = 0;
	while ((cy - r) > 0.0)
	{
		x = y;
		y *= 2;
		cx = cy;
		cy = P_M(y, drift, stime);
		count++;
	}
	cout << y << endl;
	for (int i = 0; i<(23 + count); i++)
	{
		double m = (x + y) / 2.0;
		double cm = P_M(m, drift, stime);
		if ((cm - r) > 0.0)
		{
			x = m;
			cx = cm;
		}
		else if ((cm - r) < 0.0)
		{
			y = m;
			cy = cm;
		}
	}
	return (x + y) / 2.0;
}

double rootmax(double r, double drift, double stime)
{
	double x = newtonmax(r, drift, stime);
	if (isinf(x))
	{
		x = bisectionmax(r, drift, stime);
		cout << x << "     " << r << endl;
	}
	return x;
}

void testnewton()
{
	double drift = (0.05 / 0.3 - 0.3 / 2.0);
	double stime = 1 / S2;
	cout << drift << "   " << stime << endl;
	double x = 0.000001, c = P_M(x, drift, stime);
	do
	{
		cout << "x:  " << x << "\nP(x):  " << c << "\nrho(x):  " << rho_M(x, drift, stime) << "\n\n";
		x = x - (c - 0.0079) / rho_M(x, drift, stime);
		c = P_M(x, drift, stime);
	} while (abs(c - 0.0079) > 0.0001);
	cout << "x:  " << x << "\nP(x):  " << c << "\nrho(x):  " << rho_M(x, drift, stime)<<"\n";
	//cout << "newtonmax: " << newtonmax(0.0079, drift, stime);
}

double lookback::value()
{
	double stime = sqrt(T);
	double drift = (r / sigma - sigma / 2.0);
	if (!c) drift *= -1;
	uniform_real_distribution<double> U(0.0, 1.0);
	double u = U(generator);
	double Z = rootmax(u, drift, stime);
	Z *= sigma;
	if (!c) Z *= -1;
	if (c) return exp(-r * T)*(max(S_0*exp(Z) - max(M_, K), 0.0) + max(M_ - K, 0.0));
	else return exp(-r * T)*(max(max(M_, K) - S_0 * exp(Z), 0.0) + max(K - M_, 0.0));
}

double lookback::delta_LR()
{
	double stime = sqrt(T);
	double drift = (r / sigma - sigma / 2.0);
	if (!c) drift *= -1;
	uniform_real_distribution<double> U(0.0, 1.0);
	double u = U(generator);
	double Z = newtonmax(u, drift, stime);
	if (!c) Z *= -1;
	//double c = M_ * (M_ > K);
	if (c)
	{
		if (exp(Z*sigma)*S_0 < max(M_, K))
			return 0;
		else
		{
			u = rho_prime(Z, drift, stime) / rho_M(Z, drift, stime);
			u /= (S_0 * sigma);
			return exp(-r * T)*u*(max(M_, K) - S_0 * exp(Z*sigma));
		}
	}
	else
	{
		if (exp(Z*sigma)*S_0 > min(M_, K))
			return 0;
		else
		{
			u = rho_prime(-Z, drift, stime) / rho_M(-Z, drift, stime);
			u /= (S_0 * sigma);
			return exp(-r * T)*u*(S_0 *exp(Z*sigma) - min(M_, K));
		}
	}
}

double lookback::gamma_LR()
{
	double stime = sqrt(T);
	double drift = (r / sigma - sigma / 2.0);
	if (!c) drift *= -1;
	uniform_real_distribution<double> U(0.0, 1.0);
	double u = U(generator);
	double Z = newtonmax(u, drift, stime);
	if (!c) Z *= -1;
	//double c = M_ * (M_ > K);
	if (c)
	{
		if (exp(Z*sigma)*S_0 < max(M_, K))
			return 0;
		else
		{
			u = rho_prime(Z, drift, stime) / rho_M(Z, drift, stime);
			double answer = u / (S_0 * sigma);
			answer = answer * (answer + 1 / S_0);
			return answer * exp(-r * T) * (S_0 * exp(Z*sigma) - max(M_, K));
		}
	}
	else
	{
		if (exp(Z*sigma)*S_0 > min(M_, K))
			return 0;
		else
		{
			u = rho_prime(-Z, drift, stime) / rho_M(-Z, drift, stime);
			double answer = u / (S_0 * sigma);
			answer *= (answer + 1 / S_0);
			return answer *= exp(-r * T)*(min(M_, K) - S_0 * exp(Z*sigma));
		}
	}
}
/*
double lookback::vega_LR()
{
	double stime = sqrt(T);
	double drift = (r / sigma - sigma / 2.0);
	uniform_real_distribution<double> U(0.0, 1.0);
	double u = U(generator);
	double Z = newtonmax(u, drift, stime);
	if (exp(Z*sigma)*S_0 < max(M_, K))
		return 0;
	else
	{
		double temp = rho_M(Z, drift, stime);
		u = temp + Z * rho_prime(Z, drift, stime);
		u = u / sigma;
		u += rho_pdrift(Z, drift, stime)*(0.5 + r/(sigma*sigma));
		u = -u / temp;
		return u * (S_0 *exp(Z*sigma) - max(M_, K))*exp(-r * T);
	}
} */

double lookback::vega_LR()
{
	double stime = sqrt(T);
	double drift = (r / sigma - sigma / 2.0);
	uniform_real_distribution<double> U(0.0, 1.0);
	double u = U(generator);
	double Z = newtonmax(u, drift, stime);
	if (exp(Z*sigma)*S_0 < max(M_, K))
		return 0;
	else
	{
		double k = (-Z/stime - drift *stime);
		double answer = drift * stime + Z / stime;
		answer *= stdnormal_pdf(k);
		answer += (1 + Z * 4 * r / sigma)*stdnormal_cdf(k);
		answer = answer / sigma;
		answer *= - 2 * Z*exp(2 * drift*Z);
		u = (sigma *sigma) / 4.0 - (Z - r * T / sigma)*(Z - r * T / sigma);
		u *= 2 * stdnormal_pdf(Z / stime - drift * stime);
		u = u / (T*sigma);
		u += answer;
		answer = -rho_M(Z, drift, stime) / sigma;
		answer += u;
		return (exp(Z*sigma)*S_0 - max(M_, K))*u;
		
	}
}

double lookback::delta_PW()
{
	double stime = sqrt(T);
	double drift = (r / sigma - sigma / 2.0);
	uniform_real_distribution<double> U(0.0, 1.0);
	double u = U(generator);
	double Z = newtonmax(u, drift, stime);
	if (exp(Z*sigma)*S_0 < max(M_, K))
		return max(M_ - K, 0.0)/S_0 * exp(-r*T);
	else return (exp((Z*sigma)) + max(M_ - K, 0.0) / S_0) *exp(-r * T);
}

double call::value()
{
	double mean = (r - sigma * sigma / 2.0) * T, std = sigma * sqrt(T);
	lognormal_distribution<double> L(mean, std);
	double ex = max(L(generator)*S_0 - K, 0.0);
	return ex * exp(-r * T);
}

double call::valueA()
{
	double mean = (r - sigma * sigma / 2.0) * T, std = sigma * sqrt(T);
	normal_distribution<double> L;
	double a = L(generator);
	double ex = (max(exp(a*std + mean)*S_0 - K, 0.0) + max(exp(-a * std + mean)*S_0 - K, 0.0)) / 2.0;
	return ex * exp(-r * T);
}

double call::delta_LR()
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	double instance = max(S_T - K, 0.0);
	instance *= exp(-r * T);
	instance *= Z / (S_0 * sigma * sqrt(T));
	return instance;
}

double call::delta_LRA()
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	double instance = max(S_T - K, 0.0);
	instance *= exp(-r * T);
	instance *= Z / (S_0 * sigma * sqrt(T));
	S_T = exp((-Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	double instance1 = max(S_T - K, 0.0);
	instance1 *= exp(-r * T);
	instance1 *= -Z / (S_0 * sigma * sqrt(T));
	return (instance + instance1) / 2.0;
}

double call::delta_PW() //Pathwise method of simulating delta for call - better, but can't be applied to Barrier options
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	if (S_T > K)
	{
		return (exp(-r * T)* S_T / S_0);
	}
	else
		return 0;
}

double call::delta_PWA() //antithetic above
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	double run;
	if (S_T > K)
	{
		run = (S_T / S_0);
	}
	else
		run = 0;
	S_T = exp((-Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	if (S_T > K)
	{
		run += (S_T / S_0);
	}
	return exp(-r * T)* run / 2.0;
}

double call::gamma_PWLR() //PW-LR method of calculating gamma for call
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	if (S_T > K)
	{
		return (exp(-r * T)* S_T / (S_0*S_0) * (Z / (sigma*sqrt(T)) - 1));
	}
	else
		return 0;
}

double call::gamma_PWLRA() //antithetic above
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	double run;
	if (S_T > K)
	{
		run = (S_T / (S_0*S_0) * (Z / (sigma*sqrt(T)) - 1));
	}
	else
		run = 0;
	S_T = exp((-Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	if (S_T > K)
	{
		run += (S_T / (S_0*S_0) * (-Z / (sigma*sqrt(T)) - 1));
	}
	return exp(-r * T)* run / 2.0;
}

double call::gamma_LR() //LR method of calculating gamma
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	return -(exp(-r * T)* max(S_T - K, 0.0) * ((Z*Z - 1) * sigma * sqrt(T) - Z) / (S_0 * S_0 *sigma *sigma *T));
}

double call::gamma_LRA() //antithetic above
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	double run = -(exp(-r * T)* max(S_T - K, 0.0) * ((Z*Z - 1) * sigma * sqrt(T) - Z) / (S_0 * S_0 *sigma *sigma *T));
	Z = -Z;
	S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	run += -(exp(-r * T)* max(S_T - K, 0.0) * ((Z*Z - 1) * sigma * sqrt(T) - Z) / (S_0 * S_0 *sigma *sigma *T));
	return run / 2.0;
}

double call::vega_LR() //likelihood ratio vega calculation
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	if (S_T > K) //this can be replaced by max(S_T - K, 0.0) being put in below but is this more efficient??
	{
		return (exp(-r * T) *(S_T - K) * ((Z*Z - 1) / sigma - sqrt(T) * Z));
	}
	else
		return 0;
}

double call::vega_LRA() //antithetic above
{
	normal_distribution<double> N;
	double Z = N(generator);
	double S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0, run;
	if (S_T > K) //this can be replaced by max(S_T - K, 0.0) being put in below but is this more efficient??
	{
		run = (exp(-r * T) *(S_T - K) * ((Z*Z - 1) / sigma - sqrt(T) * Z));
	}
	else
		run = 0;
	Z = -Z;
	S_T = exp((Z * sigma * sqrt(T) + (r - sigma * sigma / 2.0) * T))*S_0;
	if (S_T > K) //this can be replaced by max(S_T - K, 0.0) being put in below but is this more efficient??
	{
		run += (exp(-r * T) *(S_T - K) * ((Z*Z - 1) / sigma - sqrt(T) * Z));
	}
	return run / 2.0;
}



double average(path f, call *C) //averages values for monte carlo estimation for regular call options and greeks, not to be used for barriers. Note: call must be passed as pointer
{
	double running = 0.0;
	//bool flag = true;
	for (double i = 1.0; i <= double(M); i++)
	{
		double d = invoke(f, C)/i;
		if (d != d) d = 0; //sometimes there will be small values for C.f() that end up becoming NaN after a division, so this prevents that
		running = (1.0 - 1.0 / i)*running + d;
	}
	return running;
}

vector<double> sim_analysis(path f, call **C, double analytic) //runs an analysis on the estimation of an option, return vector of bias, mean error, variance(assumes there is low bias, really is MSE). Note, call must be passed as double pointer
{
	double bias = 0.0, mean = 0.0, var = 0.0;
	for (double i = 1.0; i <= 300.0; i++)
	{
		double d = invoke(average, f, *C);
		bias = (1.0 - 1.0 / i)*bias + d / i;
		mean = (1.0 - 1.0 / i)*mean + abs(d) / i;
		var = ((i - 1.0)*var + (d - analytic)*(d - analytic)) / i;
		if (int(i) % 50 == 0)
			cout << i;
	}
	vector<double> v;
	v.push_back(bias);
	v.push_back(mean);
	v.push_back(var);
	return v;
}


double barrier::value() //estimates the value of a barrier option
{
	double S_t = S_0, mean = (r - sigma * sigma / 2.0) * h, std = sigma * h_;
	lognormal_distribution<double> L(mean, std);
	bool hit = false;
	double time;
	for (time = 0; time <= T && !hit; time += h)
	{
		double e = L(generator);
		S_t *= e;
		if ((S_t > B) ^ !up)
			hit = true;
	}
	if (out)
	{
		if (hit)
			S_t = 0.0;
	}
	else
	{
		if (hit)
		{
			lognormal_distribution<double> L((r - sigma * sigma / 2.0) / (T - time), sigma*sqrt(T - time));
			double e = L(generator);
			S_t *= e;
		}
		else
			S_t = 0;
	}
	return max(S_t - K, 0.0)*exp(-r * T);
}

double barrier::valueA() //antithetic above
{
	double S_tp = S_0, S_tn = S_0, mean = (r - sigma * sigma / 2.0) * h, std = sigma * h_;
	normal_distribution<double> N(0.0, std);
	bool hitp = false, hitn = false;
	double time;
	for (time = 0; time < T && !(hitp && hitn); time += h)
	{
		double e = N(generator);
		S_tp *= exp(mean + e);
		S_tn *= exp(mean - e);
		if ((S_tp > B) ^ !up)
			hitp = true;
		if ((S_tn > B) ^ !up)
			hitn = true;
	}
	if (out)
	{
		if (hitp)
			S_tp = 0;
		if (hitn)
			S_tn = 0;
	}
	else
	{
		normal_distribution<double> N_(0.0, sigma*sqrt(T - time));
		double e = N_(generator);
		if (hitp)
			S_tp *= exp(mean + e);
		else
			S_tp = 0;
		if (hitn)
			S_tn *= exp(mean - e);
		else
			S_tn = 0;
	}

	return (max(S_tp - K, 0.0) + max(S_tn - K, 0.0))*exp(-r * T) / 2.0;
}

double barrier::delta() //antithetic above
{
	double S_tp = S_0 * (1 + k), S_tn = S_0 * (1 - k), mean = (r - sigma * sigma / 2.0) * h, std = sigma * h_;
	normal_distribution<double> N(0.0, std);
	bool hitp = false, hitn = false;
	double time;
	for (time = 0; time < T && !(hitp && hitn); time += h)
	{
		double e = N(generator);
		S_tp *= exp(mean + e);
		S_tn *= exp(mean + e);
		if ((S_tp > B) ^ !up)
			hitp = true;
		if ((S_tn > B) ^ !up)
			hitn = true;
	}
	if (out)
	{
		if (hitp)
			S_tp = 0;
		if (hitn)
			S_tn = 0;
	}
	else
	{
		normal_distribution<double> N_(0.0, sigma*sqrt(T - time));
		double e = N_(generator);
		if (hitp)
			S_tp *= exp(mean + e);
		else
			S_tp = 0;
		if (hitn)
			S_tn *= exp(mean + e);
		else
			S_tn = 0;
	}

	return (max(S_tp - K, 0.0) - max(S_tn - K, 0.0))*exp(-r * T) / (2*k * S_0);
}

int main()
{
	barrier b(150, 120, 0.05, 0.3, 1, 190);
	call c(150, 120, 0.03, 0.4, 1);
	call *p = &b;
	call *q = &c;
	call **r = &q;
	call **s = &p;
	path val = &call::value;
//	path pwd = &call::delta_PW;
	path lrd = &call::gamma_LR;
	lookback l(150, 120, 0.05, 0.3, 3.0, 150, false);
	call *ll = &l;
//	cout << "lookback PW delta: " << average(pwd, ll) << endl;
	cout << "lookback: " << average(val, ll) << endl;
	//testnewton();
	/*bool stop = false;
	double right = k, left = 0.0, mid;
	double rv = sim_analysis(val, s, -0.0716)[2], lv = 0.00512656;
	for (int i = 0; i < 10; i++) //got k = 0.0938477 starting from k = 1
	{
		mid = (left + right) / 2.0;
		k = mid;
		if (rv > lv)
		{
			rv = sim_analysis(val, s, -0.0716)[2];
			right = mid;
		}
		else
		{
			lv = sim_analysis(val, s, -0.0716)[2];
			left = mid;
		}
		cout << i << endl;
	}*/
	//cout << "MSE:  " << sim_analysis(val, s, -0.1187)[2] << endl;


	/*while (k > 0 && !stop)
	{
		if (average(val, p) > 0)
			stop = true;
		k -= 0.01;
	}
	cout << k;
	
	*///cout << "barrier: " << average(val, p) << "\ncall: " << average(val, q) << endl;
	//cout << "sim test: " << sim_analysis(val, r, 41.9344)[0];
	return 0;
}
