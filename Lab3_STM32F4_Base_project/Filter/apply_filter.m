clc

no_motion = csvread('no_motion.txt');

no_motion_filtered = csvread('no_motion_filtered.txt');
disp(no_motion)
disp(no_motion(:,1))

N = 2;
F3dB = 0.0625; %  normalized cutoff frequency/sampling/2 50/100 
d = fdesign.lowpass('N,F3dB',N,F3dB);
Hbutter = design(d,'butter','SystemObject',true);
s = coeffs(Hbutter);
%disp(Hbutter);
%[B,A] = sos2tf(s.sosMatrix,s.ScaleValues);
%(b0, b1,b2,1, –a1, –a2)
%disp(s.SOSMatrix);
%disp(Hbutter);

%filter()
%tf(Hbutter);
%y = filter(Hbutter, no_motion);
%plot(no_motion(:,1))
%hold on
%plot(y)

lpFilt = designfilt('lowpassiir','FilterOrder',2, ...
         'PassbandFrequency',8,'PassbandRipple',0.3, ...
         'SampleRate',50);
%fvtool(lpFilt)
[B,A] = tf(lpFilt);
y = filter(lpFilt, no_motion);
plot(y(:,1))

hold on
plot(no_motion(:,1))
disp([B,A]);

%disp([y(:,1) no_motion(:,1)]);