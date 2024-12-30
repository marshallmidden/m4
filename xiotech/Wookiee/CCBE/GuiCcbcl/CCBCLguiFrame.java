/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiFrame.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     Main JFrame for CCBCLGUI
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/

import javax.swing.*;
import java.awt.*;
import java.awt.event.*;
import java.util.*;
import java.io.*;

/*===========================================================================
** CCBCLguiFrame
**===========================================================================*/
public class CCBCLguiFrame extends JFrame
{
    
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  fnc:        - array of CCBCLguiFunctions
    **          loc:        - location of the ccbcl.pl    
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiFrame(CCBCLguiFunction[] fnc, String loc)
    {
        super("CCBCL GUI INTERFACE");
        title = new String("CCBCL GUI INTERFACE");
        funcs = fnc;
        location = loc;
        initVariables();
        initFrame();
    }
    
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  fnc:        - array of CCBCLguiFunctions
    **          loc:        - location of the ccbcl.pl
    **          width       - width of frame
    **          height      - height of frame    
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiFrame(CCBCLguiFunction[] fnc, String loc, int width, int height,
                         int x, int y)
    {
        super("CCBCL GUI INTERFACE");
        
        String tmpStr = null;
        File tmpFile = null;
        int index = 0;
        
        title = new String("CCBCL GUI INTERFACE");
        funcs = fnc;
        location = loc;
        this.setSize(width, height);
        this.setLocation(x, y);
        
        try{
            tmpFile = File.createTempFile("ccbgutmp", null);
            tmpFile.deleteOnExit();
            tmpStr = new String(tmpFile.getPath());
            index = tmpStr.lastIndexOf('\\');
            tmpStr = tmpStr.substring(0, index);
        }catch (Exception e){}
        
        ccblg = new CCBCLguiServer(this, tmpStr, "ccbCL.log",1);
        async = new CCBCLguiServer(this, loc, "perl ccbServer.pl",0);
        debug = new CCBCLguiServer(this, loc, "perl DebugConsole.pl",0);        
        
        async.setSize((int)(width  * .5), (int)(height * .50));
        async.setLocation((int)((width - (width * .50)) /2), 
                          (int)((height - (height * .50)) /2));
                                  
        debug.setSize((int)(width  * .5), (int)(height * .50));
        debug.setLocation((int)((width - (width * .50)) /2), 
                          (int)((height - (height * .50)) /2));
                
        ccblg.setSize((int)(width  * .5), (int)(height * .50));
        ccblg.setLocation((int)((width - (width * .50)) /2), 
                          (int)((height - (height * .50)) /2));
        
        async.initServer();
        debug.initServer();
        
        initVariables();
        initFrame();
        initSetup();
    }
    
    /*===========================================================================
    ** Constructor 
    **
    ** Inputs:  name:       - string to use as the program name
    **          fnc:        - array of CCBCLguiFunctions
    **          loc:        - location of the ccbcl.pl    
    **
    ** Returns: none 
    **===========================================================================*/
    public CCBCLguiFrame(String name, CCBCLguiFunction[] fnc, String loc)
    {
        super(name);
        title = new String(name);
        funcs = fnc;
        location = loc;
        initVariables();
        initFrame();
    }
    
    /*===========================================================================
    ** initVariables    initializes the private variables
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    public void initVariables()
    {
        /*
        ** Initialization
        */
        
        saveSettings        = true;
        savSetRec           = new String("SAVESETTINGS");
        ipRec               = new String("IPADDRESS");
        ipCntRec            = new String("IPCOUNT");
        hlpFntRec           = new String("HELPFONT");
        hlpBckRec           = new String("HELPBACKGROUND");
        hlpForRec           = new String("HELPFOREGROUND");
        outFntRec           = new String("OUTPUTFONT");
        outBckRec           = new String("OUTPUTBACKGROUND");
        outForRec           = new String("OUTPUTFOREGROUND");
        thsSizRec           = new String("THISSIZE");
        thsLocRec           = new String("THISLOCATION");
        asySizRec           = new String("ASYNCSIZE");
        asyLocRec           = new String("ASYNCLOCATION");
        asyVisRec           = new String("ASYNCVISIBLE");
        dbgSizRec           = new String("DEBUGSIZE");
        dbgLocRec           = new String("DEBUGLOCATION");
        dbgVisRec           = new String("DEBUGVISIBLE");
        ccbSizRec           = new String("CCBLGSIZE");
        ccbLocRec           = new String("CCBLGLOCATION");
        ccbVisRec           = new String("CCBLGVISIBLE");
        guiModRec           = new String("GUIMODE");
    
        mainPanel           = getContentPane();
        
        cmdList             = new Vector();
        cmdListPtr          = 0;
        scriptMode          = false;
        cmdMode             = false;
        currentIP           = new JLabel("");
        ipList              = new Vector(1,1);
        
        mainMenu            = null;
        commands            = null;
        fileMen             = null;
        ipMen               = null;
        opMen               = null;
        inputPanel          = null;
        parmPanel           = null;
        
        helpText            = new JTextArea("Help");
        scriptText          = new JTextArea("");
        outText             = new JTextArea("");
        outPane             = new JScrollPane(outText);
        
        parmLabels          = null;
        parmFields          = null;
        opts                = new JTextField("");
        
        currentCmd          = new JTextField("Welcome to the CCBCL GUI Implementation!");
        currentCmd.setEditable(false);
        currentCmd.setBackground(new Color(0,0,0));
        currentCmd.setForeground(new Color(0,255,0));
        
        currentFunc         = new String("HELP");
        cmdPanel            = null;
        executeCmd          = null;
        clrInputCmd         = null;
        clrOutputCmd        = null;
        fileMenu            = new JFileChooser();
        cInFile             = null;
        cOutFile            = null;
        inFile              = null;
        outFile             = null;
        
        statLbl             = null;
        status              = null;
        numProc             = 0;
        
        opt                 = new JOptionPane();
        
        timeout             = 90000;
        
        twidth              = 0;
        theight             = 0;
        
