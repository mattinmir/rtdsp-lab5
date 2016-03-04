%% IMPORT EXCEL VALUES IN RIGHT ORDER!!!!!!!!!!!!!!!!!!!!!!!###########################
%%%%%%%%%%%
fsamp = 8000;
nyquist = fsamp/2;

order = 12;
fpass1 = 180/nyquist;
fpass2 = 450/nyquist;
stop_atten = 23;
pass_ripple = 0.4;

[b,a] = ellip(order/2, pass_ripple, stop_atten, [fpass1 fpass2], 'bandpass');

% Save coefficients to a text file 'fir_coefs.txt'
format long e;

formatSpec = '%2.15e';
coefs = num2str(b',formatSpec);
coefs = cellstr(coefs);
datab = strjoin(coefs', ', ');

formatSpec = '%2.15e';
coefs = num2str(a',formatSpec);
coefs = cellstr(coefs);
dataa = strjoin(coefs', ', ');

fileID = fopen('H:\RTDSPlab\lab5\RTDSP\coef.txt', 'w');
fprintf(fileID, 'double b[]={');
fprintf(fileID,datab);
fprintf (fileID, '};');
fprintf(fileID, 'double a[]={');
fprintf(fileID,dataa);
fprintf (fileID, '};');
fclose(fileID);

[h,w] = freqz(b,a, 2^12);
plot((w*(fsamp))/(2*pi), db(abs(h)), 'Color', 'b')
xlabel('Frequency (Hz)');
ylabel('Gain (dB)');
hold on
plot (X1, Y1, 'Color' , 'r')
legend('Ideal', 'Actual');

figure
semilogx(X7, Y7-Y8)
xlabel('Frequency (Hz)');
ylabel('Gain (dB)');

figure
freqz(b,a)

figure
zplane(b,a)

