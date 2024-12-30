/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiSysCallStdThread.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     waits on a process for inputs
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/

import java.io.*;
import javax.swing.*;

/*===========================================================================
** CCBCLguiSysCallStdThread
**===========================================================================*/
public class CCBCLguiSysCallStdThread extends Thread
{
    JTextArea                           writeTo;
    private BufferedReader              brStdout;
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  p:      - process to watch STDIN for    
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiSysCallStdThread(JTextArea txt, BufferedReader rdr)
    {
        super();
        writeTo = txt;
        brStdout = rdr;
    }
    
    /*===========================================================================
    ** run:     thread that makes the system calls
    ** 
    ** Inputs:  none    
    **
    ** Returns: none 
    **===========================================================================*/
    public void run()
    {
        int caretpos = 0;
        int charsRead = 0;
        char[] buf = new char[1024];
        try{
            
            System.out.println("Process Input Stream Started");
            
            String line = null;
//            line = brStdout.readLine();
            do {
                
                charsRead = brStdout.read(buf, 0, 1024);
                
                if (charsRead == 0) {
                    do {
                        this.sleep(500);
                    } while((charsRead = brStdout.read(buf, 0, 1024)) == 0);
                }
                                
                line = new String(buf, 0, charsRead);
                caretpos = line.length() + writeTo.getText().length();
                writeTo.append(line);
                writeTo.setCaretPosition(caretpos);
                brStdout.mark(charsRead);
                brStdout.reset();
//                line = brStdout.readLine();
            }while (line != null); 
            System.out.println("Process Input Stream Finished");
            
                    
        }catch(Exception e)
        {
            System.out.println("Error Process Input Stream:\n" + e.getMessage());
        }
        
    }
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.2  2003/04/17 22:09:19  HoltyB
** TBolt00000000:  Changes for new help text layout in CCBcl.pl.
**
** Revision 1.1  2002/02/19 16:31:15  HoltyB
** Added many new features
** AsyncServer
** DebugConsole
**
** Revision 1.1  2002/02/15 22:49:51  HoltyB
** initial integration
**
****************************************************************************/