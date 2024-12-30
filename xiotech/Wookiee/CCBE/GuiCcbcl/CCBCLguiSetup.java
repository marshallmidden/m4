/* $Header$*/
/*===========================================================================
** FILE NAME:       CCBCLguiSetup.java
** MODULE TITLE:
** AUTHOR:          Bryan Holty
** DATE:            2/1/2001
**
** DESCRIPTION:     Checks and/or creates a file with the path to ccbcl.pl
**
** Copyright (c) 2001  XIOtech a Seagate Company.  All rights reserved.
**==========================================================================*/
import java.io.*;
import javax.swing.*;
import java.awt.*;


public class CCBCLguiSetup
{
    
    /*
    ** sfile.dat is the setup file and is stored in the users home directory
    */
    private static File sfile = new File(System.getProperty("user.home") + "\\sfile.dat");
    
    
    /*===========================================================================
    ** checkSetup
    ** Description:     checkSetup checks to make sure that the
    **                  command line ccbcl.pl exists and that we
    **                  have a path to it
    **
    ** return:          returns the path to the ccbcl.pl
    **                  or null if the path can not be found
    ** 
    **===========================================================================*/
    public static String checkSetup()
    {
        
        FileInputStream inFile;
        FileOutputStream outFile;
        JOptionPane input;
        int count = 0;
        int index = 0;
        boolean quit = false;
        String location = null;
        char[] arr;
        byte[] readIn;
        
        /*
        ** try 3 times to locate the file
        */
        while(!quit)
        {
            try{
                
                /*
                ** open the setup file and see if the path to
                ** the ccbcl.pl is stored there
                */
                inFile = new FileInputStream(sfile);
                readIn = new byte[inFile.available()];
                inFile.read(readIn);
                inFile.close();
                
                location = new String(readIn);
                
                /*
                ** parse the file to the CCBCL: portion and check
                ** the path.  If CCBCL: does not exist in the file
                ** we will get an exception and run the code below
                ** after the path is read, try and open the file to
                ** make sure it exists
                */
                index = location.indexOf("CCBCL:");
                if (index == -1)
                {
                    return null;
                }
            
                location = location.substring(index);
                index = location.indexOf(":");
                location = location.substring(index + 1);
                
                index = location.indexOf('\n');
            
                if (index == -1)
                {
                    return null;
                }
                
                location = location.substring(0, index);
                
                inFile = new FileInputStream(location + "\\ccbcl.pl");
                inFile.close();
                quit = true;
            }catch (Exception fe)
            {
                
                /*
                ** if count == 3 exit the loop
                */
                if(count == 3)
                {
                    quit = true;
                }
                
                /*
                ** if writeFile return false we want to quit
                */
                else
                {
                    quit = !writeFile();
                }
                
                /*
                ** if we are quitting make sure to set our return
                ** value to null
                */
                if(quit)
                {
                    location = null;
                }
                ++count;
            }
        }
        return location;             
    }
    
    /*===========================================================================
    ** writeFile
    ** Description:     asks the user for the location of the ccbcl.pl
    **                  and then write the path in the setup file sfile.dat
    **
    ** return:          returns true if succesful
    **                  return false if Exception is raised
    ** 
    **===========================================================================*/
    public static boolean writeFile()
    {
        FileOutputStream outFile;
        JOptionPane input;
        boolean returnVal = true;
        String location;
        
        /*
        ** open a JOptionpane to get the path to the ccbcl.pl
        */
        input = new JOptionPane();
        location = new String("CCBCL");
        
        JFileChooser fileMenu = new JFileChooser();
        fileMenu.setFileSelectionMode(JFileChooser.DIRECTORIES_ONLY);
        
        input.showMessageDialog(null, "Please Select the directory of " +
                                      "ccbcl.pl  ie(c:\\projects\\bigfoot\\ccbe)");
        
        int Val = fileMenu.showOpenDialog(null);
        
        /*
        ** open the Jfilechooser and get the directory returned
        */
        if(Val == JFileChooser.APPROVE_OPTION) 
        {
            if(!(updateValue(location, fileMenu.getSelectedFile().toString())))
            {
                location += ":" + fileMenu.getSelectedFile().toString() + "\n";
                
                /*
                ** write the path to sfile.dat
                */
                 try{
                    outFile = new FileOutputStream(sfile);
                    outFile.write(location.getBytes());
                    outFile.close();
                }catch (Exception fe2) 
                {
                    returnVal = false;
                    location = null;
                }
            }
        }
        else
        {
            returnVal = false;
            location = null;
            return returnVal;
        } 
    
        return returnVal;
    }
    
