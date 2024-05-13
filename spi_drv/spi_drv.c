#include <linux/cdev.h>     // cdev_add, cdev_init
#include <linux/uaccess.h>  // copy_to_user
#include <linux/module.h>   // module_init, GPL
#include <linux/spi/spi.h>  // spi_sync,
#include <linux/delay.h>    // udelay

#define MAXLEN 32
#define MODULE_DEBUG 0      // Enable/Disable Debug messages
#define DELAY_TIME_US 100   // Delay in microseconds
#define MAX_ATTEMPTS 100    // Max attempts to get correct data

/* Char Driver Globals */
static struct spi_driver spi_drv_spi_driver;
struct file_operations spi_drv_fops;
static struct class *spi_drv_class;
static dev_t devno;
static struct cdev spi_drv_cdev;

/* Definition of SPI devices */
struct PSoCdev {
  struct spi_device *spi; // Pointer to SPI device
  char header;            // Header byte
  char dataType;         // Type of data, ex. temp_3, distance
};

/* Array of SPI devices */
/* Minor used to index array */
struct PSoCdev spi_dev;
const int spi_devs_len = 1;  // Max nbr of devices
static int spi_devs_cnt = 0; // Nbr devices present

/* Macro to handle Errors */
#define ERRGOTO(label, ...)                     \
  {                                             \
    printk (__VA_ARGS__);                       \
    goto label;                                 \
  } while(0)

// Function til at sende/modtage spi data
void write_to_spi(char* cmd, char* data)
{
  if (MODULE_DEBUG)
    printk(KERN_ALERT "Writing to spi: %i %i %i\n", cmd[0], cmd[1], cmd[2]);
  struct spi_transfer t[8];
  struct spi_message m;

  // Null'er memory
  memset(t,0,sizeof(t));

  // Init spi
  spi_message_init(&m);
  m.spi = spi_dev.spi;

  // Sæt tx og rx buffers
  t[0].tx_buf = &cmd[0];
  t[0].rx_buf = NULL;
  t[0].len = 1;
  spi_message_add_tail(&t[0], &m);

  t[1].tx_buf = &cmd[1];
  t[1].rx_buf = NULL;
  t[1].len = 1;
  spi_message_add_tail(&t[1], &m);

  t[2].tx_buf = &cmd[2];
  t[2].rx_buf = NULL;
  t[2].len = 1;
  spi_message_add_tail(&t[2], &m);

  t[3].tx_buf = NULL;
  t[3].rx_buf = &data[0];
  t[3].len = 1;
  spi_message_add_tail(&t[3], &m);


  t[4].tx_buf = NULL;
  t[4].rx_buf = &data[1];
  t[4].len = 1;
  spi_message_add_tail(&t[4], &m);

  t[5].tx_buf = NULL;
  t[5].rx_buf = &data[2];
  t[5].len = 1;
  spi_message_add_tail(&t[5], &m);

  t[6].tx_buf = NULL;
  t[6].rx_buf = &data[3];
  t[6].len = 1;
  spi_message_add_tail(&t[6], &m);

  t[7].tx_buf = NULL;
  t[7].rx_buf = &data[4];
  t[7].len = 1;
  spi_message_add_tail(&t[7], &m);

  spi_sync(m.spi, &m);
  if (MODULE_DEBUG)
    printk(KERN_ALERT "Data received: %i %i %i %i %i\n", data[0], data[1], data[2], data[3], data[4]);
}

/**********************************************************
 * CHARACTER DRIVER METHODS
 **********************************************************/

/*
 * Character Driver Module Init Method
 */
