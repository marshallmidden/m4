/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiMain.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     Main program for CCBCLGUI
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/

import java.awt.event.*;
import java.awt.*;
import java.util.*;
import javax.swing.*;

/*===========================================================================
** CCBCLguiMain
**===========================================================================*/
public class CCBCLguiMain
{
    /*===========================================================================
    ** main             where the world begins
    **
    ** Inputs:          args (not used)
    **
    ** Returns: none 
    **===========================================================================*/
    public static void main(String[] args)
    {
        double          scWidth = 0;
        double          scHeight = 0;
        String          loc;
        JOptionPane     opt;
        Toolkit         tk;
        Dimension       dim;
        
        try{
            /*
            ** check for the necessary setup files and get the location
            ** of the ccbcl.pl
            */
            loc = CCBCLguiSetup.checkSetup();
            
            /*
            ** if we get a return continue with the program
            */
            if(!(loc == null))            
            {
                /*
                ** use the toolkit to get the current
                ** width and height of the screen
                */
                tk = Toolkit.getDefaultToolkit();
                dim = tk .getScreenSize();
                scWidth = dim.getWidth();
                scHeight = dim.getHeight();
                
                /*
                ** convert the ccbcl.pl file into an
                ** array of functions with help text
                ** and parameters and sort it
                */
                CCBCLguiFunction[] funcs = CCBCLguiCreateFunc.CreateFunc(loc);
                Arrays.sort(funcs);
        
                final CCBCLguiFrame ccbcl = new CCBCLguiFrame(funcs, loc, 
                                    (int)(scWidth  * .80), (int)(scHeight * .80),
                                    ((int)(scWidth - (scWidth * .80)) /2), 
                                    ((int)(scHeight - (scHeight * .80)) /2));
                
                ccbcl.addWindowListener(new WindowAdapter()
                {
                    public void windowClosing(WindowEvent we)
                    {
                        ccbcl.finalize();
                        System.exit(0);
                    }
                });
        
                ccbcl.setVisible(true);
            }
            
            /*
            ** else print an error message and exit
            */
            else
            {
                opt = new JOptionPane();
                opt.showMessageDialog(null,"Could not locate ccbcl.pl");
                System.exit(1);
            }
                            
        }catch (Exception ec)
        {
            opt = new JOptionPane();
            opt.showMessageDialog(null,"Exception caught in main: " + ec.getMessage());
            ec.printStackTrace();
        }
    }
}


/****************************************************************************
** $Log$
** Revision 1.1  2005/05/04 18:53:54  RysavyR
** Initial revision
**
** Revision 1.4  2002/02/21 22:12:05  HoltyB
** made changes to allow to save window settings
** added ccbcl.log view
**
** Revision 1.3  2002/02/19 16:31:15  HoltyB
** Added many new features
** AsyncServer
** DebugConsole
**
** Revision 1.2  2002/02/15 23:32:33  HoltyB
** Major change to persistent connection to ccbcl.pl
**
** Revision 1.1  2002/02/12 21:01:10  HoltyB
** initial integration of GUICCBCL
**
****************************************************************************/