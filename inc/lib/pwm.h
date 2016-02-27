#ifndef __MICROPY_INCLUDED_LIB_PWM_H__
#define __MICROPY_INCLUDED_LIB_PWM_H__

void pwm_start(void);
void pwm_stop(void);

int32_t pwm_set_period_us(int32_t us);
int32_t pwm_get_period_us(void);
int pwm_set_duty_cycle(int32_t pin, int32_t value);

#endif // __MICROPY_INCLUDED_LIB_PWM_H__