static int __init spi_drv_init(void)
{
  int err=0;

  printk("spi_drv driver initializing\n");

  /* Allocate major number and register fops*/
  err = alloc_chrdev_region(&devno, 0, 255, "spi_drv driver");
  if(MAJOR(devno) <= 0)
    ERRGOTO(err_no_cleanup, "Failed to register chardev\n");
  printk(KERN_ALERT "Assigned major no: %i\n", MAJOR(devno));

  cdev_init(&spi_drv_cdev, &spi_drv_fops);
  err = cdev_add(&spi_drv_cdev, devno, 255);
  if (err)
    ERRGOTO(err_cleanup_chrdev, "Failed to create class");

  /* Polulate sysfs entries */
  spi_drv_class = class_create(THIS_MODULE, "spi_drv_class");
  if (IS_ERR(spi_drv_class))
    ERRGOTO(err_cleanup_cdev, "Failed to create class");

  /* Register SPI Driver */
  /* THIS WILL INVOKE PROBE, IF DEVICE IS PRESENT!!! */
  err = spi_register_driver(&spi_drv_spi_driver);
  if(err)
    ERRGOTO(err_cleanup_class, "Failed SPI Registration\n");

  /* Success */
  return 0;

  /* Errors during Initialization */
 err_cleanup_class:
  class_destroy(spi_drv_class);

 err_cleanup_cdev:
  cdev_del(&spi_drv_cdev);

 err_cleanup_chrdev:
  unregister_chrdev_region(devno, 255);

 err_no_cleanup:
  return err;
}

/*
 * Character Driver Module Exit Method
 */
static void __exit spi_drv_exit(void)
{
  printk("spi_drv driver Exit\n");

  spi_unregister_driver(&spi_drv_spi_driver);
  class_destroy(spi_drv_class);
  cdev_del(&spi_drv_cdev);
  unregister_chrdev_region(devno, 255);
}

/*
 * Character Driver Write File Operations Method
 */
ssize_t spi_drv_write(struct file *filep, const char __user *ubuf,
                      size_t count, loff_t *f_pos)
{

  int minor, len;
  char kbuf[MAXLEN];

  minor = iminor(filep->f_inode);

  /* Limit copy length to MAXLEN allocated andCopy from user */
  len = count < MAXLEN ? count : MAXLEN;
  if(copy_from_user(kbuf, ubuf, len))
    return -EFAULT;

  /* Pad null termination to string */
  kbuf[len] = '\0';

  // Flyt data til spi_dev struct
  spi_dev.header    = kbuf[0];
  spi_dev.dataType  = kbuf[1];

  /* Move fileptr */
  *f_pos += len;

  /* return length actually written */
  return len;
}

/*
 * Character Driver Read File Operations Method
 */
ssize_t spi_drv_read(struct file *filep, char __user *ubuf,
                     size_t count, loff_t *f_pos)
{
  // Test først om f_pos er større end 0, da det betyder at vi har outputtet før, og derfor skal returnere 0
  /*if(*f_pos > 0)
  	return 0;
  */

  int minor, len;
  char resultBuf[MAXLEN];

  minor = iminor(filep->f_inode);

  /*
    Provide a result to write to user space
  */
  // Vi skal først skrive til PSoC'en, hvilken datatype og hvilken protokol vi bruger
  u8 cmd[3];
  u8 data[5];

  // Beregner checksum byte værdi (vi bruger sum complement)
  char checksum = ~(spi_dev.header + spi_dev.dataType);

  // Vi skal først sende den byte, som vi har fået fra write() funktionen
  cmd[0] = spi_dev.header;
  cmd[1] = spi_dev.dataType;
  cmd[2] = checksum;

  // Først sender vi kommandoen
  write_to_spi(cmd, data);

  // Så venter vi i 100 µs
  udelay(DELAY_TIME_US);

  // Så læser vi dataen, og checker om checksummen er korrekt, hvis ikke, beder vi om samme data igen, slaven skal da vide at den skal sende samme data igen
  int attemptCount = 0;
  do{
    write_to_spi(cmd, data);
    udelay(DELAY_TIME_US);
    attemptCount++;
  } while (((unsigned char)(data[4]) != (unsigned char)(~(data[0] + data[1] + data[2] + data[3]))) & (attemptCount < MAX_ATTEMPTS));

  /* Convert chars to string limited to "count" size. Returns
   * length excluding NULL termination */
  len = snprintf(resultBuf, count, "%3i%3i%3i%3i\n", (int)(data[0]), (int)(data[1]), (int)(data[2]), (int)(data[3]));

  /* Append Length of NULL termination */
  len++;

  /* Copy data to user space */
  if(copy_to_user(ubuf, resultBuf, len))
    return -EFAULT;

  /* Move fileptr */
  *f_pos += len;

  return len;
}

