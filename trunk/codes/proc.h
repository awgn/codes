#define BUFFSIZE	4096

enum p_type {
	p_int,	
	p_long,
	p_longlong,
	p_short,
	p_char,
	p_string
};

struct fb_data_t {
        char 	       *proc_name;	/* proc file name */
	enum p_type	proc_type;	/* type of var */

        void 	       *addr;           /* &var */
        int  		nelem;          /* nelem ? vector : variable */
	int		size;		/* sizeof(var) for strings */

        char 	       *r_spec;		/* printk specifier */
        char 	       *w_spec;		/* sscanf specifier */

	char 		buffer[BUFFSIZE];
	struct proc_dir_entry	*file;
};

