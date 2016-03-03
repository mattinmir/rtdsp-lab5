%% IMPORT EXCEL VALUES IN RIGHT ORDER!!!!!!!!!!!!!!!!!!!!!!!###########################
%%%%%%%%%%%
R = 1e3;
C= 1e-6;
b = [1];
a = [R*C 1];
q = logspace(0,4);
[h1, w1] = freqs(b,a,q);
frequency = w1/(2*pi);
mag = abs(h1);
semilogx(frequency, db(abs(h1)));
% loglog(w1, mag)
grid on
hold on
semilogx(X11, Y11, 'Color' , 'r');
hold on
semilogx(freq1,real2, 'Color', 'g');
xlabel('Frequency (Hz)');
ylabel('Gain (dB)');
legend('Ideal' , 'Actual' , '#NoFilter');

s = tf('s');

hs = 1/(s*R*C+1);
fig = figure;
% bodeplot(hs)
semilogx(frequency, db(abs(h1)));
title('Frequency Response for Ideal RC Filter');
xlabel('Frequency (Hz)');
ylabel('Gain (dB)');
print(fig, 'IdealRCFreqresp', '-dpng');


