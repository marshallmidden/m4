/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiSysCall.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     Threads off the system call
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/

import java.io.*;
import javax.swing.Timer;
import java.awt.event.*;
import javax.swing.*;

/*===========================================================================
** CCBCLguiSysCall
**===========================================================================*/
public class CCBCLguiSysCall extends Thread
{
    private static String       CLlocation;
    private static long         timeout;
    private static Timer        tmr;
    private static Process      p1 = null;
    private static Runtime      rt = null;
    private static CCBCLguiSysCallStdThread  stThread;
    private static CCBCLguiSysCallStdThread  erThread;
    private static JTextArea    txt;
    
    //private static String       command;
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  location:   - location of the ccbcl.pl    
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiSysCall()
    {
        super();
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
        try{
            if (p1 != null)
            {
                p1.destroy();
            }
            
            p1 = rt.exec("perl ccbcl.pl", null, new File(CLlocation));   
            
            InputStreamReader isrStdout = new InputStreamReader( p1.getInputStream() );
            BufferedReader brStdout = new BufferedReader( isrStdout );
            stThread = new CCBCLguiSysCallStdThread(txt, brStdout);
            
            InputStreamReader isrStderr = new InputStreamReader( p1.getErrorStream() );
            BufferedReader brStderr = new BufferedReader( isrStderr );
            erThread = new CCBCLguiSysCallStdThread(txt, brStderr);
            
            Thread proc2 = new Thread(stThread);
            proc2.setPriority(Thread.MAX_PRIORITY);
            
            Thread proc3 = new Thread(erThread);
            proc3.setPriority(Thread.MAX_PRIORITY);
            
            proc3.start();
            proc2.start();
            
            System.out.println("Server perl ccbcl.pl running");
            
            proc3.join();
            proc2.join();
            
            System.out.println("Server perl ccbcl.pl not running");
            
        }catch(Exception e)
        {
            System.out.println("Unable to create thread\n" + e.getMessage());
        }
        
        
        
    }
    
    /*===========================================================================
    ** SysCall:     static function that creates a new thread and starts it
    ** 
    ** Inputs:      timeout     - timeout of the thread
    **                            (not currently used)    
    **
    ** Returns: none 
    **===========================================================================*/
    public static void SysCall (String command, long tmt)
    {
        boolean quit = false;
        FileInputStream     result;
        int                 sizeOfInput;
        byte[]              buffr;
        String              cmd;
        String              returnString = null;
        String              results = new String("result.txt");
        
        /* start a timer to kill the process */
        tmr = new Timer((int)tmt, new ActionListener()
        {
            public void actionPerformed(ActionEvent ae)
            {
                restartProcess(CLlocation);
            }
        });
        tmr.start();

        System.out.println("Process Output Stream Started");
        
        /* tell CCBCLguiFrame that we are running */
        CCBCLguiFrame.incrementProc();
           
        try{
            
            OutputStream os = p1.getOutputStream();

            int ind1 = 0;
            int ind2 = 0;
            
            while (!quit)
            {
                try{
                    //System.out.println(command);
                    ind2 = command.indexOf('\n');
                    if (ind2 == -1)
                    {
                        quit = true;
                    }
                    else
                    {
                        cmd = command.substring(ind1, (ind2 + 1));
                        command = command.substring(ind2 + 1);            
                        //System.out.println(cmd);
                        os.write(cmd.getBytes());
                        os.flush();
                    }
                }
                catch (Exception e)
                {
                    quit = true;
                }
            }
            
            //System.out.println("Out of loop");
            
            if (tmr.isRunning())
            {
                tmr.stop();
            }
            
            CCBCLguiFrame.decrementProc();
            
            System.out.println("Process Output Stream Finished");
        
        }catch(Exception e)
        {
            CCBCLguiFrame.decrementProc();
            System.out.println("Error Process Output Stream:\n" + e.getMessage());
        }
        
    }
    
    /*===========================================================================
    ** initSysCall      MUST BE CALLED BEFORE USING THIS CLASS 
    **
    ** Inputs:  loc:   - location of the ccbcl.pl    
    **
    ** Returns: none 
    **===========================================================================*/
    public static void initSysCall(String loc, JTextArea writeTo)
    {
        if (loc == null)
        {
            CLlocation = new String("c:\\projects\\bigfoot\\ccbe");
        }
        else
        {
            CLlocation = new String(loc);
        }    
        
        if (rt == null)
        {
            rt = Runtime.getRuntime();
        }
        
        txt = writeTo;
        
        /*
        ** create a new thread and run it
        */
        CCBCLguiSysCall runner = new CCBCLguiSysCall();
        
        try{
            Thread proc = new Thread(runner);
            proc.setPriority(Thread.MAX_PRIORITY);
            proc.start();
        }catch(Exception e)
        {
            System.out.println("Unable to create thread\n" + e.getMessage());
        }
    }
    
    /*===========================================================================
    ** killProcess      kills the process in this class 
    **
    ** Inputs:  none    
    **
    ** Returns: none 
    **===========================================================================*/
    public static void killProcess()
    {
        try{
            if (p1 != null)
            {
                p1.destroy();
                p1 = null;
                System.out.println("***********PROCESS DEAD************");
            }
        }catch(Exception e)
        {
            System.out.println("Unable to destroy Thread\n" + e.getMessage());
        }
    }
    
    /*===========================================================================
    ** restartProcess      restarts the process in this class 
    **
    ** Inputs:  loc:   - location of the ccbcl.pl    
    **
    ** Returns: none 
    **===========================================================================*/
    public static void restartProcess(String loc)
    {
        if(loc == null)
        {
            loc = new String(CLlocation);
        }
        
        System.out.println("*********KILLING PROCESS***********");
        killProcess();
        CCBCLguiFrame.decrementProc();
        System.out.println("********RESTARTING PROCESS*********");
        initSysCall(loc, txt);
        System.out.println("*********PROCESS RESTARTED*********");
    }
    
    
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.5  2002/02/21 22:12:05  HoltyB
** made changes to allow to save window settings
** added ccbcl.log view
**
** Revision 1.4  2002/02/19 16:31:15  HoltyB
** Added many new features
** AsyncServer
** DebugConsole
**
** Revision 1.3  2002/02/15 23:32:33  HoltyB
** Major change to persistent connection to ccbcl.pl
**
** Revision 1.2  2002/02/13 19:38:53  HoltyB
** made changes for compatability with nt 4.0 added timeout
**
** Revision 1.1  2002/02/12 21:01:10  HoltyB
** initial integration of GUICCBCL
**
****************************************************************************/