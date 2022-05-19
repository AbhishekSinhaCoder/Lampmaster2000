struct	upc_type
	{
	char	upc[30];
	char	name[80];
	int	quantity;
	int	type;	// 1:consumable 2:appliance 3: category
	float	cost[20];
	int	total_consumed;	//how many of these have we discarded.
	struct		//for adjustable rate products
		{
		int	static_pos;
		int	price_start;
		int	price_end;
		}specials; 
	int	unit;	// size of product.
	char	unit_type[15];	// unit measurement (oz,cans, etc)
	int	parent;	// parent product (or -1 if none) (-2 if has children)
	struct
		{
		int	number;
		int	quantity;
		}children[10];
	int	parent_type;
	int	family;		// family code for coupons
	int	category;	// category for this product
	int	instant;	// Is this an instant consumable?
	};
