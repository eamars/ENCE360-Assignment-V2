#include <stdio.h>

typedef struct
{
    int a;
    long long b;
    char c;
    char d;
} Container;

typedef struct
{

} Empty;

void somefunction(void *item)
{
    Container *x = (Container *)item;
    printf("a = %d\nb = %lld\n", x->a, x->b);
    printf("%ld %ld\n", sizeof(x), sizeof(item));
}

int main(void)
{
    Container x;
    x.a = 1;
    x.b = 103;

    printf("Size = %ld\n", sizeof(Container));

    somefunction(&x);
    return 0;

}
