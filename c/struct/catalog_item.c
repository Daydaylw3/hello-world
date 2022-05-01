
#define TITLE_LEN 25
#define AUTHOR_LEN 25
#define DESIGN_LEN 25

struct catalog_item {
	int stock_number;
	float price;
	int item_type;
	char title[TITLE_LEN+1];
	char author[AUTHOR_LEN+1];
	int num_pages;
	char design[DESIGN_LEN+1];
	int colors;
	int size;
};

// Save space
struct catalog_item2 {
	int stock_number;
	float price;
	int item_type;
	union {
		struct {
			char title[TITLE_LEN+1];
			char author[AUTHOR_LEN+1];
			int num_pages;
		} book;
		struct {
			char design[DESIGN_LEN+1];
		} mug;
		struct {
			char design[DESIGN_LEN+1];
			int colors;
			int sizes;
		} shirt;
	} item;
};