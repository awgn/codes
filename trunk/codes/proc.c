/*
 *  $Id: proc.c,v 1.2 2005/10/07 00:43:51 awgn Exp $
 * 
 *  Copyright (c) 2005 Bonelli Nicola <bonelli@antifork.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */


#ifndef __KERNEL__
#  define __KERNEL__
#endif
#ifndef MODULE
#  define MODULE
#endif
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <asm/uaccess.h>

#include "proc.h"

MODULE_AUTHOR("Bonelli Nicola <bonelli@antifork.org>");
MODULE_DESCRIPTION("proc fs module");
MODULE_LICENSE("Dual BSD/GPL");

static char procname[]="proc_fs";

static struct proc_dir_entry *proc_dir;

int   var_a;
u_int var_b;
char  vec_c[4];
int   vec_d[4] = { 0 , 1 , 2 , 3 };
char *vec_e[2] = { "hello" , "world" };
		
static struct fb_data_t display_data[]= {
  { proc_name: "a", proc_type: p_int,    addr: &var_a, nelem: 0, size:0, r_spec: "%x\n", w_spec: "%d" },
  { proc_name: "b", proc_type: p_int,    addr: &var_b, nelem: 0, size:0, r_spec: "%u\n", w_spec: "%d" },
  { proc_name: "c", proc_type: p_string, addr: vec_c,  nelem: 0, size:4, r_spec: "%s\n", w_spec: "%s" },
  { proc_name: "d", proc_type: p_int,    addr: vec_d,  nelem: 4, size:0, r_spec: "%d\n", w_spec: "%d" },
  { proc_name: "e", proc_type: p_string, addr: vec_e,  nelem: 2, size:4, r_spec: "%s\n", w_spec: NULL },
};


#ifdef __GNUC__
__inline
#endif
int 
sprintf_proc_var(char *page, struct fb_data_t *entry)
{
	int len=0;

	switch(entry->proc_type) {
                case p_int:
			len = sprintf (page, entry->r_spec, *(int *)entry->addr);
                        break;
                case p_long:
                        len = sprintf (page, entry->r_spec, *(long *)entry->addr);
                        break;
                case p_longlong:
                        len = sprintf (page, entry->r_spec, *(long long *)entry->addr);
                        break;
                case p_short:
			len = sprintf (page, entry->r_spec, *(short *)entry->addr);
                        break;
                case p_char:
			len = sprintf (page, entry->r_spec, *(char *)entry->addr);
                        break;
                case p_string:
			len = sprintf (page, entry->r_spec, (char *)entry->addr);
                        break;
	}
	return len;
}


#ifdef __GNUC__
__inline
#endif
static int
sprintf_proc_vec(char *page, struct fb_data_t *entry, int index)
{
        int len;
	len = sprintf (page, "[%d] ",index);

        switch(entry->proc_type) {
                case p_int:
                        len+= sprintf (page+len, entry->r_spec, *((int *)entry->addr+index));
                        break;
                case p_long:
                        len = sprintf (page+len, entry->r_spec, *((long *)entry->addr+index));
                        break;
                case p_longlong:
                        len = sprintf (page+len, entry->r_spec, *((long long *)entry->addr+index));
                        break;
                case p_short:
                        len+= sprintf (page+len, entry->r_spec, *((short *)entry->addr+index));
                        break;
                case p_char:
                        len+= sprintf (page+len, entry->r_spec, *((char *)entry->addr+index));
                        break;
                case p_string:
                        len+= sprintf (page+len, entry->r_spec, *((char **)entry->addr+index));
                        break;
        }
        return len;
}


#ifdef __GNUC__
__inline
#endif
static int
sscanf_proc_var(char *page, int c, struct fb_data_t *entry)
{
	int len=0;
	char *buf;

        switch(entry->proc_type) {
                case p_int:
                        len = sscanf (page, entry->w_spec, (int *)entry->addr);
                        break;
                case p_long:
                        len = sscanf (page, entry->w_spec, (long *)entry->addr);
                        break;
                case p_longlong:
                        len = sscanf (page, entry->w_spec, (long long *)entry->addr);
                        break;
                case p_short:
                        len = sscanf (page, entry->w_spec, (short *)entry->addr);
                        break;
                case p_char:
                        len = sscanf (page, entry->w_spec, (char *)entry->addr);
                        break;
                case p_string:
			buf = (char *)entry->addr;
			len = min( c , (int)entry->size-1 );
                        strncpy (buf, page, len);
        		buf[len]='\0';
	                break;
        }
	return len;
}


