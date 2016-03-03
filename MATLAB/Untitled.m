x = 0 : 1/8000 : 0.02*pi;

sine_freq = 1000;

figure
y = abs(sin(sine_freq*x));
subplot(211), plot(x, y)
axis([min(x)/pi max(x)/pi min(y)-.5 max(y)+.5])
xlabel('Full-wave rectified sine') 

n = 1 : 5;
freq = (2.^(n-1))*2*sine_freq;
v = 4 ./(pi*(4*n.^2 - 1));
subplot(212), stem(freq, v)
axis([0 max(freq)*1.1 0 max(v)+.5])
xlabel('Harmonics')
for k = 1:5
    text(freq(k), v(k), freq(k)) ;
end