        CCBCLguiSysCall.initSysCall(location, outText);
    }
    
    /*===========================================================================
    ** initFrame    initializes the JFrame
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    public void initFrame()
    {
        buildMenu();
        buildInput(currentFunc);
        buildButtons();
        mainPanel.add(outPane, BorderLayout.CENTER);
    }
    
    /*===========================================================================
    ** initFrame    initializes the Setup
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    public void initSetup()
    {
        /*
        ** temps within initSetup
        */
        String tmpStr   = null;
        Color clr       = null;
        Font fnt        = null;
        int tmpint      = 0;
        int tmpint2     = 0;
        boolean tmpbool = true;        
        
        
        /*
        **  get our mode
        */
        tmpStr = CCBCLguiSetup.getValue(guiModRec);
        if (tmpStr == null)
        {
            tmpStr = new String("NORMAL");
            CCBCLguiSetup.setNewValue(guiModRec, tmpStr);
        }
        
        if (tmpStr.equals("COMMAND"))
        {
            processMenuOP("Command Line Mode On");    
        }
        
        else if (tmpStr.equals("SCRIPT"))
        {
            processMenuOP("Script Mode On");
        }
        
        
        /*
        **  setup the ip menu
        */
        tmpStr = CCBCLguiSetup.getValue(ipCntRec);
        if (tmpStr == null)
        {
            /* default 0 */
            tmpint = 0;
            CCBCLguiSetup.setNewValue(ipCntRec, String.valueOf(tmpint));
        }
        else
        {
            try{
                tmpint = Integer.parseInt(tmpStr);
            }catch (Exception e)
            {
                System.out.println("Error parsing ip count for String" + tmpStr + ":\n" + e.getMessage());
            }
        }
        
        if (tmpint > 0)
        {
            for (int i = 1; i <= tmpint; ++i)
            {
                tmpStr = ipRec + String.valueOf(i);
                tmpStr = CCBCLguiSetup.getValue(tmpStr);
                if (tmpStr != null)
                {
                    processMenuIP("Add IP", tmpStr);
                }
                else
                {
                    System.out.println("ip count does not match number of ip's");
                }
            }
        }
        
        /*
        **  setup the help textarea
        */
        helpText.setEditable(false);
        
        tmpStr = CCBCLguiSetup.getValue(hlpBckRec);
        if (tmpStr == null)
        {
            /* default White */
            CCBCLguiSetup.setNewValue(hlpBckRec, CCBCLguiSetup.packColor(Color.white));
            clr = Color.white;
        }
        else
        {
            clr = CCBCLguiSetup.unPackColor(tmpStr);
        }
        
        helpText.setBackground(clr);
        
        tmpStr = CCBCLguiSetup.getValue(hlpForRec);
        if (tmpStr == null)
        {
            /* default magenta */
            CCBCLguiSetup.setNewValue(hlpForRec, CCBCLguiSetup.packColor(Color.magenta));
            clr = Color.magenta;
        }
        else
        {
            clr = CCBCLguiSetup.unPackColor(tmpStr);
        }
        
        helpText.setForeground(clr);
        
        tmpStr = CCBCLguiSetup.getValue(hlpFntRec);
        if (tmpStr == null)
        {
            /* default font */
            fnt = new Font("Courier", Font.BOLD, 12);
            CCBCLguiSetup.setNewValue(hlpFntRec, CCBCLguiSetup.packFont(fnt));
        }
        else
        {
            fnt = CCBCLguiSetup.unPackFont(tmpStr);
        }
        
        helpText.setFont(fnt);
        
        
        /*
        **  setup the output textarea
        */
        outText.setEditable(false);
        
        tmpStr = CCBCLguiSetup.getValue(outBckRec);
        if (tmpStr == null)
        {
            /* default black */
            CCBCLguiSetup.setNewValue(outBckRec, CCBCLguiSetup.packColor(Color.black));
            clr = Color.black;
        }
        else
        {
            clr = CCBCLguiSetup.unPackColor(tmpStr);
        }
        
        outText.setBackground(clr);
        
        tmpStr = CCBCLguiSetup.getValue(outForRec);
        if (tmpStr == null)
        {
            /* default black */
            CCBCLguiSetup.setNewValue(outForRec, CCBCLguiSetup.packColor(Color.green));
            clr = Color.green;
        }
        else
        {
            clr = CCBCLguiSetup.unPackColor(tmpStr);
        }
        
        outText.setForeground(clr);
        
        tmpStr = CCBCLguiSetup.getValue(outFntRec);
        if (tmpStr == null)
        {
            /* default font */
            fnt = new Font("Courier", Font.BOLD, 12);
            CCBCLguiSetup.setNewValue(outFntRec, CCBCLguiSetup.packFont(fnt));
        }
        else
        {
            fnt = CCBCLguiSetup.unPackFont(tmpStr);
        }
        
        outText.setFont(fnt);
        
        
        /*
        **  options actionlistener
        */
        opts.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae2)
            {
                processBtnFunc("Execute");
            }
        });
        
        /*
        **  current command keylistener
        */
        currentCmd.addKeyListener(new KeyAdapter()
        {
            public void keyPressed(KeyEvent ke)
            {
                switch (ke.getKeyCode())
                {
                    case KeyEvent.VK_UP:
                        if (cmdListPtr >= 1)
                        {
                            currentCmd.setText((String)cmdList.elementAt(--cmdListPtr));
                        }
                        break;
                        
                    case KeyEvent.VK_DOWN:
                        if (cmdListPtr <  (cmdList.size()-1))
                        {
                            currentCmd.setText((String)cmdList.elementAt(++cmdListPtr));
                        }
                        else
                        {
                            if (cmdListPtr == (cmdList.size()-1))
                            {
                                ++cmdListPtr;
                            }    
                            currentCmd.setText("");
                        }    
                        break;
                    
                    case KeyEvent.VK_ENTER:
                        processBtnFunc("Execute");
                        break;
                    
                    default:
                        break;
                }
                        
            }
        });
        
        
        /*
        **  find out if we are to save settings or not
        */
        tmpStr = CCBCLguiSetup.getValue(savSetRec);
        if (tmpStr == null)
        {
            tmpStr = new String("TRUE");
            CCBCLguiSetup.setNewValue(savSetRec, tmpStr);
        }
        
        if (tmpStr.equals("TRUE"))
        {
            saveSettings = true;
            
            tmpStr = CCBCLguiSetup.getValue(thsSizRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf(this.getWidth()) + "," +
                                    String.valueOf(this.getHeight()));
                
                CCBCLguiSetup.setNewValue(thsSizRec, tmpStr);
            }
            this.setSize((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                         (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
                         
            
            
            tmpStr = CCBCLguiSetup.getValue(thsLocRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf((int)this.getLocation().getX()) + "," +
                                    String.valueOf((int)this.getLocation().getY()));
                
                CCBCLguiSetup.setNewValue(thsLocRec, tmpStr);
            }
            this.setLocation((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                             (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
            
                
            
            tmpStr = CCBCLguiSetup.getValue(asySizRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf(async.getWidth()) + "," +
                                    String.valueOf(async.getHeight()));
                
                CCBCLguiSetup.setNewValue(asySizRec, tmpStr);
            }
            async.setSize((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                          (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
                         
            
            
            tmpStr = CCBCLguiSetup.getValue(asyLocRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf((int)async.getLocation().getX()) + "," +
                                    String.valueOf((int)async.getLocation().getY()));
                
                CCBCLguiSetup.setNewValue(asyLocRec, tmpStr);
            }
            async.setLocation((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                              (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
            
            
            
            tmpStr = CCBCLguiSetup.getValue(asyVisRec);            
            if (tmpStr == null)
            {
                tmpStr = new String("FALSE");
                
                CCBCLguiSetup.setNewValue(asyVisRec, tmpStr);
            }
            
            if (tmpStr.equals("TRUE"))
            {
                async.show();
            }
                
        
            tmpStr = CCBCLguiSetup.getValue(dbgSizRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf(debug.getWidth()) + "," +
                                    String.valueOf(debug.getHeight()));
                
                CCBCLguiSetup.setNewValue(dbgSizRec, tmpStr);
            }
            debug.setSize((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                          (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
                         
            
            
            tmpStr = CCBCLguiSetup.getValue(dbgLocRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf((int)debug.getLocation().getX()) + "," +
                                    String.valueOf((int)debug.getLocation().getY()));
                
                CCBCLguiSetup.setNewValue(dbgLocRec, tmpStr);
            }
            debug.setLocation((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                              (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
            
            
            
            tmpStr = CCBCLguiSetup.getValue(dbgVisRec);            
            if (tmpStr == null)
            {
                tmpStr = new String("FALSE");
                
                CCBCLguiSetup.setNewValue(dbgVisRec, tmpStr);
            }
            
            if (tmpStr.equals("TRUE"))
            {
                debug.show();
            }
            
            
            tmpStr = CCBCLguiSetup.getValue(ccbSizRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf(ccblg.getWidth()) + "," +
                                    String.valueOf(ccblg.getHeight()));
                
                CCBCLguiSetup.setNewValue(ccbSizRec, tmpStr);
            }
            ccblg.setSize((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                          (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
                         
            
            
            tmpStr = CCBCLguiSetup.getValue(ccbLocRec);            
            if (tmpStr == null)
            {
                tmpStr = new String(String.valueOf((int)ccblg.getLocation().getX()) + "," +
                                    String.valueOf((int)ccblg.getLocation().getY()));
                
                CCBCLguiSetup.setNewValue(ccbLocRec, tmpStr);
            }
            ccblg.setLocation((int)CCBCLguiSetup.unPackPoint(tmpStr).getX(),
                              (int)CCBCLguiSetup.unPackPoint(tmpStr).getY());
            
            
            
            tmpStr = CCBCLguiSetup.getValue(ccbVisRec);            
            if (tmpStr == null)
            {
                tmpStr = new String("FALSE");
                
                CCBCLguiSetup.setNewValue(ccbVisRec, tmpStr);
            }
            
            if (tmpStr.equals("TRUE"))
            {
                ccblg.show();
            }
            
            
            buildInput("HELP");
        }
        
        else
        {
            saveSettings = false;
        }
        
        
        savSetMen = new JCheckBoxMenuItem("Save Window Settings",  saveSettings);
        
        savSetMen.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe15)
            {
                processMenuOP(aoe15.getActionCommand());
            }
        });
        opMen.addSeparator();
        opMen.add(savSetMen);
        
        this.addComponentListener(new ComponentListener()
        {
            public void componentHidden(ComponentEvent e)
            {
            
            }
                
            public void componentShown(ComponentEvent e)
            {
              
            }
                
            public void componentResized(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveThisSet();      
                }
           
                buildInput(currentFunc);   
            }
                
            public void componentMoved(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveThisSet();   
                }
            }
        });
            
        async.addComponentListener(new ComponentListener()
        {
            public void componentHidden(ComponentEvent e)
            {
                if (saveSettings)
                {
                    CCBCLguiSetup.updateValue(asyVisRec, "FALSE");
                } 
            }
                
            public void componentShown(ComponentEvent e)
            {
                if (saveSettings)
                {
                    CCBCLguiSetup.updateValue(asyVisRec, "TRUE");
                }   
            }
                
            public void componentResized(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveAsyncSet();   
                }
            }
                
            public void componentMoved(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveAsyncSet();   
                }
            }
        });
            
        debug.addComponentListener(new ComponentListener()
        {
            public void componentHidden(ComponentEvent e)
            {
                if (saveSettings)
                {
                    CCBCLguiSetup.updateValue(dbgVisRec, "FALSE");
                }   
            }
                
            public void componentShown(ComponentEvent e)
            {
                if (saveSettings)
                {
                    CCBCLguiSetup.updateValue(dbgVisRec, "TRUE");
                }    
            }
               
            public void componentResized(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveDebugSet();      
                }   
            }
                
            public void componentMoved(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveDebugSet();   
                }
            }
        });
        
        ccblg.addComponentListener(new ComponentListener()
        {
            public void componentHidden(ComponentEvent e)
            {
                if (saveSettings)
                {
                    CCBCLguiSetup.updateValue(ccbVisRec, "FALSE");
                }   
            }
                
            public void componentShown(ComponentEvent e)
            {
                if (saveSettings)
                {
                    CCBCLguiSetup.updateValue(ccbVisRec, "TRUE");
                }    
            }
               
            public void componentResized(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveCcblgSet();      
                }   
            }
                
            public void componentMoved(ComponentEvent e)
            {
                if (saveSettings)
                {
                    saveCcblgSet();   
                }
            }
        });
    }
    
    /*===========================================================================
    ** saveThisSet
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    private void saveThisSet()
    {
        CCBCLguiSetup.updateValue(thsSizRec,
                        new String(String.valueOf(getWidth()) + "," +
                                   String.valueOf(getHeight())));
                    
        CCBCLguiSetup.updateValue(thsLocRec, 
                        new String(String.valueOf((int)getLocation().getX()) + "," +
                                   String.valueOf((int)getLocation().getY())));   
    }
    
    /*===========================================================================
    ** saveAsyncSet
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    private void saveAsyncSet()
    {
        CCBCLguiSetup.updateValue(asySizRec,
                        new String(String.valueOf(async.getWidth()) + "," +
                                   String.valueOf(async.getHeight())));
                                       
        CCBCLguiSetup.updateValue(asyLocRec, 
                        new String(String.valueOf((int)async.getLocation().getX()) + "," +
                                   String.valueOf((int)async.getLocation().getY())));
                                   
        if (async.isVisible())
        {
            CCBCLguiSetup.updateValue(asyVisRec, "TRUE");
        }
        else
        {
            CCBCLguiSetup.updateValue(asyVisRec, "FALSE");
        }    
    }
    
    /*===========================================================================
    ** saveDebugSet
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    private void saveCcblgSet()
    {
        CCBCLguiSetup.updateValue(ccbSizRec,
                        new String(String.valueOf(ccblg.getWidth()) + "," +
                                   String.valueOf(ccblg.getHeight())));
                                     
        CCBCLguiSetup.updateValue(ccbLocRec, 
                        new String(String.valueOf((int)ccblg.getLocation().getX()) + "," +
                                   String.valueOf((int)ccblg.getLocation().getY())));
                                   
        if (ccblg.isVisible())
        {
            CCBCLguiSetup.updateValue(ccbVisRec, "TRUE");
        }
        else
        {
            CCBCLguiSetup.updateValue(ccbVisRec, "FALSE");
        }    
    }
    
    /*===========================================================================
    ** saveCcblgSet
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    private void saveDebugSet()
    {
        CCBCLguiSetup.updateValue(dbgSizRec,
                        new String(String.valueOf(debug.getWidth()) + "," +
                                   String.valueOf(debug.getHeight())));
                                     
        CCBCLguiSetup.updateValue(dbgLocRec, 
                        new String(String.valueOf((int)debug.getLocation().getX()) + "," +
                                   String.valueOf((int)debug.getLocation().getY())));
                                   
        if (debug.isVisible())
        {
            CCBCLguiSetup.updateValue(dbgVisRec, "TRUE");
        }
        else
        {
            CCBCLguiSetup.updateValue(dbgVisRec, "FALSE");
        }    
    }
    
    /*===========================================================================
    ** buildMenu        builds the menus and menu bar
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    private void buildMenu()
    {
        /*
        ** create the main menus
        */
        fileMen = new JMenu("File");
        ipMen = new JMenu("Connect");
        opMen = new JMenu("Options");
        JMenuItem temp;
        
        /*
        ** these variables are for the construction of the CCBCLguiFunction menus
        */
        mainMenu = new JMenuBar();
        commands = new JMenu[26];
        JMenuItem item;
        String tempStr;
        char[] arr1;
        char[] arr2;
        int i = 0;
        int j = 0;
        
        
        /*
        **==================FILE MENU=======================
        */
        /* Load Script */
        temp = new JMenuItem("Load Script");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe1)
            {
                processMenuFile(afe1.getActionCommand());
            }
        });
        fileMen.add(temp);
        
        /* Load Output */
        temp = new JMenuItem("Load Output");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe2)
            {
                processMenuFile(afe2.getActionCommand());
            }
        });
        fileMen.add(temp);
        
        /* Save Script */
        temp = new JMenuItem("Save Script");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe3)
            {
                processMenuFile(afe3.getActionCommand());
            }
        });
        fileMen.add(temp);
        
        /* Save Script As */
        temp = new JMenuItem("Save Script As");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe4)
            {
                processMenuFile(afe4.getActionCommand());
            }
        });
        fileMen.add(temp);
        
        /* Save Output */
        temp = new JMenuItem("Save Output");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe5)
            {
                processMenuFile(afe5.getActionCommand());
            }
        });
        fileMen.add(temp);
        
        /* Save Output As */
        temp = new JMenuItem("Save Output As");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe6)
            {
                processMenuFile(afe6.getActionCommand());
            }
        });
        fileMen.add(temp);
        fileMen.addSeparator();
        
        /* Exit */
        temp = new JMenuItem("Exit");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent afe8)
            {
                processMenuFile(afe8.getActionCommand());
            }
        });
        fileMen.add(temp);
        
        /*
        **====================IP MENU=======================
        */
        /* IP */
        temp = new JMenuItem("Add IP");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aie1)
            {
                processMenuIP(aie1.getActionCommand(), null);
            }
        });
        ipMen.add(temp);
        
        temp = new JMenuItem("Remove IP");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aie2)
            {
                processMenuIP(aie2.getActionCommand(), null);
            }
        });
        ipMen.add(temp);
        ipMen.addSeparator();
        
        /*
        **====================OPTIONS MENU=======================
        */
        /* Script Mode */
        temp = new JMenuItem("Script Mode On");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe1)
            {
                processMenuOP(aoe1.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* Command Line Mode */
        temp = new JMenuItem("Command Line Mode On");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe2)
            {
                processMenuOP(aoe2.getActionCommand());
            }
        });
        opMen.add(temp);
        opMen.addSeparator();
        
        /* Time Out */
        temp = new JMenuItem("Time Out");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe4)
            {
                processMenuOP(aoe4.getActionCommand());
            }
        });
        opMen.add(temp);
        opMen.addSeparator();
        
        /* new Location of ccbcl.pl */
        temp = new JMenuItem("Pick New Exerciser");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe5)
            {
                processMenuOP(aoe5.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* new Location of ccbcl.pl */
        temp = new JMenuItem("Refresh Process");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe6)
            {
                processMenuOP(aoe6.getActionCommand());
            }
        });
        opMen.add(temp);
        opMen.addSeparator();
        
        /* set the output Background */
        temp = new JMenuItem("Set Output Background");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe6)
            {
                processMenuOP(aoe6.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* set the output ForeGround */
        temp = new JMenuItem("Set Output Foreground");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe7)
            {
                processMenuOP(aoe7.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* set the output font */
        temp = new JMenuItem("Set Output Font");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe8)
            {
                processMenuOP(aoe8.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* set the help Background */
        temp = new JMenuItem("Set Help Background");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe9)
            {
                processMenuOP(aoe9.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* set the help ForeGround */
        temp = new JMenuItem("Set Help Foreground");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe10)
            {
                processMenuOP(aoe10.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* set the help font */
        temp = new JMenuItem("Set Help Font");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe11)
            {
                processMenuOP(aoe11.getActionCommand());
            }
        });
        opMen.add(temp);
        opMen.addSeparator();
        
        /* Show CCB LOG */
        temp = new JMenuItem("Show CCBE Log");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe7)
            {
                processMenuOP(aoe7.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* shows async server */
        temp = new JMenuItem("Show Async Server");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe12)
            {
                processMenuOP(aoe12.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /* shows debug server */
        temp = new JMenuItem("Show Debug Console");
        temp.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent aoe13)
            {
                processMenuOP(aoe13.getActionCommand());
            }
        });
        opMen.add(temp);
        
        /*
        ** add menus to menu bar
        */
        mainMenu.add(fileMen);
        mainMenu.add(ipMen);
        mainMenu.add(opMen);
        
                
        /*
        ** start the creation of the CCBCLguiFunction menus
        */
        tempStr = funcs[0].getFuncName();
        arr1 = tempStr.toCharArray();
        tempStr = new String("");
        tempStr += arr1[0];
        commands[j++] = new JMenu(tempStr);
        
        /*
        ** run through funcs, the CCBCLguiFunction array
        ** and add the functions to the menus
        ** we want to make an alphabetical list on the
        ** menu bar, by the order of the first letter
        ** of the function
        */
        for (i = 1; i < funcs.length; ++i)
        {
            item = new JMenuItem(new String(arr1));
            commands[j-1].add(item);
            
            item.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent ae1)
                {
                    processMenuFunc(ae1.getActionCommand());
                }
            });
            
            /* get the next function from the array */
            arr2 = funcs[i].getFuncName().toCharArray();
            
            /* compare the first letters of the functions
            ** if they are different, create a new menu
            ** using the first letter of the new function
            */
            if (arr1[0] != arr2[0])
            {
                tempStr = new String("");
                tempStr += arr2[0];
                commands[j++] = new JMenu(tempStr);
            }  
            
            arr1 = arr2;
        }
        
        /*
        ** we need to get the last function, and add it
        */
        item = new JMenuItem(new String(arr1));
        commands[j-1].add(item);
        
        item.addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae2)
            {
                processMenuFunc(ae2.getActionCommand());
            }
        });
        
        
        /*
        ** run through the function menus, and add them to the menu bar
        */
        for (i = 0; i < j; ++i)
        {
            mainMenu.add(commands[i]);
        }
        
        this.setJMenuBar(mainMenu);
    }
        
    /*===========================================================================
    ** buildInput       builds the input side of the JFrame
    **
    ** Inputs:          func:   - Function to open or null
    **
    ** Returns: none 
    **===========================================================================*/
    private void buildInput(String func)
    {
        int sWidth = (this.getWidth());
        int sHeight = (this.getHeight());    
        boolean quit = false;
        boolean found = false;
        int i = 0;
        String[] parms;
        
        twidth = sWidth;
        theight = theight; 
        
        /*
        ** clear the options text field
        */
        opts.setText("");
        
        /*
        ** if the input panel is null we want to clear it
        */
        if(inputPanel != null)
        {
            inputPanel.setVisible(false);
            inputPanel.removeAll();
            mainPanel.remove(inputPanel);
        }
        
        /*
        ** if the parmPanel is null we want to clear it
        */
        if(parmPanel != null)
        {
            parmPanel.removeAll();
        }
        
        /*
        ** the function to open is null set it to HELP
        */
        if (func == null)
        {
            func = new String("HELP");
        }
        
        /*
        ** set the current function so we know what it is
        */
        currentFunc = new String(func);
        
        /*
        ** look for the function in the funcs array
        */
        while (!quit && !found && (func != null))
        {
            if(funcs[i].getFuncName().equals(func))
            {
                found = true;
                quit = true;
            }
            
            if(i + 1 == funcs.length)
            {
                quit = true;
            }
            ++i;
        }
        
        /*
        ** check to see if we are in script mode or not
        ** and follow the correct path
        */
        if (scriptMode)
        {
            
            /*
            ** if the function was found we want to create a
            ** panel and show the scriptText and the functions
            ** help
            */
            if (found)
            {
                inputPanel = new JPanel(new GridLayout(2,1,0,0));
            }
            
            /*
            ** else we just want to show the scriptText
            */
            else
            {
                inputPanel = new JPanel(new GridLayout(1,1,0,0));
            }
            
            /*
            ** set all the sizes for the panel and the scriptText
            */
            inputPanel.setSize((int)(sWidth *.3), (int)(sHeight * .9));
            inputPanel.setMinimumSize(new Dimension(((int)(sWidth *.3)), ((int)(sHeight * .9))));
            inputPanel.setPreferredSize(new Dimension((int)(sWidth *.3), (int)(sHeight * .9)));
            scriptText.setSize((int)(sWidth *.3), (int)(sHeight * .9));
            
            /*
            ** add scriptText to the input panel
            */
            inputPanel.add(new JScrollPane(scriptText));
            
            /*
            ** once again if the function was found, add its help
            ** text to the input panel
            */
            if (found)
            {
                helpText.setText(funcs[i-1].getHelp());
                inputPanel.add(new JScrollPane(helpText));
            }
            
            /*
            ** add the input panel to the ourself and refresh
            */
            mainPanel.add(inputPanel, BorderLayout.WEST);
            mainPanel.repaint();
            mainPanel.setVisible(true);
        }
        
        /*
        ** if we are not in script mode we want to create a panel
        ** with the parameters that we could parse as text fields
        ** and the function help text as well.
        */
        else
        {   
            /*
            ** if the function was found set up the input panel
            */
            if (found)
            {
                /*
                ** set up all of our components
                */
                helpText.setText(funcs[i-1].getHelp());
                parms = funcs[i-1].getParameters();
                
                if (!cmdMode)
                {
                    inputPanel = new JPanel(new GridLayout(2,1,0,0));
                    parmPanel = new JPanel(new GridLayout(12, 2, 0, 0));
                    
                    currentCmd.setText(funcs[i-1].getFuncName());
                    
                    parmLabels = new JLabel[parms.length];
                    parmFields = new JTextField[parms.length];
                
                    /*
                    ** add an options parameter for any options
                    ** or function fields that we missed
                    */
                    parmPanel.add(new JLabel("Options"));
                    parmPanel.add(opts);

                    /*
                    ** get the parameters for the function
                    ** and create JLabels and JTextFields
                    ** for them and add them to the parameter
                    ** panel (parmPanel)
                    */
                    for (i=0; i < parms.length; ++i)
                    {
                        parmLabels[i] = new JLabel(parms[i]);
                        parmFields[i] = new JTextField("");
                        parmPanel.add(parmLabels[i]);
                        parmPanel.add(parmFields[i]);
                        parmFields[i].addActionListener(new ActionListener()
                        {
                            public void actionPerformed(ActionEvent ae2)
                            {
                                processBtnFunc("Execute");
                            }
                        });
                    }

                    /*
                    ** add our components to the input panel
                    */
                    inputPanel.add(parmPanel);
                }
                
                else
                {
                    inputPanel = new JPanel(new GridLayout(1,1,0,0));
                }
                
                inputPanel.setSize((int)(sWidth *.3), (int)(sHeight * .9));
                inputPanel.setMinimumSize(new Dimension(((int)(sWidth *.3)), ((int)(sHeight * .9))));
                inputPanel.setPreferredSize(new Dimension((int)(sWidth *.3), (int)(sHeight * .9)));
                
                inputPanel.add(new JScrollPane(helpText));
                inputPanel.repaint();
                
                /*
                ** add the input panel to the ourself and refresh
                */
                mainPanel.add(inputPanel, BorderLayout.WEST);
                mainPanel.repaint();
                mainPanel.setVisible(true);
                
                /*
                ** divert the focus first to the help text
                ** and then set the focus on the first thing in the 
                ** parameter panel
                */
                helpText.requestFocus();
                
                if (cmdMode)
                {
                    currentCmd.requestFocus();
                }
                else if (parms.length > 0)
                {
                    parmFields[0].requestFocus();
                }
                else
                {
                    opts.requestFocus();
                }   
            }
            
            /*
            ** if the function was not found display an error message
            */
            else
            {
                opt.showMessageDialog(null,"Could not find function: " + func);
            }
        }
    }
    
    /*===========================================================================
    ** buildButtons     this is the panel at the bottom of the screen
    **                  with the buttons and labels
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    private void buildButtons()
    {
        /*
        ** Create the buttons and the labels
        */
        executeCmd = new JButton("Execute");
        executeCmd. addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae1)
            {
                processBtnFunc(ae1.getActionCommand());
            }
        });
        
        clrInputCmd = new JButton("Clear Inputs");
        clrInputCmd. addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae2)
            {
                processBtnFunc(ae2.getActionCommand());
            }
        });
        
        clrOutputCmd = new JButton("Clear Output");
        clrOutputCmd. addActionListener(new ActionListener()
        {
            public void actionPerformed(ActionEvent ae3)
            {
                processBtnFunc(ae3.getActionCommand());
            }
        });
        
        currentIP = new JLabel("Current IP: NONE",SwingConstants.CENTER);
        statLbl             = new JLabel("Status: ", SwingConstants.RIGHT);
        status              = new JLabel("Waiting for Command", SwingConstants.LEFT);
        
        /*
        ** Create the panel
        */
        cmdPanel = new JPanel(new GridLayout(1, 6, 3, 0));
        
        Container tmp = new JPanel(new GridLayout(2, 1, 0, 0));
        tmp.add(currentCmd);
    
        
        /*
        ** add the buttons and the labels to the cmdPanel
        */
        cmdPanel.add(executeCmd);
        cmdPanel.add(clrInputCmd);
        cmdPanel.add(clrOutputCmd);
        cmdPanel.add(currentIP);
        cmdPanel.add(statLbl);
        cmdPanel.add(status);
        tmp.add(cmdPanel);
        
        /*
        ** add the cmdPanel to ourself
        */
        mainPanel.add(tmp, BorderLayout.SOUTH);
    }
    
    /*===========================================================================
    ** processMenuFile  processes a menu selection from the file menu
    **
    ** Inputs:          ae  - Name on menu (used by ActionListener)
    **
    ** Returns: none 
    **===========================================================================*/
    private void processMenuFile(String ae)
    {
        String resp;
        
        /*
        ** if Load Script ask for the script to open
        ** open it in the scriptText
        */
        if(ae.equals("Load Script"))
        {
            resp = openFile(cInFile);  
            
            if (resp!=null)
            {
                scriptText.setText(resp);
                processMenuOP("Script Mode On");
            }   
        }
        
        /*
        ** if Load Output ask for the output to open
        ** open it in the outText
        */
        else if(ae.equals("Load Output"))
        {
            resp = openFile(cOutFile);
            if (resp!=null)
            {
                outText.setText(resp);
            }
        }
        
        /*
        ** if Save Script check to see that cInFile is null
        ** if it is call saveFileAs
        ** else call saveFile
        */
        else if(ae.equals("Save Script"))
        {
            if(cInFile != null)
            {
                saveFile(cInFile, scriptText.getText());
            }
            else
            {
                cInFile = saveFileAs(scriptText.getText());
            }
        }
        
        /*
        ** if Save Script As call saveFileAs
        */
        else if(ae.equals("Save Script As"))
        {
            cInFile = saveFileAs(scriptText.getText());
        }
        
        /*
        ** if Save Output check to see that cOutFile is null
        ** if it is call saveFileAs
        ** else call saveFile
        */
        else if(ae.equals("Save Output"))
        {
            if(cOutFile != null)
            {
                saveFile(cOutFile, outText.getText());
            }
            else
            {
                cOutFile = saveFileAs(outText.getText());
            }
        }
        
        /*
        ** if Save Ouput As call saveFileAs
        */
        else if(ae.equals("Save Output As"))
        {
            cOutFile = saveFileAs(outText.getText());
        }
        
        else if(ae.equals("Exit"))
        {
            finalize();
        }
    }
    
    /*===========================================================================
    ** processMenuIP    processes a menu selection from the IP menu
    **
    ** Inputs:          ae  - Name on menu (used by ActionListener)
    **
    ** Returns: none 
    **===========================================================================*/
    private void processMenuIP(String ae, String ip)
    {
        String ipAdr;
        boolean quit = false;
        int count = 0;
        boolean changeip = false;
        /*
        ** if Add IP ask for the IP address an add it to the menu
        ** and currentIP
        */
        if(ae.equals("Add IP"))
        {
            /*
            ** give 3 chances to give a valid ip
            */
            while (!quit)
            {
                if (ip == null)
                {
                    ipAdr = opt.showInputDialog(null,"Enter an Ip Address");
                }
                else
                {
                    ipAdr = ip;
                }
                
                /*
                ** if ipAdr == null, user must have left it blank or hit
                ** cancel so exit the loop now
                */
                if (ipAdr == null)
                {
                    quit = true;
                }
                
                /*
                ** else call parseIP to see if valid ip address
                ** if it is a valid ip add it to the IP menu and
                ** the currentIP Label
                */
                else if(parseIp(ipAdr))
                {
                    ae = new String(ipAdr);
                    
                    if (!(currentIP.getText().equals("Current IP: " + ae)) && ip == null)
                    {
                        currentIP.setText("Current IP: " + ae);
                        changeip = true;
                    }
                    
                    if (ipList.contains(ipAdr))
                    {
                        opt.showMessageDialog(null,"IP already is in the List");
                    }
                    else
                    {
                        String tmpIP = null;
                        ipList.add(ipAdr);
                        
                        if (ip == null)
                        {
                            tmpIP = ipRec + String.valueOf(ipList.size());
                            CCBCLguiSetup.setNewValue(tmpIP, ipAdr);
                            CCBCLguiSetup.updateValue(ipCntRec, String.valueOf(ipList.size()));
                        }
                        
                        JMenuItem temp = new JMenuItem(ipAdr);
                    
                        temp.addActionListener(new ActionListener()
                        {
                            public void actionPerformed(ActionEvent ae1)
                            {
                                processMenuIP(ae1.getActionCommand(), null);
                            }
                        });
                    
                        /*
                        ** refresh the IP menu
                        */
                        //ipMen.setVisible(false);
                        ipMen.add(temp);
                        ipMen.repaint();
                        ipMen.setVisible(true);
                    }            
                    quit = true;
                }
                
                /*
                ** if we have gone through the loop 3 times
                ** without a valid ip exit the loop
                */
                if (++count == 3)
                {
                    quit = true;
                }
            }
        }
        
        /*
        ** if Remove IP ask for the IP to remove
        */
        else if(ae.equals("Remove IP"))
        {
            String tmpStr = null;
            Vector tmpvec = new Vector(1,1);
            /*
            ** if there is nothing in the list to remove display message
            */
            if (ipList.size() == 0)
            {
                opt.showMessageDialog(null,"There are no IP addresses to remove");
            }
            
            /*
            ** else get the ip
            */
            else
            {
                ipAdr = opt.showInputDialog(null,"Enter an Ip Address");
                
                /*
                ** if ipAdr == null, user must have left it blank or hit
                ** cancel so exit now
                */
                if (ipAdr != null)
                {
                    /*
                    ** else call parseIP to see if valid ip address
                    ** if it is a valid ip add it to the IP menu and
                    ** the currentIP Label
                    */
                    if(parseIp(ipAdr))
                    {
                        /*
                        ** if we don't already have the ip there
                        ** is no need to remove it
                        */
                        if (!(ipList.contains(ipAdr)))
                        {
                            opt.showMessageDialog(null,"IP is not in the List");
                        }
                        
                        else
                        {
                            count = ipList.indexOf(ipAdr);
                            ipList.remove(ipAdr);
                            if (currentIP.getText().equals("Current IP: " + ipAdr))
                            {
                                currentIP.setText("Current IP: NONE");
                            }
                            
                            for (int i = 1; i <= (ipList.size() + 1); ++i)
                            {
                                tmpStr = ipRec + String.valueOf(i);
                                tmpStr = CCBCLguiSetup.getValue(tmpStr);
                                if (tmpStr != null)
                                {
                                    tmpvec.add(tmpStr);
                                    CCBCLguiSetup.removeValue(ipRec + String.valueOf(i));
                                }
                                else
                                {
                                    System.out.println("ip count does not match number of ip's");
                                }
                            }
                            
                            tmpvec.remove(ipAdr);
                            for (int i = 1; i <= ipList.size(); ++i)
                            {
                                tmpStr = ipRec + String.valueOf(i);
                                CCBCLguiSetup.setNewValue(tmpStr, (String)tmpvec.remove(0));
                            }
                            
                            CCBCLguiSetup.updateValue(ipCntRec, String.valueOf(ipList.size()));
                            
                            ipMen.remove(count+3);
                            ipMen.repaint();
                            ipMen.setVisible(true);    
                        }
                    }
                    
                    /*
                    ** else ivvalid ip address
                    */
                    else
                    {
                        opt.showMessageDialog(null,"Invalid IP address");
                    }
                }
            }
        }
        
        /*
        ** else they have selected an ip address that they have
        ** already added to the list, so set it as the currentIP
        ** on currentIP label
        */
        else
        {
            if (!(currentIP.getText().equals("Current IP: " + ae)))
            {
                currentIP.setText("Current IP: " + ae);
                changeip = true;
            }
        }
        
        if (changeip && (ip == null))
        {
            String cmdStr = new String("connect " + ae);
            
            try{
                CCBCLguiSysCall.SysCall(cmdStr + "\n", timeout);
            }catch (Exception e)
            {
                outText.append(e.getMessage());
            }
        }   
    }
    
    /*===========================================================================
    ** processMenuOP    processes a menu selection from the Options menu
    **
    ** Inputs:          ae  - Name on menu (used by ActionListener)
    **
    ** Returns: none 
    **===========================================================================*/
    private void processMenuOP(String ae)
    {
        JMenuItem temp = null;
        
        /*
        ** if Script Mode On set scriptMode to true
        ** build the input menu again so we can see it in
        ** scripting mode
        ** remove Script Mode On from the menu and replace
        ** it with Script Mode Off
        */
        if(ae.equals("Script Mode On"))
        {
            scriptMode = true;
            buildInput(currentFunc);
            opMen.remove(0);
            temp = new JMenuItem("Script Mode Off");
            opMen.insert(temp, 0);
            processMenuOP("Command Line Mode Off");
            CCBCLguiSetup.updateValue(guiModRec, "SCRIPT");
        }
        
        /*
        ** if Script Mode Off set scriptMode to false
        ** build the input menu again so we can see it in
        ** regular mode
        ** remove Script Mode Off from the menu and replace
        ** it with Script Mode On
        */
        else if(ae.equals("Script Mode Off"))
        {
            scriptMode = false;
            buildInput(currentFunc);
            opMen.remove(0);
            temp = new JMenuItem("Script Mode On");
            opMen.insert(temp, 0);
            CCBCLguiSetup.updateValue(guiModRec, "NORMAL");
        }
        
        /*
        ** if Command Line Mode On set cmdMode to true
        ** build the input menu again so we can see it in
        ** command line mode
        ** remove Command Line Mode On from the menu and replace
        ** it with Command Line Mode Off
        */
        else if(ae.equals("Command Line Mode On"))
        {
            cmdMode = true;
            
            currentCmd.setBackground(Color.white);
            currentCmd.setForeground(Color.black);
            currentCmd.setText("");
            currentCmd.setEditable(true);
        
            buildInput(currentFunc);
            opMen.remove(1);
            temp = new JMenuItem("Command Line Mode Off");
            opMen.insert(temp, 1);
            processMenuOP("Script Mode Off");
            CCBCLguiSetup.updateValue(guiModRec, "COMMAND");
        }
        
        /*
        ** if Command Line Mode On set cmdMode to false
        ** build the input menu again so we can see it in
        ** normal mode
        ** remove Command Line Mode Off from the menu and replace
        ** it with Command Line Mode On
        */
        else if(ae.equals("Command Line Mode Off"))
        {
            cmdMode = false;
            
            currentCmd.setBackground(Color.black);
            currentCmd.setForeground(Color.green);
            currentCmd.setEditable(false);  
            
            buildInput(currentFunc);
            opMen.remove(1);
            temp = new JMenuItem("Command Line Mode On");
            opMen.insert(temp, 1);
            CCBCLguiSetup.updateValue(guiModRec, "NORMAL");
        }
        
        /*
        ** if Time Out ask for a new timeout in seconds
        ** convert time in seconds to long and save in timeout
        */
        else if(ae.equals("Time Out"))
        {
            long tmot = 0;
            String tmp;
            
            tmp = opt.showInputDialog(null, "Enter new Time Out in seconds. Current (" +
                                        (timeout/1000) + ") seconds");
            if(tmp != null)
            {
                try{
                    tmot = Long.parseLong(tmp);
                    timeout = (tmot * 1000);
                }catch(Exception e)
                {
                    opt.showMessageDialog(null, "Unable to set new timeout");
                }
            }
        }
        
        /*
        ** pick a new location for the ccbcl
        */
        else if(ae.equals("Pick New Exerciser"))
        {
            String tempStr = null;
            CCBCLguiFunction[] tempfuncs = null;
            /*
            ** if the write is successful check to make
            ** sure it is a valid location and that the
            ** file exists
            */
            if (CCBCLguiSetup.writeFile()) 
            {
                tempStr = CCBCLguiSetup.checkSetup();
                if (tempStr != null)
                {
                    location = new String(tempStr);
                    
                    try{
                        tempfuncs = CCBCLguiCreateFunc.CreateFunc(location);
                    }
                    catch (Exception fe)
                    {
                        opt.showMessageDialog(null,fe.getMessage());
                    }    
                    
                    if (tempfuncs != null)
                    {
                        funcs = tempfuncs;
                        Arrays.sort(funcs);
                    
                        mainMenu.setVisible(false);
                        mainMenu.removeAll();
                        buildMenu();
                        mainMenu.repaint();
                        mainMenu.setVisible(true);
                        buildInput("HELP");
                        CCBCLguiSysCall.restartProcess(location);
                    }
                    else
                    {
                        opt.showMessageDialog(null,"Error Parsing ccbcl.pl");
                    }   
                }
                else
                {
                    opt.showMessageDialog(null,"Could not locate ccbcl.pl");
                }
            }
            
            else
            {
                opt.showMessageDialog(null,"Could not locate ccbcl.pl");
            }    
        }
        
        /*
        **  refresh the process
        */
        else if(ae.equals("Refresh Process"))
        {
            CCBCLguiSysCall.restartProcess(location);
        }
        
        /*
        **  refresh the process
        */
        else if(ae.equals("Save Window Settings"))
        {
            saveSettings = savSetMen.getState();
            
            if (saveSettings)
            {
                CCBCLguiSetup.updateValue(savSetRec, "TRUE");
                saveThisSet();
                saveAsyncSet();
                saveDebugSet();
                saveCcblgSet();
            }
            else
            {
                CCBCLguiSetup.updateValue(savSetRec, "FALSE");
            }
        }
        
        /*
        ** if Set Output Background set a new Color based
        ** on user input
        */
        else if(ae.equals("Set Output Background"))
        {
            JColorChooser clr = new JColorChooser();
            Color nwclr = null;
            
            nwclr = clr.showDialog(this,  "Choose New Output Background Color", outText.getBackground());
            
            if (nwclr != null)
            {
                outText.setBackground(nwclr);
                CCBCLguiSetup.updateValue(outBckRec, CCBCLguiSetup.packColor(nwclr));
                outText.repaint();
                this.repaint();
            }
        }
        
        /*
        ** if Set Output Foreground set a new Color based
        ** on user input
        */
        else if(ae.equals("Set Output Foreground"))
        {
            JColorChooser clr = new JColorChooser();
            Color nwclr = null;
            
            nwclr = clr.showDialog(this,  "Choose New Output Foreground Color", outText.getForeground());
            
            if (nwclr != null)
            {
                outText.setForeground(nwclr);
                CCBCLguiSetup.updateValue(outForRec, CCBCLguiSetup.packColor(nwclr));
                outText.repaint();
                this.repaint();
            }
        }
        
        /*
        ** if Set Output Font set a new font based
        ** on user input
        */
        else if(ae.equals("Set Output Font"))
        {
            Font font = null;
            font = CCBCLguiFontChooser.showDialog(this,null,outText.getFont());
            
            if (font != null)
            {
                CCBCLguiSetup.updateValue(outFntRec, CCBCLguiSetup.packFont(font));
                outText.setFont(font);
            }
        }
        
        /*
        ** if Set Output Background set a new Color based
        ** on user input
        */
        else if(ae.equals("Set Help Background"))
        {
            JColorChooser clr = new JColorChooser();
            Color nwclr = null;
            
            nwclr = clr.showDialog(this,  "Choose New Help Background Color", helpText.getBackground());
            
            if (nwclr != null)
            {
                helpText.setBackground(nwclr);
                CCBCLguiSetup.updateValue(hlpBckRec, CCBCLguiSetup.packColor(nwclr));
                helpText.repaint();
                this.repaint();
            }
        }
        
        /*
        ** if Set Output Foreground set a new Color based
        ** on user input
        */
        else if(ae.equals("Set Help Foreground"))
        {
            JColorChooser clr = new JColorChooser();
            Color nwclr = null;
            
            nwclr = clr.showDialog(this,  "Choose New Help Foreground Color", helpText.getForeground());
            
            if (nwclr != null)
            {
                helpText.setForeground(nwclr);
                CCBCLguiSetup.updateValue(hlpForRec, CCBCLguiSetup.packColor(nwclr));
                helpText.repaint();
                this.repaint();
            }
        }
        
        /*
        ** if Set Output Font set a new font based
        ** on user input
        */
        else if(ae.equals("Set Help Font"))
        {
            Font font = null;
            font = CCBCLguiFontChooser.showDialog(this,null,helpText.getFont());
            
            if (font != null)
            {
                CCBCLguiSetup.updateValue(hlpFntRec, CCBCLguiSetup.packFont(font));
                helpText.setFont(font);
            }
        }
        
        /*
        ** if Show CCB Log Server show the async server
        */
        else if(ae.equals("Show CCBE Log"))
        {
            ccblg.show();
        }
        
        /*
        ** if Show Async Server show the async server
        */
        else if(ae.equals("Show Async Server"))
        {
            async.show();
        }
        
        /*
        ** if Show Async Server show the async server
        */
        else if(ae.equals("Show Debug Console"))
        {
            debug.show();
        }
        
        /*
        ** if we created a new JMenuItem
        ** add a new action Listener to the new menuItem
        */
        if (temp != null)
        {
            temp.addActionListener(new ActionListener()
            {
                public void actionPerformed(ActionEvent ae1)
                {
                    processMenuOP(ae1.getActionCommand());
                }
            });
        }
        
        /*
        ** refresh the options menu
        */
        opMen.repaint();
        opMen.setVisible(true);
    }
    
    /*===========================================================================
    ** processMenuFunc  processes a menu selection from the Command Menus
    **
    ** Inputs:          ae  - Name on menu (used by ActionListener)
    **
    ** Returns: none 
    **===========================================================================*/
    private void processMenuFunc(String ae)
    {
        /*
        ** We built these menus from our array of CCBCLguiFunctions
        ** so we will just pass it through to the buildInput function
        ** and let it handle the appropriate course of action
        */
        buildInput(ae);
    }

    /*===========================================================================
    ** processBtnFunc   processes a button selection from the cmdPanel
    **
    ** Inputs:          ae  - Name on button (used by ActionListener)
    **
    ** Returns: none 
    **===========================================================================*/
    private void processBtnFunc(String ae)
    {
        /*
        ** if Execute we want to execute the current command or script
        */
        if(ae.equals("Execute"))
        {
            String cmdStr = new String("");
            String ipAdr;
            int index;
            int i;
            
            /*
            ** check to make sure we have an IP address
            ** if not add one
            */
            if(currentIP.getText().equals("Current IP: NONE") && !cmdMode)
            {
                processMenuIP("Add IP", null);
            }
            
            /*
            ** check to make sure we don't have any outstanding threads
            ** still working on a command
            */
            if (numProc > 0)
            {
                opt.showMessageDialog(null, "There is already a command executing!");
            }
            
            /*
            ** if we have a good IP address and there are no processes already
            ** running, proceed with th command
            */
            else if(!(currentIP.getText().equals("Current IP: NONE")) || cmdMode)
            {  
                boolean good = true;
                ipAdr = new String(currentIP.getText());
                index = ipAdr.lastIndexOf(' ');
                ipAdr  = ipAdr.substring(index+1);
                
                /*
                ** if we are in script mode, all we need to do is to get the 
                ** commands from the scriptText
                */
                if(scriptMode)
                {
                    cmdStr += scriptText.getText();
                }
                
                else if (cmdMode)
                {
                    cmdStr += currentCmd.getText();
                    cmdList.add(currentCmd.getText());
                    cmdListPtr = cmdList.size();
                    currentCmd.setText("");   
                }
                
                /*
                ** else we need to extact the commands from the regular input
                ** screen
                */
                else
                {
                    boolean set = false;
                    
                    /*
                    ** first get the command from the currentCmd
                    ** and any options that may have been added
                    */
                    cmdStr += currentCmd.getText() + " ";
                    cmdStr += opts.getText() + " ";
                    
                    /*
                    ** next we will check the text fields in our
                    ** parmFields array for simple validity
                    ** basically we are checking to see that there
                    ** are no fields with data, that have a blank field
                    ** above them
                    */
                    for (i=(parmFields.length - 1); i >= 0; --i)
                    {
                        if (!(parmFields[i].getText().equals("")))
                        {
                            set = true;
                        }
                        else
                        {
                            if(set == true)
                            {
                                good = false;
                            }
                        }  
                    }
                    
                    /*
                    ** if are parameter checking passed, loop through
                    ** the fields again and retrieve their values
                    */
                    if(good)
                    {
                        for (i=0; i < parmFields.length; ++i)
                        {
                            if (!(parmFields[i].getText().equals("")))
                            {
                                cmdStr += parmFields[i].getText() + " ";
                            }
                        }
                        
                    }
                    
                    /*
                    ** else display an error message
                    */
                    else
                    {
                        opt.showMessageDialog(null,"Missing required Parameter");
                    }
                }
                
                /*
                ** if all is good call CCBCLguiSysCall.writeScript
                ** to write the script, and then call
                ** CCBCLguiSysCall.SysCall to create a thread to
                ** execute the script 
                */
                if(good)
                {
                    try{
                        this.displayOutputText(cmdStr + "\n");
                        CCBCLguiSysCall.SysCall(cmdStr + "\n", timeout);
                    }catch (Exception e)
                    {
                        outText.append(e.getMessage());
                    }
                }
            }
        }
        
        /*
        ** if Clear Output clear the output outText
        ** and set the title to title
        ** also set cOutFile to null to ensure
        ** that the next SaveOutput does not overwrite
        ** a file that may have been opened
        */
        else if(ae.equals("Clear Output"))
        {
            outText.setText("");
            this.setTitle(title);
            cOutFile = null;
        }
        
        /*
        ** if Clear Inputs clear the input scriptText or the
        ** parmFields.  set the title to title
        ** also set cInFile to null to ensure
        ** that the next SaveScript does not overwrite
        ** a file that may have been opened
        */
        else if(ae.equals("Clear Inputs"))
        {
            this.setTitle(title);
            cInFile = null;
            
            /*
            ** if scriptMode clear the scriptText
            */
            if(scriptMode)
            {
                scriptText.setText("");
            }
            
            /*
            ** else loop through the parmFields and clear them
            */
            else
            {
                int i;
                for (i=0; i < parmFields.length; ++i)
                {
                    parmFields[i].setText("");
                }
                
                opts.setText("");
            }
                  
        }
    }
    
    /*===========================================================================
    ** parseIp      parses an IP address to determine its validity
    **
    ** Inputs:      ipAdr   - ip address to parse
    **
    ** Returns:     true    - if good ip address
    **              false   - if bad ip address 
    **===========================================================================*/
    private boolean parseIp(String ipAdr)
    {
        boolean returnVal = true;
        boolean quit = false;
        char[] arr = ipAdr.toCharArray();
        int numCount = 0;
        int dotCount = 0;
        int i = 0;
        
        while (!quit)
        {
            switch (arr[i])
            {
                /*
                ** valid number
                */
                case '0':
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    
                    /*
                    ** if we have had more than 3 numbers
                    ** in a row we have a bad ip
                    */
                    if(++numCount > 3)
                    {
                        returnVal = false;
                        quit = true;
                    }
                    break;
                    
                /*
                ** we have a dot .
                */
                case '.':
                    /*
                    ** if we have no numbers bad ip
                    */
                    if(numCount <= 0)
                    {
                        returnVal = false;
                        quit = true;
                    }
                    
                    /*
                    ** else set numcount to 0
                    */
                    else
                    {
                        numCount = 0;
                        
                        /*
                        ** if we have more than 3 dots
                        ** bad ip
                        */
                        if(++dotCount > 3)
                        {
                            returnVal = false;
                            quit = true;
                        }
                    }
                    break;
                    
                /*
                ** not a valid character bad ip
                */
                default:
                    returnVal = false;
                    quit = true;
                    break;
            }
            if(++i >= arr.length)
                quit = true;
        }
                
        /*
        ** if dot count does not == 3 bad ip
        */
        if(dotCount != 3)
        {
            returnVal = false;
        }
        
        return returnVal;
    }
    
    /*===========================================================================
    ** openFile     opens a JFileChooser to select a file to open
    **
    ** Inputs:      file to Save the new file in
    **
    ** Returns:     returnStr   - The text in the file
    **===========================================================================*/
    private String openFile(File tmp)
    {
        String returnStr = null;
        int size = 0;
        byte[] bffr;
                
        /*
        ** open the JfileChooser for selection of file
        */
        int returnVal = fileMenu.showOpenDialog(this);
        if(returnVal == JFileChooser.APPROVE_OPTION) 
        {
            tmp = new File(new String(fileMenu.getCurrentDirectory() + "\\" + 
                                          fileMenu.getSelectedFile().getName()));
        }
        
        /*
        ** if File tmp is not null, open tmp and read in the text
        ** and store the text String in returnStr
        */
        if(tmp != null)
        {
            try{
                inFile = new FileInputStream(tmp);
                bffr = new byte[inFile.available()];
                inFile.read(bffr);
                returnStr = new String(bffr);
                this.setTitle(title + "--File: " + tmp);
                inFile.close();
            }catch (Exception e)
            {
                opt.showMessageDialog(null,"Unable to open file: " + tmp);
            }
        }
        
        return returnStr;
    }
    
    /*===========================================================================
    ** saveFile     saves a file
    **
    ** Inputs:      tmp - file to save
    **              msg - text to save in File tmp
    **
    ** Returns: none 
    **===========================================================================*/
    private void saveFile(File tmp, String msg)
    {
        /*
        ** if our input values are not null open the file
        ** and save String msg in it
        */
        if((tmp != null) && (msg != null))
        {
            try{
                outFile = new FileOutputStream(tmp);
                outFile.write(msg.getBytes());
                outFile.close();
                this.setTitle(title + "--File: " + tmp);
            }catch (Exception e)
            {
                opt.showMessageDialog(null,"Unable to save file: " + tmp);
            }
        }  
    }
    
    /*===========================================================================
    ** saveFileAs       saves a file
    **
    ** Inputs:          msg - text to save in the new File
    **
    ** Returns:         tmp - new file used for save
    **===========================================================================*/
    private File saveFileAs(String msg)
    {
        File tmp = null;
        int returnVal = fileMenu.showSaveDialog(this);
        
        /*
        ** open the JfileChooser for selection of new file
        ** to save As
        */
        if(returnVal == JFileChooser.APPROVE_OPTION) 
        {
            tmp = new File(new String(fileMenu.getCurrentDirectory() + "\\" + 
                                          fileMenu.getSelectedFile().getName()));
        }
        
        /*
        ** if the new File chosen is not null
        ** call saveFile to save it
        */
        if(tmp != null)
        {
            saveFile(tmp, msg);
        }
        return tmp;  
    }
    
    /*===========================================================================
    ** displayOutputText    displays text in outText
    **                      purpose is for Thread to call when
    **                      finished executing to display results
    **
    ** Inputs:              msg - String to display in outText
    **
    ** Returns: none 
    **===========================================================================*/
    public static void displayOutputText(String msg)
    {
        int caretPos = 0;
        caretPos = msg.length() + outText.getText().length();
        outText.append(msg);
        outText.setCaretPosition(caretPos);
    }
    /*===========================================================================
    ** incrementProc    increments the number of running threads
    **                  purpose is for Thread to call when
    **                  start executing
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    public static void incrementProc()
    {
        /*
        ** increment numProc and set Status to Executing
        */
        ++numProc;
        //status.setVisible(false);
        status.setBackground(new Color(10,255,10));
        status.setText("Executing!!!");
        status.repaint();
        status.setVisible(true);
        cmdPanel.repaint();
    }
    
    /*===========================================================================
    ** decrementProc    decrements the number of running threads
    **                  purpose is for Thread to call when
    **                  finished executing
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    public static void decrementProc()
    {
        /*
        ** check to make sure that numProc does not go less
        ** than 0 (shouldn't happen, but check anyway)
        */
        if ((numProc - 1) < 0)
        {
            numProc = 0;
        }
        else
        {
            --numProc;
        }
        
        if (numProc == 0)
        {
            /*
            ** decrement numProc and set Status to Waiting for Command
            */
            //status.setVisible(false);
            status.setBackground(new Color(204,204,204));
            status.setText("Waiting for Command");
            status.repaint();
            status.setVisible(true);
            cmdPanel.repaint();
        }
    }    
    
    /*===========================================================================
    ** finalize     close what needs to be
    **
    ** Inputs:  tmp     - the async Server
    **
    ** Returns: none 
    **===========================================================================*/
    public void finalize ()
    {
        CCBCLguiSysCall.killProcess();
        async.finalize();
        debug.finalize();
        System.exit(0);
    }
    
    /*===========================================================================
    ** private Variables 
    **===========================================================================*/
    

    
    /* Private variables */
    private CCBCLguiFunction[]      funcs;              /* Array of CCBCLguiFunction   */
    private String                  location;           /* location of ccbcl.pl        */
    private Vector                  ipList;             /* List of valid IP addresses  */
    private Vector                  cmdList;            /* List of commands            */
    private int                     cmdListPtr;         /* pointer to where we are in  */
                                                        /* the current list            */
    private static boolean          scriptMode;         /* scriptMode On/Off           */
    private static boolean          cmdMode;            /* Command Line Mode On/Off    */
    private Container               mainPanel;          /* our JFrame Panel            */
    private String                  title;              /* Application Title on JFrame */
    
    private static JTextArea        outText;            /* output Text Area            */
    private static JScrollPane      outPane;            /* output text pane            */
    
    private JMenuBar                mainMenu;           /* Our Menu Bar                */
    private JMenu                   commands[];         /* CCBCLguiFunction alpha menus*/
    private JMenu                   fileMen;            /* File Menu                   */
    private JMenu                   ipMen;              /* IP Menu                     */
    private JMenu                   opMen;              /* Options Menu                */
    private JCheckBoxMenuItem       savSetMen;          /* SaveSettings Munu check box */    
    
    private JPanel                  inputPanel;         /* panel for inputs            */
    private JPanel                  parmPanel;          /* panel for parameters        */
    private JTextArea               helpText;           /* displays help text          */
    private JTextArea               scriptText;         /* script text                 */
    private JLabel[]                parmLabels;         /* Parmeter Labels             */
    private JTextField[]            parmFields;         /* Parmeter Fields             */
    private JTextField              opts;               /* additional options field    */
    
    private JTextField              currentCmd;         /* displays current command    */
    private String                  currentFunc;        /* sustains current Function   */
    
    private static JPanel           cmdPanel;           /* panel with buttons an labels*/
    private JButton                 executeCmd;         /* Executes a command          */
    private JButton                 clrInputCmd;        /* clears input                */
    private JButton                 clrOutputCmd;       /* clears ouput                */
    private JLabel                  currentIP;          /* current ip being used       */
    private JLabel                  statLbl;            /* Status label                */
    private static JLabel           status;             /* threads status              */
    private static int              numProc;            /* number of running threads   */
    
    private JFileChooser            fileMenu;           /* filechooser to choose files */
    private File                    cInFile;            /* current script file         */
    private File                    cOutFile;           /* current output file         */
    private FileInputStream         inFile;             /* input file stream           */
    private FileOutputStream        outFile;            /* output file stream          */
    
    private static JOptionPane      opt;                /* get inputs and display msg  */
    
    private long                    timeout;            /* timeout of thread           */
    
    private static int              twidth;             /* width of frame              */
    private static int              theight;            /* height of frame             */    
    private CCBCLguiServer          async;              /* async dialog                */
    private CCBCLguiServer          debug;              /* debug dialog                */
    private CCBCLguiServer          ccblg;              /* debug dialog                */    
    
    private boolean                 saveSettings;       /* save settings               */
    private String                  savSetRec;          /* save settings record        */
    private String                  ipRec;              /* ip key value to save        */
    private String                  ipCntRec;           /* number of ips saved         */
    private String                  hlpFntRec;          /* help font key value to save */
    private String                  hlpBckRec;          /* help color Background to save*/
    private String                  hlpForRec;          /* help color Foreground to save*/
    private String                  outFntRec;          /* output font key value to save */
    private String                  outBckRec;          /* output color Background to save*/
    private String                  outForRec;          /* output color Foreground to save*/
    private String                  thsSizRec;          /* our size                     */
    private String                  thsLocRec;          /* our location                 */
    private String                  asySizRec;          /* async size                   */
    private String                  asyLocRec;          /* async location               */
    private String                  asyVisRec;          /* async visible                */
    private String                  dbgSizRec;          /* debug size                   */
    private String                  dbgLocRec;          /* debug location               */
    private String                  dbgVisRec;          /* debug visible                */
    private String                  ccbSizRec;          /* ccb size                     */
    private String                  ccbLocRec;          /* ccb location                 */
    private String                  ccbVisRec;          /* ccb visible                  */
    private String                  guiModRec;          /* what mode are we in          */ 
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.9  2003/04/17 22:09:19  HoltyB
** TBolt00000000:  Changes for new help text layout in CCBcl.pl.
**
** Revision 1.8  2002/02/21 22:12:04  HoltyB
** made changes to allow to save window settings
** added ccbcl.log view
**
** Revision 1.7  2002/02/19 20:52:48  HoltyB
** Added command line mode to interact directly with ccbcl.pl
**
** Revision 1.6  2002/02/19 16:31:15  HoltyB
** Added many new features
** AsyncServer
** DebugConsole
**
** Revision 1.5  2002/02/15 23:32:33  HoltyB
** Major change to persistent connection to ccbcl.pl
**
** Revision 1.4  2002/02/14 13:22:41  HoltyB
** Added function to change Exercisers being used
**
** Revision 1.3  2002/02/13 19:38:39  HoltyB
** made changes for compatability with nt 4.0
**
** Revision 1.2  2002/02/13 13:47:58  HoltyB
** added functionality to execute a command by pressing the enter key
** whille not in script mode
**
** Revision 1.1  2002/02/12 21:01:10  HoltyB
** initial integration of GUICCBCL
**
****************************************************************************/