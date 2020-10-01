#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ------------------------------------------------------------------------ */
#define MAX_ARRAY	500

struct array_values
{
    int	        value;
    const char *str;
} array_of_values[MAX_ARRAY] = {
    { 5, "five" },
    { 20, "twenty" },
    { 29, "twenty nine" },
    { 32, "thirty two" },
    { 63, "sixty three" }
};   
static unsigned int num_array = 5;

/* ------------------------------------------------------------------------ */
static const struct array_values *last_b;

/* ------------------------------------------------------------------------ */
static int cmpfunc(const void *a, const void *b)
{
    last_b = b;
// fprintf(stderr, "a=%p, b= %p, a->value=%d, b->value=%d\n", a,b,((const struct array_values *)a)->value, ((const struct array_values *)b)->value);

    return(((const struct array_values *)a)->value - ((const struct array_values *)b)->value);
}   /* End of cmpfunc */

/* ------------------------------------------------------------------------ */
int main(void)
{
    struct array_values *item;
    int             find_list[] = { 0,1,4,5,6,19,20,21,28,29,30,31,32,33,62,63,64,0,1,19 };
    unsigned int    i;
    unsigned int    entry;

    for (i = 0; i < sizeof(find_list)/sizeof(find_list[0]); i++)
    {
	struct array_values key;


	key.value = find_list[i];
	key.str = "new";

        item = (struct array_values *)bsearch(&key, array_of_values, num_array, sizeof(struct array_values), cmpfunc);

	entry = last_b - &array_of_values[0];
        if (item != NULL)
        {
            fprintf(stderr, "Found item = %d (%s), entry=%d\n", item->value, item->str, entry);
        }
        else
        {
	    if (key.value < array_of_values[entry].value)	// insert before entry
	    {
		memmove(&array_of_values[entry+1], &array_of_values[entry], (num_array-entry) * sizeof(struct array_values));
		memmove(&array_of_values[entry], &key, sizeof(struct array_values));
		num_array++;
	    }
	    else						// insert after entry
	    {
		if (entry+1 != num_array)	// Check if something to move
		{
		    memmove(&array_of_values[entry+2], &array_of_values[entry+1], (num_array-entry-1) * sizeof(struct array_values));
		}
		memmove(&array_of_values[entry+1], &key, sizeof(struct array_values));
		num_array++;
	    }
        }
    }

    return (0);
}   /* End of main */

/* ------------------------------------------------------------------------ */
