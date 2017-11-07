clc

Xb_down = csvread('Xb_down.txt');
Yb_down = csvread('Yb_down.txt');
Xb_up = csvread('Xb_up.txt');
Yb_up = csvread('Yb_up.txt');
Zb_up = csvread('Zb_up.txt');
Zb_down = csvread('Zb_down.txt');
disp(size(Yb_up));
Zb_down_normalized = [0 0 1];
Zb_up_normalized = [0 0 -1];
Yb_down_normalized = [0 1 0];
Yb_up_normalized = [0 -1 0];
Xb_down_normalized = [1 0 0];
Xb_up_normalized = [-1 0 0];
Y = [
    repmat(Zb_down_normalized,size(Zb_down,1),1);
    repmat(Zb_up_normalized,size(Zb_up,1),1);
    repmat(Yb_down_normalized,size(Yb_down,1),1);
    repmat(Yb_up_normalized,size(Yb_up,1),1);
    repmat(Xb_down_normalized,size(Xb_down,1),1);
    repmat(Xb_up_normalized,size(Xb_up,1),1) 
    ];

W = [Zb_down;Zb_up;Yb_down;Yb_up;Xb_down;Xb_up];
W(:, 4) = 1;
X = inv(transpose(W)*W) * transpose(W) * Y;
disp(X);