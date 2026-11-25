/*
 * CH32V003J4M6 three-button NEC IR remote.
 *
 * SW1 (PC4) -> NEC command 152, SW2 (PC2) -> 31, SW3 (PC1) -> 62.
 * NEC frame: address 0x00, ~address 0xFF, command, ~command, LSB first.
 * PA2 (TIM1_CH2N) drives the IR LED and the indicator LED, active low.
 *
 * While a button is held its NEC frame repeats every 200 ms; releasing it
 * returns the MCU to standby (~10 uA) until the next falling edge. With
 * several buttons held, SW1 wins over SW2, which wins over SW3. The 38 kHz
 * carrier is pure timer hardware; the core only gates it.
 */

#include <stdint.h>

#define REG32(a) (*(volatile uint32_t *)(a))
#define REG16(a) (*(volatile uint16_t *)(a))

/* RCC */
#define RCC_CFGR0       REG32(0x40021004u)
#define RCC_APB2PCENR   REG32(0x40021018u)
#define RCC_APB1PCENR   REG32(0x4002101Cu)

/* AFIO / EXTI */
#define AFIO_EXTICR     REG32(0x40010008u)
#define EXTI_EVENR      REG32(0x40010404u)
#define EXTI_FTENR      REG32(0x4001040Cu)
#define EXTI_INTFR      REG32(0x40010414u)

/* GPIO */
#define GPIOA_CFGLR     REG32(0x40010800u)
#define GPIOA_BSHR      REG32(0x40010810u)
#define GPIOC_CFGLR     REG32(0x40011000u)
#define GPIOC_INDR      REG32(0x40011008u)
#define GPIOC_BSHR      REG32(0x40011010u)
#define GPIOD_CFGLR     REG32(0x40011400u)
#define GPIOD_BSHR      REG32(0x40011410u)

/* TIM1 */
#define TIM1_CTLR1      REG16(0x40012C00u)
#define TIM1_SWEVGR     REG16(0x40012C14u)
#define TIM1_CHCTLR1    REG16(0x40012C18u)
#define TIM1_CCER       REG16(0x40012C20u)
#define TIM1_ATRLR      REG16(0x40012C2Cu)
#define TIM1_CH2CVR     REG32(0x40012C38u)
#define TIM1_BDTR       REG16(0x40012C44u)

/* PWR / core */
#define PWR_CTLR        REG32(0x40007000u)
#define SYSTICK_CTLR    REG32(0xE000F000u)
#define SYSTICK_CNT     REG32(0xE000F008u)
#define PFIC_SCTLR      REG32(0xE000ED10u)

#define RCC_AFIOEN      (1u << 0)
#define RCC_IOPAEN      (1u << 2)
#define RCC_IOPCEN      (1u << 4)
#define RCC_IOPDEN      (1u << 5)
#define RCC_TIM1EN      (1u << 11)
#define RCC_PWREN       (1u << 28)

#define PWR_PDDS        (1u << 1)
#define PFIC_SLEEPDEEP  (1u << 2)
#define PFIC_WFITOWFE   (1u << 3)  /* make wfi wait for events, not IRQs */

#define KEY_MASK        ((1u << 1) | (1u << 2) | (1u << 4))  /* PC1, PC2, PC4 */

#define NEC_ADDR        0x00u
#define NEC_CMD_SW1     152u
#define NEC_CMD_SW2     31u
#define NEC_CMD_SW3     62u

#define NEC_HDR_MARK_US     9000u
#define NEC_HDR_SPACE_US    4500u
#define NEC_BIT_MARK_US     560u
#define NEC_ZERO_SPACE_US   560u
#define NEC_ONE_SPACE_US    1690u

/* SysTick ticks at HCLK = 1.5 MHz; 9000 ticks = 6 ms debounce. */
#define DEBOUNCE_TICKS  9000u
/* Gap between frames while a button is held: 300000 ticks = 200 ms. */
#define REPEAT_GAP_TICKS 300000u
/* Stay awake after power-up so the WCH-LinkE can still reach SWIO.
   Once standby is entered the debug pin is dead until a button wakes
   the chip, so without this window the part could not be reflashed. */
#define FLASH_WINDOW_TICKS 4500000u  /* 3 s */

static inline void delay_ticks(uint32_t n)
{
    uint32_t end = SYSTICK_CNT + n;
    while ((int32_t)(SYSTICK_CNT - end) < 0)
        ;
}

static inline void delay_us(uint32_t us)
{
    delay_ticks(us + (us >> 1));
}

static inline __attribute__((always_inline)) void ir_on(void)
{
    /* PA2 alternate-function push-pull: TIM1_CH2N drives the LEDs */
    GPIOA_CFGLR = (GPIOA_CFGLR & ~(0xFu << 8)) | (0x9u << 8);
}

static inline __attribute__((always_inline)) void ir_off(void)
{
    /* preset the output latch high (LEDs off) before leaving AF mode */
    GPIOA_BSHR = 1u << 2;
    GPIOA_CFGLR = (GPIOA_CFGLR & ~(0xFu << 8)) | (0x1u << 8);
}

static inline void ir_mark(uint32_t us)
{
    ir_on();
    delay_us(us);
    ir_off();
}

