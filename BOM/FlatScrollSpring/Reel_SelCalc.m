clc;
clear;

T1 = 240;
T2 = 400;
B = 10;
E = 206000;
K3 = 0.85;
K4 = 0.84;
n = 8;


XI   = 1600;
XI1  = 0.6 * XI;

Tj = T2 / K3;

h1 = sqrt(6 * Tj/(B * XI))*10;
h = ceil(sqrt(6 * Tj/(B * XI))*10)/10;

L = pi*E*h*n/(K3*K4*XI);

d1_Min =25*h;
d1 = 20;

Ld = 1.2*pi*d1;
LD = 0.8*pi*d1;
L  = L + Ld + LD;

D2 = sqrt(2.55*L*h + d1^2);
D2_Min =  round(D2);

n0 = (1/(2*h))*(sqrt(4*L*h/pi+d1^2)-d1)-(K3*L*XI/(pi*E*h));
n2 = (1/(2*h))*(sqrt(4*L*h/pi+d1^2)-d1);
n1 = (1/(2*h))*(D2-sqrt(D2^2-4*L*h/pi));

N_devel = K4*(n2 -n1)
