/*
myled.c
This is Device driver to handle 5 LEDs for Raspberry Pi.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/uaccess.h>
#include <linux/io.h>

#define     N       8
#define     DATA    c-'0'

MODULE_AUTHOR("AtsuyaKoike");
MODULE_DESCRIPTION("driver for LED control with ROS");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.1");

static dev_t dev;
static struct cdev cdv;
static struct class *cls = NULL;

static volatile u32 *gpio_base = NULL;
static int port[] = {4, 18, 17, 27, 22, 23, 24, 25};


static ssize_t led_write(struct file* filp, const char* buf, size_t count, loff_t* pos)
{
	char c;
    int i, j, input = 0;

    if ( copy_from_user( &c, buf, sizeof(char)) )
        return -EFAULT;
    
    if ( c == '1') input = 1;
    else if ( c == '2') input = 2;
    else if ( c == '3') input = 3;
    else if ( c == '4') input = 4;
    else if ( c == '5') input = 5;
    else if ( c == '6') input = 6;
    else if ( c == '7') input = 7;
    else if ( c == '8') input = 8;

    if ( input == 8 ) {
        for ( i = 0; i < input; i++) {
            gpio_base[7] = 1 << port[i];
        }
    }
    
    else if ( input > 0 && input < 8 ) {
    for ( i = 0; i < input; i++) {
        gpio_base[7] = 1 << port[i];
    }
    for ( j = input; j <= N; j++ ) {
        gpio_base[10] = 1 << port[j];
    }
    }
    
    else if ( input == 0 ) {
        for ( i = 0; i < input; i++) {
            gpio_base[10] = 1 << port[i];
        }
    }
    /*
    if( c == '8' ) {
		for(i = 0; i < N; i++) {
			gpio_base[7] = 1 << port[i]; //All_HIGH
		}   
    }
    */
    return 1;
}


static struct file_operations led_fops = {
	.owner = THIS_MODULE,
	.write = led_write
};

static int __init init_mod(void) {
	int retval;
	int i = 0;
	gpio_base = ioremap_nocache(0x3f200000, 0xA0);
	retval = alloc_chrdev_region(&dev, 0, 1, "myled");
	
	if (retval < 0) {
		printk(KERN_ERR "alloc_chrdev_region failed.\n");
		return retval;
	}
	printk(KERN_INFO "%s is loaded. major:%d\n",__FILE__,MAJOR(dev));
	
	cdev_init(&cdv, &led_fops);
	retval = cdev_add(&cdv, dev, 1);
	
	if ( retval < 0 ) {
		printk( KERN_ERR "cdev_add failed. major:%d, minor:%d", MAJOR(dev), MINOR(dev) );
		return retval;
	}

	cls = class_create( THIS_MODULE, "myled" );
	
	if ( IS_ERR(cls) ) {
		printk(KERN_ERR "class_create failed.");
		return PTR_ERR(cls);
	}
	device_create( cls, NULL, dev, NULL, "myled%d", MINOR(dev) );
	
	for ( i = 0; i < N; i++ ) {
		const u32 led = port[i];
		const u32 index = led / 10;
		const u32 shift  =( led % 10 ) * 3;
		const u32 mask = ~( 0x7 << shift );
		gpio_base[index] = (gpio_base[index]  & mask ) | ( 0x1 << shift );
	}
	return 0;
}

static void __exit cleanup_mod(void)
{
	cdev_del(&cdv);
	device_destroy(cls, dev);
	class_destroy(cls);
	unregister_chrdev_region(dev, 1);
	printk(KERN_INFO "%s is unloaded. major:%d\n", __FILE__, MAJOR(dev));
}

module_init(init_mod);
module_exit(cleanup_mod);
