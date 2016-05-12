clc,clear
m = 5;
n = 5;
%rep_in = zeros(m, n);
rep_in = [000,000,000,000,000;
          000,001,011,100,000;
          000,100,110,001,000;
          000,010,010,110,000;
          000,000,000,000,000];

rep_out_w = zeros(2*m+1 , 2*n+1);
rep_out_b = zeros(2*m+1 , 2*n+1);
for i = 1:m
    for j = 1:n
        switch (rep_in(i,j))
            case 001 
                rep_out_w(2*i,2*j) = 1;
                rep_out_w(2*i,2*j-1) = 1;
                rep_out_w(2*i-1,2*j) = 1;
                rep_out_b(2*i,2*j) = 1;
                rep_out_b(2*i,2*j+1) = 1;
                rep_out_b(2*i+1,2*j) = 1;
            case 110 
                rep_out_w(2*i,2*j) = 1;
                rep_out_w(2*i,2*j+1) = 1;
                rep_out_w(2*i+1,2*j) = 1;
                rep_out_b(2*i,2*j) = 1;
                rep_out_b(2*i,2*j-1) = 1;
                rep_out_b(2*i-1,2*j) = 1;
            case 011 
                rep_out_w(2*i,2*j) = 1;
                rep_out_w(2*i,2*j+1) = 1;
                rep_out_w(2*i-1,2*j) = 1;
                rep_out_b(2*i,2*j) = 1;
                rep_out_b(2*i,2*j-1) = 1;
                rep_out_b(2*i+1,2*j) = 1;
            case 100 
                rep_out_w(2*i,2*j) = 1;
                rep_out_w(2*i,2*j-1) = 1;
                rep_out_w(2*i+1,2*j) = 1;
                rep_out_b(2*i,2*j) = 1;
                rep_out_b(2*i,2*j+1) = 1;
                rep_out_b(2*i-1,2*j) = 1;
            case 010 
                rep_out_w(2*i,2*j) = 1;
                rep_out_w(2*i-1,2*j) = 1;
                rep_out_w(2*i+1,2*j) = 1;
                rep_out_b(2*i,2*j) = 1;
                rep_out_b(2*i,2*j-1) = 1;
                rep_out_b(2*i,2*j+1) = 1;
            case 101 
                rep_out_w(2*i,2*j) = 1;
                rep_out_w(2*i,2*j-1) = 1;
                rep_out_w(2*i,2*j+1) = 1;
                rep_out_b(2*i,2*j) = 1;
                rep_out_b(2*i-1,2*j) = 1;
                rep_out_b(2*i+1,2*j) = 1;
            otherwise
        end
    end
end