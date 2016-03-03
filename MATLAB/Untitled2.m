func = abs(sin(1000*x));
T = 1/1000;


syms t;
w = 2*pi/T;
n = 1:5;
a0 = (2/T)*int(func,t, -T/2, T/2);
an = (2/T)*int(func*cos(n*w*t),t, -T/2, T/2);
bn = (2/T)*int(func*sin(n*w*t),t, -T/2, T/2);
f = a0/2 + dot(an,cos(n*w*t)) + dot (bn, sin(n*w*t));
ezplot(func)
