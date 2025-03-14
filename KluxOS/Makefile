CC = gcc
AS = nasm
LD = ld

CFLAGS = -m32 -nostdlib -fno-builtin -fno-stack-protector -nostartfiles -nodefaultlibs -Wall -Wextra -c -I include
LDFLAGS = -T scripts/link.ld -melf_i386
ASFLAGS = -f elf32

# C kaynak dosyaları
SOURCES = $(wildcard src/kernel/*.c src/kernel/*/*.c src/drivers/*/*.c src/security/*.c src/ui/*/*.c)
HEADERS = $(wildcard include/*/*.h)

# Assembly kaynak dosyaları
ASM_SOURCES = $(wildcard src/kernel/*.asm src/kernel/*/*.asm)
ASM_OBJECTS = $(ASM_SOURCES:.asm=.o)

# Tüm objeler
C_OBJECTS = $(SOURCES:.c=.o)
OBJECTS = $(C_OBJECTS) $(ASM_OBJECTS)

# ISO için kullanılacak dosyalar
ISO_DIR = iso
ISO_BOOT = $(ISO_DIR)/boot
ISO_GRUB = $(ISO_BOOT)/grub

.PHONY: all clean run debug iso

all: kernel.bin

kernel.bin: $(OBJECTS)
	@echo "Kernel bağlanıyor..."
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "Kernel başarıyla oluşturuldu: $@"

# C kodlarını derleme kuralı
%.o: %.c $(HEADERS)
	@echo "Derleniyor: $<"
	$(CC) $(CFLAGS) -o $@ $<

# Assembly kodlarını derleme kuralı
%.o: %.asm
	@echo "Assembly derleniyor: $<"
	$(AS) $(ASFLAGS) -o $@ $<

# QEMU ile çalıştırma
run: kernel.bin
	@echo "QEMU ile çalıştırılıyor..."
	qemu-system-i386 -kernel kernel.bin -serial stdio -display sdl

# QEMU ile debug modunda çalıştırma
debug: kernel.bin
	@echo "QEMU ile debug modunda çalıştırılıyor..."
	qemu-system-i386 -kernel kernel.bin -serial stdio -display sdl -s -S

# ISO oluşturma (GRUB bootloader kullanarak)
iso: kernel.bin
	@echo "ISO dosyası oluşturuluyor..."
	@mkdir -p $(ISO_BOOT)/grub
	@cp kernel.bin $(ISO_BOOT)/
	@echo 'set timeout=10' > $(ISO_GRUB)/grub.cfg
	@echo 'set default=0' >> $(ISO_GRUB)/grub.cfg
	@echo 'set menu_color_normal=white/black' >> $(ISO_GRUB)/grub.cfg
	@echo 'set menu_color_highlight=black/light-gray' >> $(ISO_GRUB)/grub.cfg
	@echo '' >> $(ISO_GRUB)/grub.cfg
	@echo 'menuentry "KluxOS" {' >> $(ISO_GRUB)/grub.cfg
	@echo '    multiboot2 /boot/kernel.bin' >> $(ISO_GRUB)/grub.cfg
	@echo '    boot' >> $(ISO_GRUB)/grub.cfg
	@echo '}' >> $(ISO_GRUB)/grub.cfg
	@echo '' >> $(ISO_GRUB)/grub.cfg
	@echo 'menuentry "KluxOS (Multiboot1 Modu)" {' >> $(ISO_GRUB)/grub.cfg
	@echo '    multiboot /boot/kernel.bin' >> $(ISO_GRUB)/grub.cfg
	@echo '    boot' >> $(ISO_GRUB)/grub.cfg
	@echo '}' >> $(ISO_GRUB)/grub.cfg
	@grub-mkrescue -o kluxos.iso $(ISO_DIR)
	@echo "ISO oluşturuldu: kluxos.iso"

# Temizleme
clean:
	@echo "Derleme dosyaları temizleniyor..."
	rm -f $(OBJECTS) kernel.bin kluxos.iso
	rm -rf $(ISO_DIR)
	@echo "Temizleme tamamlandı."