/*
 * Character Driver File Operations Structure
 */
struct file_operations spi_drv_fops =
  {
    .owner   = THIS_MODULE,
    .write   = spi_drv_write,
    .read    = spi_drv_read,
  };

/**********************************************************
 * LINUX DEVICE MODEL METHODS (spi)
 **********************************************************/

/*
 * spi_drv Probe
 * Called when a device with the name "spi_drv" is
 * registered.
 */
static int spi_drv_probe(struct spi_device *sdev)
{
  int err = 0;
  while (spi_devs_cnt < spi_devs_len)
  {
    struct device *spi_drv_device;

    printk(KERN_DEBUG "New SPI device: %s using chip select: %i\n",
          sdev->modalias, sdev->chip_select);

    /* Check we are not creating more
      devices than we have space for */
    if (spi_devs_cnt > spi_devs_len) {
      printk(KERN_ERR "Too many SPI devices for driver\n");
      return -ENODEV;
    }

    /* Configure bits_per_word, always 8-bit for RPI!!! */
    sdev->bits_per_word = 8;
    spi_setup(sdev);

    /* Create devices, populate sysfs and
      active udev to create devices in /dev */

    /* We map spi_devs index to minor number here */
    spi_drv_device = device_create(spi_drv_class, NULL,
                                  MKDEV(MAJOR(devno), spi_devs_cnt),
                                  NULL, "spi_drv%d", spi_devs_cnt);
    if (IS_ERR(spi_drv_device))
      printk(KERN_ALERT "FAILED TO CREATE DEVICE\n");
    else
      printk(KERN_ALERT "Using spi_devs%i on major:%i, minor:%i\n",
            spi_devs_cnt, MAJOR(devno), spi_devs_cnt);

    /* Update local array of SPI devices */
    spi_dev.spi = sdev;
    ++spi_devs_cnt;
  }

  return err;
}

/*
 * spi_drv Remove
 * Called when the device is removed
 * Can deallocate data if needed
 */
static int spi_drv_remove(struct spi_device *sdev)
{
  int its_minor = 0;

  printk (KERN_ALERT "Removing spi device\n");

  /* Destroy devices created in probe() */
  while(spi_devs_cnt > 0)
  {
    --spi_devs_cnt;
    device_destroy(spi_drv_class, MKDEV(MAJOR(devno), its_minor));
    its_minor++;
  }
  return 0;
}

/*
 * spi Driver Struct
 * Holds function pointers to probe/release
 * methods and the name under which it is registered
 */
static const struct of_device_id of_spi_drv_spi_device_match[] = {
  { .compatible = "ase, spi_drv", }, {},
};

static struct spi_driver spi_drv_spi_driver = {
  .probe      = spi_drv_probe,
  .remove           = spi_drv_remove,
  .driver     = {
    .name   = "spi_drv",
    .bus    = &spi_bus_type,
    .of_match_table = of_spi_drv_spi_device_match,
    .owner  = THIS_MODULE,
  },
};

/**********************************************************
 * GENERIC LINUX DEVICE DRIVER STUFF
 **********************************************************/

/*
 * Assignment of module init/exit methods
 */
module_init(spi_drv_init);
module_exit(spi_drv_exit);

/*
 * Assignment of author and license
 */
MODULE_AUTHOR("Peter Hoegh Mikkelsen <phm@ase.au.dk>");
MODULE_LICENSE("GPL");