static int proc_read_var ( char *page,
                char **start,
                off_t off, 
                int count,
                int *eof, 
                void *data)
{
	int len;
	
	MOD_INC_USE_COUNT;

	len = sprintf_proc_var (page, (struct fb_data_t *)data);

	MOD_DEC_USE_COUNT;
	
	return len;
}

static int proc_write_var ( struct file *file,
                const char *buffer,
                unsigned long count,
                void *data)
{
	struct fb_data_t *entry = (struct fb_data_t *)data;

	int len;

	MOD_INC_USE_COUNT;

	len = ( count > BUFFSIZE ? BUFFSIZE : count );

	if ( copy_from_user (entry->buffer, buffer, len)) {
		MOD_DEC_USE_COUNT;
		return -EFAULT;
	}

	if ( len > 0 && entry->buffer[len-1] == '\n' )
		entry->buffer[len-1]= '\0';
	else
		entry->buffer[len]= '\0';

	sscanf_proc_var ( entry->buffer, len, (struct fb_data_t *)data);

	MOD_DEC_USE_COUNT;
	return len;
}


static int proc_read_vec ( char *page,
                char **start,
                off_t off,
                int count,
                int *eof,
                void *data)
{
	struct fb_data_t *entry=(struct fb_data_t *)data;
	static int mod_index;
        int len=0;

        MOD_INC_USE_COUNT;

        if (off == 0) /* restart */
                mod_index=0;
        else if (mod_index >= entry->nelem)
        	goto stop;

        while (len < (count-80) && mod_index < entry->nelem) {
		len += sprintf_proc_vec (page+len, (struct fb_data_t *)data, mod_index);		
                mod_index++;
        }
        *start=page;

        MOD_DEC_USE_COUNT;
        return len;
stop:
        *eof=1;

        MOD_DEC_USE_COUNT;
        return len;
}


static int proc_write_vec ( struct file *file,
                const char *buffer,
                unsigned long count,
                void *data)

{
        printk(KERN_INFO "%s()\n",__FUNCTION__);
	return 0;
}

 
int proc_module_init(void);
void proc_module_cleanup(void);

int __init proc_module_init()
{
	int ret=0;
	int i;
        /* create directory */

        proc_dir = proc_mkdir(procname,NULL);
        if ( proc_dir == NULL ) {
		ret = -ENOMEM;
		goto out;
	}
	printk (KERN_INFO "/proc/%s created\n",procname);

        proc_dir->owner = THIS_MODULE;

	for (i=0; i < sizeof(display_data)/sizeof(display_data[0]) ; i++ ) {

		printk(KERN_INFO "registering -> %s/%s\n", procname, display_data[i].proc_name);
				
		display_data[i].file = create_proc_entry(display_data[i].proc_name, 0644, proc_dir);
		if ( display_data[i].file == NULL ) {
			ret = -ENOMEM;
			goto unregister;
		}

#define proc_test(i)	( display_data[i].nelem == 0 )

		display_data[i].file->data      = &display_data[i];
		display_data[i].file->read_proc =(display_data[i].r_spec ? 
						(proc_test(i) ? proc_read_var : proc_read_vec) : NULL);
		display_data[i].file->write_pro =(display_data[i].w_spec ?
						(proc_test(i) ? proc_write_var : proc_write_vec) : NULL);
		display_data[i].file->owner     = THIS_MODULE;
	}	

	return 0;

    unregister:
        for (i--; i >= 0 ; i-- ) {
		remove_proc_entry(display_data[i].proc_name,proc_dir);
                printk(KERN_INFO "%s/%s unregistered\n", procname, display_data[i].proc_name);
	}
    out:
        return ret;
}


/*
 * cleanup the module, 
 */
void __exit proc_module_cleanup()
{
	int i;

        if (proc_dir) {
        	for (i=0; i < sizeof(display_data)/sizeof(display_data[0]) ; i++ ) {
			remove_proc_entry(display_data[i].proc_name,proc_dir);
			printk(KERN_INFO "%s/%s unregistered\n", procname, display_data[i].proc_name);
		}
                remove_proc_entry(procname,NULL);
        }
        printk(KERN_INFO "%s removed.\n",procname);
}

module_init(proc_module_init);
module_exit(proc_module_cleanup);
 
