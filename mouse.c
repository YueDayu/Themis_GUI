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
  int x_overflow, y_overflow;
  uint tick;
} packet;
static int count;
static int recovery;
static int lastbtn, lastdowntick, lastclicktick;

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
    lastclicktick = lastdowntick = -1000;
}

void genMouseUpMessage(int btns)
{
  message msg;
  msg.msg_type = M_MOUSE_UP;
  msg.params[0] = btns;
  handleMessage(&msg);
}

void
genMouseMessage()
{
  if (packet.x_overflow || packet.y_overflow) return;
	int x = packet.x_sgn ? (0xffffff00 | (packet.x_mov & 0xff)) : (packet.x_mov & 0xff);
	int y = packet.y_sgn ? (0xffffff00 | (packet.y_mov & 0xff)) : (packet.y_mov & 0xff);
/*	if(x == 127 || x == -127 || y == 127 || y == -127){
		x = 0;
		y = 0;
	}*/
	packet.x_mov = x;
	packet.y_mov = y;
	
  int btns = packet.l_btn | (packet.r_btn << 1) | (packet.m_btn << 2);
  message msg;
  if (packet.x_mov || packet.y_mov)
  {
    msg.msg_type = M_MOUSE_MOVE;
    msg.params[0] = packet.x_mov;
    msg.params[1] = packet.y_mov;
    msg.params[2] = btns;
    lastdowntick = lastclicktick = -1000;
    if (btns != lastbtn) genMouseUpMessage(btns);
  }
  else if (btns)
  {
    msg.msg_type = M_MOUSE_DOWN;
    msg.params[0] = btns;
    lastdowntick = packet.tick;
  }
  else if (packet.tick - lastdowntick < 30)
  {
    if (lastbtn & 1) msg.msg_type = M_MOUSE_LEFT_CLICK;
    else msg.msg_type = M_MOUSE_RIGHT_CLICK;
    if (packet.tick - lastclicktick < 60)
    {
      msg.msg_type = M_MOUSE_DBCLICK;
      lastclicktick = -1000;
    }
    else lastclicktick = packet.tick;
  }
  else
  {
    genMouseUpMessage(btns);
  }
  lastbtn = btns;
  handleMessage(&msg);
}


void
mouseintr(uint tick)
{
  acquire(&mouselock);
  int state;
  while (((state = inb(0x64)) & 1) == 1) {
    int data = inb(0x60);
    count++;

	  if (recovery == 0 && (data & 255) == 0)
		  recovery = 1;
	  else if (recovery == 1 && (data & 255) == 0)
		  recovery = 2;
	  else if ((data & 255) == 12)
		  recovery = 0;
	  else
		  recovery = -1;

    switch(count)
    {
        case 1: if(data & 0x08)
                {
                    packet.y_overflow = (data >> 7) & 0x1;
                    packet.x_overflow = (data >> 6) & 0x1;
                    packet.y_sgn = (data >> 5) & 0x1;
                    packet.x_sgn = (data >> 4) & 0x1;
                    packet.m_btn = (data >> 2) & 0x1;
                    packet.r_btn = (data >> 1) & 0x1;
                    packet.l_btn = (data >> 0) & 0x1;
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
                 packet.tick = tick;
                 break;
        default: count=0;    break;
    }

	  if (recovery == 2)
	  {
		  count = 0;
		  recovery = -1;
	  }
	  else if (count == 3)
	  {
		  count = 0;
		  genMouseMessage();
	  }
	}

  release(&mouselock);
}



