#include "helper.h"
#include <math.h>

void intToBinary(int num, char *binary){
	int i = 31;
	int j = 0;
	int a;
	
	for(i = 31; i >= 0; i--){
		a = num >> i;
		if(a & 1){
			binary[j] = '1';
		}else{
			binary[j] = '0';
		}	
		j++;
	}
	
	binary[32] = '\n';
}

int binaryToInt(char *binary){
	int i = 0;
	int num = 0;
	
	num -= (binary[i] - '0') * pow(2, 31 - i);
	for(i = 1; i <= 31; i++){
		num += (binary[i] - '0') * pow(2, 31 - i);
	}	
	
	return num;
}