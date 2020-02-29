#include <future.h>

int ffib(int n) {

  int minus1 = 0;
  int minus2 = 0;
  int this = 0;
  int zero=0;
  int one=1;

  if (n == 0) {
    future_set(fibfut[0], &zero);
    return OK;
  }

  if (n == 1) {
    future_set(fibfut[1], &one);
    return OK;
  }

  /*int status = (int) future_get(fibfut[n-2], &minus2);

  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }

  status = (int) future_get(fibfut[n-1], &minus1);

  if (status < 1) {
    printf("future_get failed\n");
    return -1;
  }*/
  
  for(int i=2; i<=n; i++)
  {
	  //printf("The value of n is %d\n", n);
	  //int first = fibfut[i-2];
	  //int second = fibfut[i-1];
	  //future_get(zero, &minus2);
	  //future_get(one, &minus1);
	  minus1 = one;
	  minus2 = zero;
	  int value = minus1 + minus2;
	  //printf("first is %d\n", minus2);
	  //printf("second is %d\n", minus1);
	  //printf("value is %d\n", value);
	  future_set(fibfut[i], &value);
	  zero = one;
	  one = value;
  }
  //future_get(fibfut[n-2], &minus2);
  //future_get(fibfut[n-1], &minus1);

  //this = minus1 + minus2;

  

  return(0);

}