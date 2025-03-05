

typedef struct
{
    int* elements;
    int count;
} ArrayInt;


void ArrayInt_push(ArrayInt* array, int a);
int ArrayInt_pop(ArrayInt* array);

void ArrayInt_get(ArrayInt* array, int index, int* out);
