#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>


static int __init my_init(void) {

	printk("Welcome - hello kernal\n");
	return 0;
}







module_init(my_init);
