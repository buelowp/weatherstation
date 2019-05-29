#ifndef __PIN_HANDLER__
#define __PIN_HANDLER__

#include <GPIOPin.h>
#include <GPIOAccess.h>

class Geiger_Pin_Handler_Object : public GPIO_Irq_Handler_Object {
public:
    Geiger_Pin_Handler_Object(GPIO_Irq_Type type) {
        irqType = type;
        m_count = 0;
    }

    void handleIrq(int pinNum, GPIO_Irq_Type type) {
        if (type == irqType) {
        	m_count++;
        }
    }

    unsigned int count() { return m_count; }
    unsigned int reset() { m_count = 0; }
    
private:
    GPIO_Irq_Type irqType;
    unsigned int m_count;
};

#endif