    /*===========================================================================
    ** getValue
    ** Description:     returns the value of the associated key
    **
    ** return:          value of the key if it exists
    **                  null if key does not exist
    ** 
    **===========================================================================*/
    public static String getValue(String key)
    {
        FileInputStream inFile;
        int count = 0;
        int index = 0;
        String location = null;
        char[] arr;
        byte[] readIn;
        
        try{
                
            /*
            ** open the setup file and look for the key
            */
            inFile = new FileInputStream(sfile);
            readIn = new byte[inFile.available()];
            inFile.read(readIn);
            inFile.close();
                
            location = new String(readIn);
                
            /*
            ** parse the file to the key portion and check
            ** the path.  If key does not exist in the file
            ** we will get an exception and run the code below
            */
            index = location.indexOf(key + ":");
            if (index == -1)
            {
                return null;
            }
            location = location.substring(index);
            index = location.indexOf(":");
            location = location.substring(index + 1);
            index = location.indexOf('\n');
            
            if (index == -1)
            {
                return null;
            }
            
            location = location.substring(0, index);
            
            }catch (Exception fe)
            {
                System.out.println("Error opening " + sfile.toString());
            }
    
        return location;
    }
    
    
    /*===========================================================================
    ** setNewValue
    ** Description:     sets a new key value pair
    **
    ** return:          returns true if succesful
    **                  return false if Exception is raised
    ** 
    **===========================================================================*/
    public static boolean setNewValue(String key, String val)
    {
        FileOutputStream outFile;
        String location = null;
        boolean returnVal = true;
        
        try{
                
            /*
            ** open the setup file in append mode
            */
            outFile = new FileOutputStream(sfile.toString(), true);
            
            location = key + ":" + val + "\n";
            
            outFile.write(location.getBytes());
            
            outFile.close();
                
            }catch (Exception fe)
            {
                returnVal = false;
                System.out.println("Error opening " + sfile.toString());
            }
    
        return returnVal;
    }
    
    /*===========================================================================
    ** updateValue
    ** Description:     updates a value for a key
    **
    ** return:          returns true if succesful
    **                  return false if unsuccesful
    ** 
    **===========================================================================*/
    public static boolean updateValue(String key, String val)
    {
        FileInputStream inFile;
        FileOutputStream outFile;
        int count = 0;
        int index = 0;
        String location = null;
        String temp = null;
        String temp2 = null;
        char[] arr;
        byte[] readIn;
        boolean returnVal = true;
        
        try{
                
            /*
            ** open the setup file and look for the key
            */
            inFile = new FileInputStream(sfile);
            readIn = new byte[inFile.available()];
            inFile.read(readIn);
            inFile.close();
                
            location = new String(readIn);
            temp = new String(location);    
            /*
            ** parse the file to the key portion and check
            ** the path.  If key does not exist in the file
            ** we will get an exception and run the code below
            */
            index = temp.indexOf(key + ":");
            if (index == -1)
            {
                return false;
            }
            temp = temp.substring(index);
            index = temp.indexOf('\n');
            
            if (index == -1)
            {
                return false;
            }
            
            temp = temp.substring(0, index);
            
            index = location.indexOf(temp);
            count = location.indexOf('\n', index);
            
            location = location.substring(0, index) + location.substring(count + 1);
            location += key + ":" + val + "\n";
            
            /*
            ** open the setup file
            */
            outFile = new FileOutputStream(sfile);
            
            outFile.write(location.getBytes());
            
            outFile.close();
                
            }catch (Exception fe)
            {
                returnVal = false;
                System.out.println("Error opening " + sfile.toString());
            }
    
        return returnVal;
    }
    
