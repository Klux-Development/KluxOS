# KluxOS Kesme İşleyicileri
# Bu dosya CPU istisnaları ve donanım kesmeleri için ilk giriş noktalarını tanımlar

.section .text
.align 4

# IDT'yi yükleme
.global idt_flush
idt_flush:
    mov 4(%esp), %eax  # İlk parametre - IDTR yapısı adresi
    lidt (%eax)        # IDTR'yi yükle
    ret

# Makrolar
# ------------------------------

# Hata kodu üreten istisnalar için
.macro ISR_ERRCODE num
.global isr\num
isr\num:
    cli               # Kesmeleri devre dışı bırak
    push $\num        # Kesme no (istisna no)
    jmp isr_common    # Ortak işleyiciye atla
.endm

# Hata kodu üretmeyen istisnalar için
.macro ISR_NOERRCODE num
.global isr\num
isr\num:
    cli               # Kesmeleri devre dışı bırak
    push $0           # Sıfır hata kodu (dummy)
    push $\num        # Kesme no (istisna no)
    jmp isr_common    # Ortak işleyiciye atla
.endm

# IRQ işleyicileri için
.macro IRQ num, irqnum
.global irq\num
irq\num:
    cli               # Kesmeleri devre dışı bırak
    push $0           # Sıfır hata kodu (dummy)
    push $\irqnum     # IRQ no
    jmp irq_common    # Ortak IRQ işleyicisine atla
.endm

# CPU İstisnaları (0-31)
# ------------------------------
ISR_NOERRCODE 0  # Sıfıra bölme hatası
ISR_NOERRCODE 1  # Debug istisnası
ISR_NOERRCODE 2  # NMI kesme
ISR_NOERRCODE 3  # Breakpoint istisnası
ISR_NOERRCODE 4  # Overflow istisnası
ISR_NOERRCODE 5  # Bound Range aşıldı
ISR_NOERRCODE 6  # Geçersiz opcode
ISR_NOERRCODE 7  # Cihaz kullanılamıyor
ISR_ERRCODE   8  # Çift hata
ISR_NOERRCODE 9  # Coprocessor segment aşımı
ISR_ERRCODE   10 # Geçersiz TSS
ISR_ERRCODE   11 # Segment yok
ISR_ERRCODE   12 # Stack segment hatası
ISR_ERRCODE   13 # Genel koruma hatası
ISR_ERRCODE   14 # Sayfa hatası
ISR_NOERRCODE 15 # Ayrılmış
ISR_NOERRCODE 16 # x87 FPU hatası
ISR_ERRCODE   17 # Alignment check hatası
ISR_NOERRCODE 18 # Machine check hatası
ISR_NOERRCODE 19 # SIMD FPU hatası
ISR_NOERRCODE 20 # Virtualization hatası
ISR_NOERRCODE 21 # Ayrılmış
ISR_NOERRCODE 22 # Ayrılmış
ISR_NOERRCODE 23 # Ayrılmış
ISR_NOERRCODE 24 # Ayrılmış
ISR_NOERRCODE 25 # Ayrılmış
ISR_NOERRCODE 26 # Ayrılmış
ISR_NOERRCODE 27 # Ayrılmış
ISR_NOERRCODE 28 # Ayrılmış
ISR_NOERRCODE 29 # Ayrılmış
ISR_ERRCODE   30 # Güvenlik istisnası
ISR_NOERRCODE 31 # Ayrılmış

# IRQ İşleyicileri (0-15) -> INT 32-47
# ------------------------------
IRQ 0, 32   # Zamanlayıcı
IRQ 1, 33   # Klavye
IRQ 2, 34   # PIC kaskadı
IRQ 3, 35   # COM2
IRQ 4, 36   # COM1
IRQ 5, 37   # LPT2
IRQ 6, 38   # Disket
IRQ 7, 39   # LPT1
IRQ 8, 40   # CMOS RTC
IRQ 9, 41   # Boş / ACPI
IRQ 10, 42  # Boş / SCSI / NIC
IRQ 11, 43  # Boş / SCSI / NIC
IRQ 12, 44  # PS/2 Mouse
IRQ 13, 45  # FPU / Coprocessor
IRQ 14, 46  # ATA birincil disk
IRQ 15, 47  # ATA ikincil disk

# İstisna işleyici - C işleyicisine geçer
isr_common:
    # Tüm yazmacları kaydet
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    # Veri segmentlerini çekirdek veri segmenti ile değiştir
    mov $0x10, %ax   # Çekirdek veri segmenti
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    # C işleyicisine geç
    push %esp        # İstisna çerçevesi adresi (hata kodu ve istisna no)
    call isr_handler # C işleyicisini çağır
    add $4, %esp     # Yığını temizle
    
    # Segmentleri ve yazmacları geri yükle
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    # Hata kodu ve istisna no'yu yığından temizle
    add $8, %esp
    
    # Kesmeden çık ve önceki duruma dön
    iret

# IRQ işleyici - C IRQ işleyicisine geçer
irq_common:
    # Tüm yazmacları kaydet
    pusha
    push %ds
    push %es
    push %fs
    push %gs
    
    # Veri segmentlerini çekirdek veri segmenti ile değiştir
    mov $0x10, %ax   # Çekirdek veri segmenti
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    
    # C işleyicisine geç
    mov 48(%esp), %eax  # IRQ no (32-47)
    sub $32, %eax       # IRQ numarasına dönüştür (0-15)
    push %eax           # IRQ no
    call irq_handler    # C IRQ işleyicisini çağır
    add $4, %esp        # Yığını temizle
    
    # Segmentleri ve yazmacları geri yükle
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popa
    
    # Hata kodu ve IRQ no'yu yığından temizle
    add $8, %esp
    
    # Kesmeden çık ve önceki duruma dön
    iret 