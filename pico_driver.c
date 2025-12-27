#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/timer.h>
#include <linux/jiffies.h>

#define DEVICE_NAME "pico_usb0"
#define CLASS_NAME  "pico"
#define MY_TIMEOUT  2000 // 2 segundos

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pedro Wilson");
MODULE_DESCRIPTION("Driver TPSE II - Simulação Pico USB");

static int majorNumber;
static char message[256] = {0};
static short size_of_message;
static struct timer_list timer0;
static int counter = 0;

// Callback do Timer (Exatamente como no PDF)
static void timer_callback(struct timer_list *t) {
    printk(KERN_INFO "Pico Driver: Evento gerado. Contador: %d\n", counter++);
    mod_timer(&timer0, jiffies + msecs_to_jiffies(MY_TIMEOUT));
}

// Operação de abertura do dispositivo
static int dev_open(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Pico Driver: Dispositivo aberto.\n");
    return 0;
}

// Operação de leitura (O que a user_app recebe)
static ssize_t dev_read(struct file *filep, char *buffer, size_t len, loff_t *offset) {
    int error_count = 0;
    // Simula o Pico respondendo com o valor do contador atual
    sprintf(message, "Pico respondendo! Contador atual: %d", counter);
    size_of_message = strlen(message);

    error_count = copy_to_user(buffer, message, size_of_message);

    if (error_count == 0) {
        printk(KERN_INFO "Pico Driver: Enviados %d caracteres para o usuario\n", size_of_message);
        return (size_of_message = 0); // Limpa para a próxima leitura
    } else {
        printk(KERN_INFO "Pico Driver: Falha ao enviar dados\n");
        return -EFAULT;
    }
}

// Operação de escrita (O que a user_app envia)
static ssize_t dev_write(struct file *filep, const char *buffer, size_t len, loff_t *offset) {
    copy_from_user(message, buffer, len);
    printk(KERN_INFO "Pico Driver: Recebido do usuario: %s\n", message);
    return len;
}

static int dev_release(struct inode *inodep, struct file *filep) {
    printk(KERN_INFO "Pico Driver: Dispositivo fechado.\n");
    return 0;
}

static struct file_operations fops = {
    .open = dev_open,
    .read = dev_read,
    .write = dev_write,
    .release = dev_release,
};

static int __init pico_driver_init(void) {
    printk(KERN_INFO "Pico Driver: Iniciando driver simulado...\n");

    // Registra o char device
    majorNumber = register_chrdev(0, DEVICE_NAME, &fops);
    if (majorNumber < 0) {
        printk(KERN_ALERT "Pico Driver: Erro ao registrar major number\n");
        return majorNumber;
    }
    printk(KERN_INFO "Pico Driver: Registrado com Major Number %d. Use mknod /dev/%s c %d 0\n", majorNumber, DEVICE_NAME, majorNumber);

    // Configura o Timer (Item do PDF)
    timer_setup(&timer0, timer_callback, 0);
    mod_timer(&timer0, jiffies + msecs_to_jiffies(MY_TIMEOUT));

    return 0;
}

static void __exit pico_driver_exit(void) {
    del_timer(&timer0);
    unregister_chrdev(majorNumber, DEVICE_NAME);
    printk(KERN_INFO "Pico Driver: Driver encerrado!\n");
}

module_init(pico_driver_init);
module_exit(pico_driver_exit);