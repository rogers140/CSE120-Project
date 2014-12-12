int array[192] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
         21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
         31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
         21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
         31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
         21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
         31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
         11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
         21, 22, 23, 24, 25, 26, 27, 28, 29, 30,
         31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
         41, 42, 43, 44, 45, 46, 47};


unsigned int seed;

void
irand_init()
{
  seed = 93186752;
}

unsigned int
irand(int l, int h)
{
  unsigned int a = 1588635695, m = 429496U, q = 2, r = 1117695901;
  unsigned int val;

  seed = a*(seed % q) - r*(seed / q);
  val = (seed / m) % (h - l) + l;
 
  return val;
}

int
add_array(int n)
{
    int i, sum;
    unsigned int ranNum;
    sum = 0;
    for (i = 0; i < n/2; i++) {
        ranNum= irand(0,192);
        sum += array[ranNum];
    }
    return sum;
}

int
main ()
{
    int sum;
    irand_init();   
    sum = add_array(192);
    Exit(sum);
}