static void nec_byte(uint8_t value)
{
    for (uint8_t i = 0; i < 8; i++, value >>= 1) {
        ir_mark(NEC_BIT_MARK_US);
        delay_us((value & 1u) ? NEC_ONE_SPACE_US : NEC_ZERO_SPACE_US);
    }
}

static void nec_send(uint8_t cmd)
{
    ir_mark(NEC_HDR_MARK_US);
    delay_us(NEC_HDR_SPACE_US);
    nec_byte((uint8_t)NEC_ADDR);
    nec_byte((uint8_t)~NEC_ADDR);
    nec_byte(cmd);
    nec_byte((uint8_t)~cmd);
    ir_mark(NEC_BIT_MARK_US);
    ir_off();
}

static void clock_init(void)
{
    RCC_CFGR0 = 0x000000B0u;  /* HSI 24 MHz / 16 = 1.5 MHz */
}

static void gpio_init(void)
{
    /* PA2 output push-pull high: both LEDs off */
    GPIOA_BSHR = 1u << 2;
    GPIOA_CFGLR = (GPIOA_CFGLR & ~(0xFu << 8)) | (0x1u << 8);

    /* PC1, PC2, PC4 input with internal pull-up (buttons to GND) */
    uint32_t cfg = GPIOC_CFGLR;
    cfg &= ~((0xFu << 4) | (0xFu << 8) | (0xFu << 16));
    cfg |= (0x8u << 4) | (0x8u << 8) | (0x8u << 16);
    GPIOC_CFGLR = cfg;
    GPIOC_BSHR = (1u << 1) | (1u << 2) | (1u << 4);

    /* PD6 is not connected: pull it up so it cannot float */
    uint32_t dcfg = GPIOD_CFGLR;
    dcfg &= ~(0xFu << 24);
    dcfg |= (0x8u << 24);
    GPIOD_CFGLR = dcfg;
    GPIOD_BSHR = 1u << 6;
}

static void systick_init(void)
{
    SYSTICK_CTLR = 0x5;  /* enable counter, clock = HCLK */
}

static void pwm_init(void)
{
    TIM1_CCER = 0x0040;      /* CC2NE: complementary output 2 enable */
    TIM1_CHCTLR1 = 0x7000;   /* OC2M = PWM mode 2 */
    TIM1_BDTR = 0x8000;      /* MOE: main output enable */
    TIM1_ATRLR = 38;         /* 1.5 MHz / 39 = 38.46 kHz carrier */
    TIM1_CH2CVR = 10;        /* 10/39 = 25.6 % LED on-time */
    TIM1_SWEVGR = 0x0001;    /* update generation */
    TIM1_CTLR1 = 0x0081;     /* ARPE | CEN */
}

static void exti_init(void)
{
    AFIO_EXTICR = (AFIO_EXTICR & ~((3u << 2) | (3u << 4) | (3u << 8)))
                | ((2u << 2) | (2u << 4) | (2u << 8));  /* lines 1,2,4 -> port C */
    EXTI_EVENR |= KEY_MASK;  /* wake the core with an event, no ISR needed */
    EXTI_FTENR |= KEY_MASK;
}

static void hw_init(void)
{
    clock_init();
    RCC_APB2PCENR |= RCC_AFIOEN | RCC_IOPAEN | RCC_IOPCEN | RCC_IOPDEN | RCC_TIM1EN;
    RCC_APB1PCENR |= RCC_PWREN;
    gpio_init();
    systick_init();
    pwm_init();
    exti_init();
    ir_off();
}

static void standby(void)
{
    EXTI_INTFR = KEY_MASK;   /* drop edges latched while awake */

    PWR_CTLR |= PWR_PDDS;    /* PDDS: deep sleep = standby */
    /* CH32V003 has no WFE; ch32fun maps wfi to event wait in SCTLR.
       The extra bit-5 pulse is part of that well-tested sequence. */
    uint32_t sctlr = PFIC_SCTLR;
    PFIC_SCTLR |= PFIC_SLEEPDEEP | PFIC_WFITOWFE | (1u << 5);
    PFIC_SCTLR = (PFIC_SCTLR & ~(1u << 5)) | (sctlr & (1u << 5));

    __asm__ volatile("wfi" ::: "memory");
    __asm__ volatile("wfi" ::: "memory");

    PFIC_SCTLR = sctlr;
    PWR_CTLR &= ~PWR_PDDS;
}

static uint8_t pressed_cmd(void)
{
    uint32_t pins = GPIOC_INDR;
    if (!(pins & (1u << 4)))
        return NEC_CMD_SW1;
    if (!(pins & (1u << 2)))
        return NEC_CMD_SW2;
    if (!(pins & (1u << 1)))
        return NEC_CMD_SW3;
    return 0;
}

void main(void)
{
    hw_init();
    delay_ticks(FLASH_WINDOW_TICKS);

    for (;;) {
        standby();
        /* Re-running init is harmless if state survived standby and
           mandatory if the clock tree was reset on wake. */
        hw_init();
        delay_ticks(DEBOUNCE_TICKS);

        uint8_t cmd;
        while ((cmd = pressed_cmd()) != 0) {
            nec_send(cmd);
            delay_ticks(REPEAT_GAP_TICKS);
        }

        ir_off();
    }
}
