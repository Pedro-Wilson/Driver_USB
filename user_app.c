#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

int main() {
    int fd;
    char rx_buffer[256];
    char tx_buffer[] = "Mensagem do Pedro para o Pico";

    // Abre o arquivo de dispositivo que você criou com o mknod
    fd = open("/dev/pico_usb0", O_RDWR);
    if (fd < 0) {
        perror("Erro ao abrir o driver! Verifique se o /dev/pico_usb0 existe");
        return -1;
    }

    // 1. Envia dados (aciona o dev_write no driver)
    printf("Enviando dados: %s\n", tx_buffer);
    write(fd, tx_buffer, strlen(tx_buffer));

    // 2. Recebe dados (aciona o dev_read no driver)
    printf("Lendo resposta do Pico...\n");
    if (read(fd, rx_buffer, sizeof(rx_buffer)) < 0) {
        perror("Erro na leitura");
    } else {
        printf("Resposta do Driver: %s\n", rx_buffer);
    }

    close(fd);
    return 0;
}