    /*===========================================================================
    ** removeValue
    ** Description:     removes key and value
    **
    ** return:          returns true if succesful
    **                  return false if unsuccesful
    ** 
    **===========================================================================*/
    public static boolean removeValue(String key)
    {
        FileInputStream inFile;
        FileOutputStream outFile;
        int count = 0;
        int index = 0;
        String location = null;
        String temp = null;
        char[] arr;
        byte[] readIn;
        boolean returnVal = true;
        
        try{
                
            /*
            ** open the setup file and look for the key
            */
            inFile = new FileInputStream(sfile);
            readIn = new byte[inFile.available()];
            inFile.read(readIn);
            inFile.close();
                
            location = new String(readIn);
            temp = new String(location);    
            /*
            ** parse the file to the key portion and check
            ** the path.  If key does not exist in the file
            ** we will get an exception and run the code below
            */
            index = temp.indexOf(key + ":");
            
            if (index == -1)
            {
                return false;
            }
            
            temp = temp.substring(index);
            index = temp.indexOf('\n');
            
            if (index == -1)
            {
                return false;
            }
            
            temp = temp.substring(0, index);
            
            index = location.indexOf(temp);
            count = location.indexOf('\n', index);
            
            location = location.substring(0, index) + location.substring(count + 1);
            
            /*
            ** open the setup file
            */
            outFile = new FileOutputStream(sfile);
            
            outFile.write(location.getBytes());
            
            outFile.close();
                
            }catch (Exception fe)
            {
                returnVal = false;
                System.out.println("Error opening " + sfile.toString());
            }
    
        return returnVal;
    }
    
    
    /*===========================================================================
    ** buildMenu        builds the menus and menu bar
    **
    ** Inputs:  none
    **
    ** Returns: none 
    **===========================================================================*/
    public static Point unPackPoint(String clr)
    {
        Point ret = null;
        int w;
        int h;
        int i = 0;
        String tmp = new String("");
        char[] arr = clr.toCharArray();
        
        try{
            while (arr[i] != ',')
            {
                tmp += arr[i++];
            }
        
            w = Integer.parseInt(tmp);
            ++i;
            tmp = new String("");
            
            while (i < arr.length)
            {
                tmp += arr[i++];
            }
        
            h = Integer.parseInt(tmp);
            
            ret = new Point(w,h);
            
        }catch (Exception e)
        {
            System.out.println("Error parsing Point:\n" + e.getMessage());
        }
        
        return ret;   
    }
    
    /*===========================================================================
    ** unPackColor    unpacks a Color
    **
    ** Inputs:  clr     - string representation of a color via packColor
    **
    ** Returns: none 
    **===========================================================================*/
    public static Color unPackColor(String clr)
    {
        Color ret = null;
        int r;
        int g;
        int b;
        int i = 0;
        String tmp = new String("");
        char[] arr = clr.toCharArray();
        
        try{
            while (arr[i] != ',')
            {
                tmp += arr[i++];
            }
        
            r = Integer.parseInt(tmp);
            ++i;
            tmp = new String("");
            
            while (arr[i] != ',')
            {
                tmp += arr[i++];
            }
        
            g = Integer.parseInt(tmp);
            ++i;
            tmp = new String("");
            
            while (i < arr.length)
            {
                tmp += arr[i++];
            }
        
            b = Integer.parseInt(tmp);
            
            ret = new Color(r,g,b);
            
        }catch (Exception e)
        {
            System.out.println("Error parsing Color:\n" + e.getMessage());
        }
        
        return ret;
    }
    
    /*===========================================================================
    ** packColor    packs a color
    **
    ** Inputs:  clr     - Color representation of a String via unPackColor
    **
    ** Returns: none 
    **===========================================================================*/
    public static String packColor(Color clr)
    {
        String ret = null;
        ret = String.valueOf(clr.getRed()) + "," +
              String.valueOf(clr.getGreen()) + "," +
              String.valueOf(clr.getBlue());
              
        return ret;
    }
    
    /*===========================================================================
    ** unPackFont    unpacks a Font
    **
    ** Inputs:  fnt     - string representation of a Font via packFont
    **
    ** Returns: none 
    **===========================================================================*/
    public static Font unPackFont(String fnt)
    {
        Font ret = null;
        String name = null;
        int style;
        int size;
        int i = 0;
        String tmp = new String("");
        char[] arr = fnt.toCharArray();
        
        try{
            while (arr[i] != ',')
            {
                tmp += arr[i++];
            }
        
            name = new String(tmp);
            ++i;
            tmp = new String("");
            
            while (arr[i] != ',')
            {
                tmp += arr[i++];
            }
        
            style = Integer.parseInt(tmp);
            ++i;
            tmp = new String("");
            
            while (i < arr.length)
            {
                tmp += arr[i++];
            }
        
            size = Integer.parseInt(tmp);
            
            ret = new Font(name,style,size);
            
        }catch (Exception e)
        {
            System.out.println("Error parsing Color:\n" + e.getMessage());
        }
        
        return ret;
    }
    
    /*===========================================================================
    ** packFont    packs a Font
    **
    ** Inputs:  fnt     - Font representation of a String via unPackFont
    **
    ** Returns: none 
    **===========================================================================*/
    public static String packFont(Font fnt)
    {
        String ret = null;
        ret = fnt.getName() + "," +
              String.valueOf(fnt.getStyle()) + "," +
              String.valueOf(fnt.getSize());
              
        return ret;
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
** Revision 1.2  2002/02/14 13:22:49  HoltyB
** Added function to change Exercisers being used
**
** Revision 1.1  2002/02/12 21:01:10  HoltyB
** initial integration of GUICCBCL
**
****************************************************************************/