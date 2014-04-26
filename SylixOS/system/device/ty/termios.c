/*********************************************************************************************************
**
**                                    �й������Դ��֯
**
**                                   Ƕ��ʽʵʱ����ϵͳ
**
**                                SylixOS(TM)  LW : long wing
**
**                               Copyright All Rights Reserved
**
**--------------�ļ���Ϣ--------------------------------------------------------------------------------
**
** ��   ��   ��: termios.c
**
** ��   ��   ��: Han.Hui (����)
**
** �ļ���������: 2012 �� 01 �� 10 ��
**
** ��        ��: sio -> termios ���޼��ݿ�.
*********************************************************************************************************/
#define  __SYLIXOS_KERNEL
#include "SylixOS.h"
#include "termios.h"
/*********************************************************************************************************
  �ü���
*********************************************************************************************************/
#if (LW_CFG_DEVICE_EN > 0) && (LW_CFG_SIO_DEVICE_EN > 0)
/*********************************************************************************************************
** ��������: cfgetispeed
** ��������: ��� termios �ṹ�е����벨����
** �䡡��  : tp     termios �ṹ
** �䡡��  : ������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
speed_t cfgetispeed (const struct termios *tp)
{
    return  (tp->c_cflag & CBAUD);
}
/*********************************************************************************************************
** ��������: cfgetospeed
** ��������: ��� termios �ṹ�е����������
** �䡡��  : tp     termios �ṹ
** �䡡��  : ������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
speed_t cfgetospeed (const struct termios *tp)
{
    return  (tp->c_cflag & CBAUD);
}
/*********************************************************************************************************
** ��������: cfsetispeed
** ��������: ���� termios �ṹ�е����벨����
** �䡡��  : tp     termios �ṹ
**           speed  ������
** �䡡��  : ���ý��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int cfsetispeed (struct termios *tp, speed_t speed)
{
    if (speed & ~CBAUD) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    tp->c_cflag |= speed;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: cfsetospeed
** ��������: ���� termios �ṹ�е����������
** �䡡��  : tp     termios �ṹ
**           speed  ������
** �䡡��  : ���ý��
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int cfsetospeed (struct termios *tp, speed_t speed)
{
    if (speed & ~CBAUD) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    tp->c_cflag |= speed;
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: tcdrain
** ��������: �ȴ�ָ�����ļ����������ݷ������
** �䡡��  : fd      �ļ�������
** �䡡��  : �ȴ����
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  tcdrain (int  fd)
{
    INT     iRet;

    __THREAD_CANCEL_POINT();                                            /*  ����ȡ����                  */

    iRet = ioctl(fd, FIOSYNC, 0);                                       /*  ����������ͬ�����          */
    
    return  (iRet);
}
/*********************************************************************************************************
** ��������: tcflow
** ��������: �����豸���, Ŀǰ��֧��
** �䡡��  : fd      �ļ�������
**           action  ��Ϊ
** �䡡��  : ���
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  tcflow (int  fd, int  action)
{
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: tcflush
** ��������: ����豸����
** �䡡��  : fd      �ļ�������
**           queue   ��Ϊ
** �䡡��  : ���
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  tcflush (int  fd, int  queue)
{
    switch (queue) {
    
    case TCIFLUSH:
        return  (ioctl(fd, FIORFLUSH, 0));
    
    case TCOFLUSH:
        return  (ioctl(fd, FIOWFLUSH, 0));
    
    case TCIOFLUSH:
        return  (ioctl(fd, FIOFLUSH, 0));
    
    default:
        errno = EINVAL;
        return  (PX_ERROR);
    }
}
/*********************************************************************************************************
** ��������: tcgetsid
** ��������: ��õ�ǰ�����Ự���� (��֧��)
** �䡡��  : fd      �ļ�������
** �䡡��  : ����������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
pid_t tcgetsid (int fd)
{
    errno = ENOSYS;
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: tcsendbreak
** ��������: ���������� 0 ֵ������������һ��ʱ�䣬����ն�ʹ���첽�������ݴ���Ļ������ duration �� 0��
             �����ٴ��� 0.25 �룬���ᳬ�� 0.5 �롣��� duration ���㣬�����͵�ʱ�䳤����ʵ�ֶ��塣
** �䡡��  : fd      �ļ�������
**           duration
** �䡡��  : ����������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int tcsendbreak (int  fd, int  duration)
{
    errno = ENOSYS;
    return  (PX_ERROR);
}
/*********************************************************************************************************
** ��������: tcgetattr
** ��������: ���������� 0 ֵ������������һ��ʱ�䣬����ն�ʹ���첽�������ݴ���Ļ������ duration �� 0��
             �����ٴ��� 0.25 �룬���ᳬ�� 0.5 �롣��� duration ���㣬�����͵�ʱ�䳤����ʵ�ֶ��塣
** �䡡��  : fd      �ļ�������
**           tp      termios �ṹ
** �䡡��  : ����������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  tcgetattr (int  fd, struct termios *tp)
{
    LONG    lOpt = 0;
    LONG    lHwOpt = 0;
    LONG    lBaud = 0;
    INT     iError;
    
    if (!tp) {
        errno = EINVAL;
        return  (PX_ERROR);
    }
    
    lib_bzero(tp, sizeof(struct termios));
    
    iError = ioctl(fd, FIOGETOPTIONS, &lOpt);
    if (iError < ERROR_NONE) {
        return  (iError);
    }
    
    ioctl(fd, SIO_HW_OPTS_GET, &lHwOpt);                                /*  �����������������������ṩ  */
    ioctl(fd, SIO_BAUD_GET, &lBaud);

    iError = ioctl(fd, FIOGETCC, tp->c_cc);
    if (iError < ERROR_NONE) {
        return  (iError);
    }
    
    if (lOpt & OPT_ECHO) {
        tp->c_lflag |= (ECHO | ECHOE);
    }
    if (lOpt & OPT_CRMOD) {
        tp->c_iflag |= (ICRNL | INLCR);
        tp->c_oflag |= ONLCR;
    }
    if (lOpt & OPT_TANDEM) {
        tp->c_iflag |= (IXON | IXOFF);
    }
    if (lOpt & OPT_MON_TRAP) {
        tp->c_iflag |= BRKINT;
    }
    if (lOpt & OPT_ABORT) {
        tp->c_iflag |= BRKINT;
    }
    if (lOpt & OPT_LINE) {
        tp->c_lflag |= (ICANON | ECHOK);
    }
    
    if ((lHwOpt & CLOCAL) == 0) {
        tp->c_cflag |= CRTSCTS;
    } else {
        tp->c_cflag |= CLOCAL;
    }
    if (lHwOpt & CREAD) {
        tp->c_cflag |= CREAD;
    }
    
    tp->c_cflag |= (unsigned int)(lHwOpt & CSIZE);
    
    if (lHwOpt & HUPCL) {
        tp->c_cflag |= HUPCL;
    }
    if (lHwOpt & STOPB) {
        tp->c_cflag |= STOPB;
    }
    if (lHwOpt & PARENB) {
        tp->c_cflag |= PARENB;
    }
    if (lHwOpt & PARODD) {
        tp->c_cflag |= PARODD;
    }
    
    switch (lBaud) {
    
    case 50:
        tp->c_cflag |= B50;
        break;
        
    case 75:
        tp->c_cflag |= B75;
        break;
        
    case 110:
        tp->c_cflag |= B110;
        break;
        
    case 134:
        tp->c_cflag |= B134;
        break;
    
    case 150:
        tp->c_cflag |= B150;
        break;
    
    case 200:
        tp->c_cflag |= B200;
        break;
        
    case 300:
        tp->c_cflag |= B300;
        break;
        
    case 600:
        tp->c_cflag |= B600;
        break;
    
    case 1200:
        tp->c_cflag |= B1200;
        break;
    
    case 1800:
        tp->c_cflag |= B1800;
        break;
    
    case 2400:
        tp->c_cflag |= B2400;
        break;
        
    case 4800:
        tp->c_cflag |= B4800;
        break;
    
    case 9600:
        tp->c_cflag |= B9600;
        break;
    
    case 19200:
        tp->c_cflag |= B19200;
        break;
    
    case 38400:
        tp->c_cflag |= B38400;
        break;
    
    case 57600:
        tp->c_cflag |= B57600;
        break;
    
    case 115200:
        tp->c_cflag |= B115200;
        break;
    
    case 230400:
        tp->c_cflag |= B230400;
        break;
    
    case 460800:
        tp->c_cflag |= B460800;
        break;
    
    default:
        tp->c_cflag |= B0;
        break;
    }
    
    return  (ERROR_NONE);
}
/*********************************************************************************************************
** ��������: tcgetattr
** ��������: ���������� 0 ֵ������������һ��ʱ�䣬����ն�ʹ���첽�������ݴ���Ļ������ duration �� 0��
             �����ٴ��� 0.25 �룬���ᳬ�� 0.5 �롣��� duration ���㣬�����͵�ʱ�䳤����ʵ�ֶ��塣
** �䡡��  : fd      �ļ�������
**           opt     ѡ�� TCSANOW, TCSADRAIN, TCSAFLUSH
**           tp      termios �ṹ
** �䡡��  : ����������
** ȫ�ֱ���: 
** ����ģ��: 
                                           API ����
*********************************************************************************************************/
LW_API  
int  tcsetattr (int  fd, int  opt, const struct termios  *tp)
{
    LONG    lOpt = 0;
    LONG    lHwOpt = 0;
    LONG    lBaud = 0;
    
    LONG    lOptOld = 0;
    LONG    lHwOptOld = 0;
    LONG    lBaudOld = 0;
    INT     iError;

    if (opt == TCSADRAIN) {
        iError = ioctl(fd, FIOSYNC, 0);
        if (iError < ERROR_NONE) {
            return  (iError);
        }
    
    } else if (opt == TCSAFLUSH) {
        iError = ioctl(fd, FIOSYNC, 0);
        if (iError < ERROR_NONE) {
            return  (iError);
        }
        iError = ioctl(fd, FIOFLUSH, 0);
        if (iError < ERROR_NONE) {
            return  (iError);
        }
    }

    if (tp->c_lflag & ECHO) {
        lOpt |= OPT_ECHO;
    }
    if ((tp->c_iflag & ICRNL) ||
        (tp->c_oflag & ONLCR)) {
        lOpt |= OPT_CRMOD;
    }
    if (tp->c_iflag & IXON) {
        lOpt |= OPT_TANDEM;
    }
    if (tp->c_iflag & BRKINT) {
        lOpt |= OPT_ABORT;
        lOpt |= OPT_MON_TRAP;
    }
    if (tp->c_lflag & ICANON) {
        lOpt |= OPT_LINE;
    }
    
    if (tp->c_cflag & CLOCAL) {
        lHwOpt |= CLOCAL;
    }
    if (tp->c_cflag & CRTSCTS) {
        lHwOpt &= ~CLOCAL;
    }
    if (tp->c_cflag & CREAD) {
        lHwOpt |= CREAD;
    }
    
    lHwOpt |= (LONG)(tp->c_cflag & CSIZE);
    
    if (tp->c_cflag & HUPCL) {
        lHwOpt |= HUPCL;
    }
    if (tp->c_cflag & STOPB) {
        lHwOpt |= STOPB;
    }
    if (tp->c_cflag & PARENB) {
        lHwOpt |= PARENB;
    }
    if (tp->c_cflag & PARODD) {
        lHwOpt |= PARODD;
    }
    
    switch (tp->c_cflag & CBAUD) {
    
    case B0:
        lBaud = 0;
        break;
        
    case B50:
        lBaud = 50;
        break;
        
    case B75:
        lBaud = 75;
        break;
        
    case B110:
        lBaud = 110;
        break;
        
    case B134:
        lBaud = 134;
        break;
        
    case B150:
        lBaud = 150;
        break;
        
    case B200:
        lBaud = 200;
        break;
        
    case B300:
        lBaud = 300;
        break;
        
    case B600:
        lBaud = 600;
        break;
        
    case B1200:
        lBaud = 1200;
        break;
        
    case B1800:
        lBaud = 1800;
        break;
        
    case B2400:
        lBaud = 2400;
        break;
        
    case B4800:
        lBaud = 4800;
        break;
        
    case B9600:
        lBaud = 9600;
        break;
        
    case B19200:
        lBaud = 19200;
        break;
        
    case B38400:
        lBaud = 38400;
        break;
        
    case B57600:
        lBaud = 57600;
        break;
        
    case B115200:
        lBaud = 115200;
        break;
        
    case B230400:
        lBaud = 230400;
        break;
        
    case B460800:
        lBaud = 460800;
        break;
        
    default:
        lBaud = 9600;
        break;
    }
    
    iError = ioctl(fd, FIOGETOPTIONS, &lOptOld);
    if (iError < ERROR_NONE) {
        return  (iError);
    }
    if (lOptOld != lOpt) {
        iError = ioctl(fd, FIOSETOPTIONS, lOpt);
        if (iError < ERROR_NONE) {
            return  (iError);
        }
    }
    
    iError = ioctl(fd, SIO_HW_OPTS_GET, &lHwOptOld);                    /*  pty û�д˹���               */
    if ((iError >= ERROR_NONE) && (lHwOptOld != lHwOpt)) {
        ioctl(fd, SIO_HW_OPTS_SET, lHwOpt);
    }
    
    iError = ioctl(fd, SIO_BAUD_GET, &lBaudOld);                        /*  pty û�д˹���               */
    if ((iError >= ERROR_NONE) && (lBaudOld != lBaud)) {
        ioctl(fd, SIO_BAUD_SET, lBaud);
    }
    
    iError = ioctl(fd, FIOSETCC, tp->c_cc);
    if (iError < ERROR_NONE) {
        return  (iError);
    }
    
    return  (ERROR_NONE);
}
#endif                                                                  /*  (LW_CFG_DEVICE_EN > 0) &&   */
                                                                        /*  (LW_CFG_SIO_DEVICE_EN > 0)  */
/*********************************************************************************************************
  END
*********************************************************************************************************/
