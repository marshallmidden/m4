/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiServer.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     shows th async server
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/

import java.io.*;
import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

public class CCBCLguiServer extends JDialog implements Runnable
{
    public CCBCLguiServer(Frame parent, String Location, String cmd, int option)
    {
        super (parent,false);
        setTitle("Server View for " + cmd);
        loc = new String(Location);
        command = new String(cmd);
        us = this;
        mode = option;
        initFrame();
    }
    
    public void initFrame()
    {
        Container tCon = this.getContentPane();
        Container gridCon = null;
        
        if (mode == 0)
        {
            gridCon = new JPanel(new GridLayout(1,4,0,0));
        }
        else if (mode == 1)
        {
            gridCon = new JPanel(new GridLayout(1,4,0,0));
        }
        
        msgs = new JTextArea();
        msgs.setEditable(false);
        
        JButton clsBtn = new JButton("CLOSE");
        clsBtn.setToolTipText("Close this window");
        clsBtn.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent a1)
            {
                us.hide();
            }
        });
        gridCon.add(clsBtn);
        
        if (mode == 0)
        {
            JButton stpBtn = new JButton("STOP");
            stpBtn.setToolTipText("Stop this server");
            stpBtn.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent a1)
                {
                    killServer();
                }
            });
            gridCon.add(stpBtn);
        }
        else if (mode == 1)
        {
            JButton mtBtn = new JButton("CLEAR FILE");
            mtBtn.setToolTipText("Deletes Contents of file and clears the screen causes ccbcl.pl to restart");
            mtBtn.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent a1)
                {
                    renewFile();
                }
            });
            gridCon.add(mtBtn);
        }    
        
        JButton refBtn = new JButton("REFRESH");
        refBtn.setToolTipText("Stop this server, and restart it");
        refBtn.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent a1)
            {
                killServer();
                initServer();
            }
        });
        gridCon.add(refBtn);
        
        JButton clrBtn = new JButton("CLEAR");
        clrBtn.setToolTipText("Clear this window");
        clrBtn.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent a1)
            {
                msgs.setText("");
            }
        });
        gridCon.add(clrBtn);
        
        tCon.add(new JScrollPane(msgs), BorderLayout.CENTER);
        tCon.add(gridCon, BorderLayout.SOUTH);    
    }
    
    public void initServer()
    {
        Thread serverThread = new Thread(this);
        serverThread.start();   
    }
    
    public void run()
    {
        if (p1 != null)
        {
            p1.destroy();
        }
            
        try{
            if (mode == 0)
            {
                rt = Runtime.getRuntime();
            
                p1 = rt.exec(command, null, new File(loc));   
            
                InputStreamReader isrStdout = new InputStreamReader( p1.getInputStream() );
                BufferedReader brStdout = new BufferedReader( isrStdout );
                CCBCLguiSysCallStdThread stThread = new CCBCLguiSysCallStdThread(msgs, brStdout);
            
                InputStreamReader isrStderr = new InputStreamReader( p1.getErrorStream() );
                BufferedReader brStderr = new BufferedReader( isrStderr );
                CCBCLguiSysCallStdThread erThread = new CCBCLguiSysCallStdThread(msgs, brStderr);
            
                Thread proc2 = new Thread(stThread);
                proc2.setPriority(Thread.MAX_PRIORITY);
                
                Thread proc3 = new Thread(erThread);
                proc3.setPriority(Thread.MAX_PRIORITY);
            
                proc3.start();
                proc2.start();
            
                System.out.println("Server " + command + " running");
            
                proc3.join();
                proc2.join();
    
                System.out.println("Server " + command + " not running");
            }
            
            else if (mode == 1)
            {
                byte[] arr;
                FileInputStream inp = new FileInputStream(loc + "\\" + command);
                arr = new byte[inp.available()];
                inp.read(arr);
                msgs.setText(new String(arr));
                inp.close();
            }
    
        }catch(Exception e)
        {
            if (mode == 0)
            {
                System.out.println("Unable to create thread "+ 
                                    loc + "\\" + command);
            }
            else if (mode == 1)
            {
                System.out.println("Unable to open file "+ 
                                    loc + "\\" + command);
            }
            
            System.out.println(e.getMessage());
            e.printStackTrace();
        }
    }
    
    public void killServer()
    {
        if (p1 != null)
        {
            p1.destroy();
            p1 = null;
        }
    }
    
    public void renewFile()
    {
        boolean good = false;
        try{
            File tmpFile = new File(loc + "\\" + command);
            CCBCLguiSysCall.restartProcess(null);
            if (tmpFile.delete())
            {
                good = tmpFile.createNewFile();
            }
        }catch (Exception e)
        {
            System.out.println("Unable to recreate file" + loc + "\\" + command);
        }
            
        if (!good)
        {
            msgs.setText("Unable to renew file" + loc + "\\" + command);
        }
        else
        {
            msgs.setText("");
        }
    }
    
    public void finalize()
    {
        killServer();
        this.dispose();
    }
        
    private int mode;
    private JDialog us;
    private Runtime rt;
    private Process p1;
    private String loc;
    private String command;
    private JTextArea msgs;
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.3  2002/02/22 20:12:20  HoltyB
** Added additional exception handling
**
** Revision 1.2  2002/02/21 22:12:05  HoltyB
** made changes to allow to save window settings
** added ccbcl.log view
**
** Revision 1.1  2002/02/19 19:20:53  HoltyB
** initial integration
**
****************************************************************************/