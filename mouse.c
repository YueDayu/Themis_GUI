#include "types.h"
#include "x86.h"
#include "defs.h"
#include "msg.h"
#include "spinlock.h"
#include "traps.h"

static struct spinlock mouselock;
static struct {
  int x_sgn, y_sgn, x_mov, y_mov;
  int l_btn, r_btn, m_btn;
} packet;
static int count;

void
mouse_wait(uchar type)
{
    uint time_out = 100000;
    if(type == 0)
    {
        while(--time_out)
        {
            if((inb(0x64) & 1) == 1)
                return;
        }
    }
    else
    {
        while(--time_out)
        {
            if((inb(0x64) & 2) == 0)
                return;
        }   
    }
}

void
mouse_write(uchar word)
{
    mouse_wait(1);
    outb(0x64, 0xd4);
    mouse_wait(1);
    outb(0x60, word);
}

uint
mouse_read()
{
    mouse_wait(0);
    return inb(0x60);
}

void
mouseinit(void)
{
    uchar statustemp;

    mouse_wait(1);
    outb(0x64, 0xa8);		//激活鼠标接口
    
    mouse_wait(1);		//激活中断
    outb(0x64, 0x20);
    mouse_wait(0);
    statustemp = (inb(0x60) | 2);
    mouse_wait(0);
    outb(0x64, 0x60);
    mouse_wait(1);
    outb(0x60, statustemp);

    mouse_write(0xf6);		//设置鼠标为默认设置
    mouse_read();

    mouse_write(0xf3);		//设置鼠标采样率
    mouse_read();
    mouse_write(10);
    mouse_read();
 
    mouse_write(0xf4);
    mouse_read();    

    initlock(&mouselock, "mouse");
    picenable(IRQ_MOUSE);
    ioapicenable(IRQ_MOUSE, 0);
    
    count = 0;
}

void
genMouseMessage()
{
  if (packet.x_sgn) packet.x_mov -= 256;
  if (packet.y_sgn) packet.y_mov -= 256;
  int btns = packet.l_btn | (packet.r_btn << 1) | (packet.m_btn << 2);
  message msg;
  if (packet.x_mov || packet.y_mov)
  {
    msg.msg_type = M_MOUSE_MOVE;
    msg.params[0] = packet.x_mov;
    msg.params[1] = packet.y_mov;
    msg.params[2] = btns;
  }
  else if (btns)
  {
    msg.msg_type = M_MOUSE_DOWN;
    msg.params[0] = btns;
  }
  else
  {
    msg.msg_type = M_MOUSE_UP;
    msg.params[0] = btns;
  }
  handleMessage(&msg);
}


void
mouseintr(void)
{
  acquire(&mouselock);
  int data = inb(0x60);
  count++;

  switch(count)
  {
      case 1: if(data & 0x08)
              {
                  packet.y_sgn = data >> 5 & 0x1;
                  packet.x_sgn = data >> 4 & 0x1;
                  packet.m_btn = data >> 2 & 0x1;
                  packet.r_btn = data >> 1 & 0x1;
                  packet.l_btn = data >> 0 & 0x1;
                  break;
              }
              else
              {
                  count = 0;
                  break;
              }
               
      case 2:  packet.x_mov = data;
               break;
      case 3:  packet.y_mov = data;  
               count = 0;
               genMouseMessage();
               break;
      default: count=0;    break;
  }

  release(&mouselock);
}



