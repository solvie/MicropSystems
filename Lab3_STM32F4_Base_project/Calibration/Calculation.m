Q = 0.5;
cut_f = 50;
sample_f = 100;


wc = 2*pi*cut_f/sample_f;
wS = sin(wc);
w_C = cos(wc);
alpha = wS/(2*Q);

b0 = (1 - w_C)/2;
b1 = 1 - w_C;
b2 = (1 - w_C)/2;
a0 = 1 + alpha;
a1 = -2*w_C;
a2 = 1 - alpha;

ok = [b0 b1 b2 a0 a1 a2];

% disp(ok)



N = 2;
F3dB = 0.0625; %  normalized cutoff frequency/sampling/2 50/100 
d = fdesign.lowpass('N,F3dB',N,F3dB);
Hbutter = design(d,'butter','SystemObject',true);
s = coeffs(Hbutter);
%[B,A] = sos2tf(s.sosMatrix,s.ScaleValues);
%(b0, b1,b2,1, –a1, –a2)
disp(s.SOSMatrix);
