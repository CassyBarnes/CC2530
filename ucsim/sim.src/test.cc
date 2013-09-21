#include <stdio.h>


class dummy {

public:
  static int counter;
  dummy() { data = counter++; }
  int data;
};

int dummy::counter = 0;


void f(int size)
{
  dummy* p;
  p = new dummy [size];
  for (int i = 0; i < size; i++)
    printf("p[%d] = %d\n", i, p[i].data);
}

int main()
{
  f(10);
  f(2);
}
